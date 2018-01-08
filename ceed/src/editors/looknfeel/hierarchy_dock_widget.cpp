/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hierarchy_dock_widget.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_dock_widget {

LookNFeelHierarchyDockWidget::LookNFeelHierarchyDockWidget(visual::LookNFeelVisualEditing *visual_, tabbed_editor::LookNFeelTabbedEditor *tabbedEditor)
    : QDockWidget()
{
    m_visual = visual_;
    m_tabbedEditor = tabbedEditor;

    m_ui = new Ui_LookNFeelHierarchyDockWidget();
    m_ui->setupUi(this);

    m_ignoreSelectionChanges = false;

    m_model = new hierarchy_tree_model::LookNFeelHierarchyTreeModel(this);

    m_widgetLookNameLabel = m_ui->widgetLookName;
    m_widgetLookNameLabel->setText("");

    m_displayStateCombobox = m_ui->displayStateCombobox;
    connect(m_displayStateCombobox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &LookNFeelHierarchyDockWidget::slot_displayStateComboboxCurrentIndexChanged);

    m_treeView = m_ui->treeView;
    m_treeView->m_dockWidget = this;
#if 0
    m_treeView->m_dockWidget->m_tabbedEditor->m_visual = visual_; // must be set before setModel() which calls LookNFeelHierarchyTreeView::selectionChanged()
    m_treeView->setModel(m_model);
    updateToNewWidgetLook(m_tabbedEditor->m_targetWidgetLook);
#endif
}

void LookNFeelHierarchyDockWidget::updateToNewWidgetLook(const QString& targetWidgetLook)
{
    if (!targetWidgetLook.isEmpty()) { // FIXME? not using this value
        QString name = tabbed_editor::LookNFeelTabbedEditor::unmapMappedNameIntoOriginalParts(m_tabbedEditor->m_targetWidgetLook).first;
        m_widgetLookNameLabel->setText(name);
    } else {
        m_widgetLookNameLabel->setText("");
    }

    updateStateCombobox();
    updateHierarchy();
}

void LookNFeelHierarchyDockWidget::updateStateCombobox()
{
    m_displayStateCombobox->blockSignals(true);
    m_displayStateCombobox->clear();

    if (m_tabbedEditor->m_targetWidgetLook.isEmpty()) {
        return;
    }

    CEGUI::WidgetLookManager::WidgetLookPointerMap widgetLookMap = CEGUI::WidgetLookManager::getSingleton().getWidgetLookPointerMap();
    CEGUI::WidgetLookFeel* widgetLookObject = widgetLookMap[FROM_QSTR(m_tabbedEditor->m_targetWidgetLook)];//CEGUI::Workarounds.WidgetLookFeelMapGet(widgetLookMap, m_tabbedEditor->targetWidgetLook);

    // Add the default entry: The show-all option
    QList<QPair<QString, QVariant>> listOfComboboxEntries = { { "Show all", QVariant() } };

    auto stateImageryMap = widgetLookObject->getStateImageryMap();
    for (auto it = stateImageryMap.begin(); it != stateImageryMap.end(); it++) {
        CEGUI::String currentStateImageryName = it->first;
        listOfComboboxEntries.append( { TO_QSTR(currentStateImageryName), TO_QSTR(currentStateImageryName) });
    }

    std::sort(listOfComboboxEntries.begin(), listOfComboboxEntries.end(),
              [](const QPair<QString, QVariant>& a, const QPair<QString, QVariant>& b)
    {
        QString nameA = a.first;
        QString nameB = b.first;
        if (nameA == "Show all")
            nameA.prepend("000");
        if (nameB == "Show all")
            nameB.prepend("000");
        return nameA < nameB;
    });

    // Go through the list and add each item
    for (auto entry : listOfComboboxEntries) {
        m_displayStateCombobox->addItem(entry.first, entry.second);
    }

    // Make the "Show all" option be the currently selected one.
    int showAllIndex = m_displayStateCombobox->findData(QVariant());
    m_displayStateCombobox->setCurrentIndex(showAllIndex);
    m_displayStateCombobox->blockSignals(false);
}

void LookNFeelHierarchyDockWidget::updateHierarchy()
{
    QString limitDisplayTo = m_displayStateCombobox->itemData(m_displayStateCombobox->currentIndex()).toString();

    auto widgetLookName = m_tabbedEditor->m_targetWidgetLook;
    if (!widgetLookName.isEmpty()) {
        auto widgetLookMap = CEGUI::WidgetLookManager::getSingleton().getWidgetLookPointerMap();
        CEGUI::WidgetLookFeel* widgetLookObject = widgetLookMap[widgetLookName.toStdString()]; // CEGUI::Workarounds::WidgetLookFeelMapGet(widgetLookMap, widgetLookName)
        m_model->updateTree(widgetLookObject, limitDisplayTo);
    } else
        m_model->updateTree(nullptr, limitDisplayTo);

    m_treeView->expandAll();
}

#if 0 // copy-paste error from layout/visual.h ?
bool LookNFeelHierarchyDockWidget::synchroniseSubtree(layout::visual::WidgetHierarchyItem *hierarchyItem, cegui::widgethelpers::Manipulator *manipulator, bool recursive)
{
    if (hierarchyItem == nullptr || manipulator == nullptr)
        // no manipulator = no hierarchy item, we definitely can't synchronise
        return false;

    if (hierarchyItem->m_manipulator != manipulator)
        // this widget hierarchy item itself will need to be recreated
        return false;

    hierarchyItem->refreshPathData(false);

    if (recursive) {
        auto manipulatorsToRecreate = manipulator->getChildManipulators();

        int i = 0;
        // we knowingly do NOT use range in here, the rowCount might change
        // while we are processing!
        while (i < hierarchyItem->rowCount()) {
            auto childHierarchyItem = dynamic_cast<layout::visual::WidgetHierarchyItem*>(hierarchyItem->child(i));

            if (manipulatorsToRecreate.contains(childHierarchyItem->m_manipulator) &&
                    synchroniseSubtree(childHierarchyItem, childHierarchyItem->m_manipulator, true)) {
                manipulatorsToRecreate.removeOne(childHierarchyItem->m_manipulator);
                i += 1;

            } else {
                hierarchyItem->removeRow(i);
            }
        }

        for (auto childManipulator : manipulatorsToRecreate) {
            if (shouldManipulatorBeSkipped(childManipulator))
                // skip this branch as per settings
                continue;

            hierarchyItem->appendRow(constructSubtree(childManipulator));
        }
    }

    return true;
}
#endif

} // namespace hierarchy_dock_widget
} // namespace looknfeel
} // namespace editors
} // namespace CEED
