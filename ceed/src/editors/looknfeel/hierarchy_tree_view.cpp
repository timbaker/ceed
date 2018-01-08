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

#include "hierarchy_tree_view.h"

#include "editors/looknfeel/falagard_element_editor.h"
#include "editors/looknfeel/falagard_element_inspector.h"
#include "editors/looknfeel/hierarchy_dock_widget.h"
#include "editors/looknfeel/editor_looknfeel_visual.h"

#include <QMenu>
#include <QtEvents>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_view {

LookNFeelHierarchyTreeView::LookNFeelHierarchyTreeView(QWidget *parent):
    super(parent)
{
    m_dockWidget = nullptr;

    m_originalBackgroundColour = QColor(237, 215, 215);
}

void LookNFeelHierarchyTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    propertytree::ui::PropertyTreeView::paintAlternatingRowBackground(this, m_originalBackgroundColour, painter, options, index);

    // Calling the regular draw function
    super::drawRow(painter, options, index);
}

void LookNFeelHierarchyTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    super::selectionChanged(selected, deselected);

    auto model = dynamic_cast<hierarchy_tree_model::LookNFeelHierarchyTreeModel*>(this->model());

    // Notify the falagard element editor of the change
    auto selectedIndices = selected.indexes();
    if (!selectedIndices.isEmpty()) {
        auto firstItem = dynamic_cast<hierarchy_tree_item::LookNFeelHierarchyItem*>(model->itemFromIndex(selectedIndices[0]));
        m_dockWidget->m_tabbedEditor->m_visual->m_falagardElementEditorDockWidget->m_inspector->setSource(firstItem->m_falagardElement);
    } else
        m_dockWidget->m_tabbedEditor->m_visual->m_falagardElementEditorDockWidget->m_inspector->setSource(FalagardElement());


    // we are running synchronization the other way, this prevents infinite loops and recursion
    if (m_dockWidget->m_ignoreSelectionChanges)
        return;

    m_dockWidget->m_visual->m_scene->m_ignoreSelectionChanges = true; // copy-paste error from layout/visual.h?
#if 0
    for (auto index : selected.indexes()) {
        auto item = dynamic_cast<hierarchy_tree_item::LookNFeelHierarchyItem*>(model->itemFromIndex(index));

        if (item) {
            QString manipulatorPath = item->data(Qt::UserRole).toString();
            cegui::widgethelpers::Manipulator* manipulator = nullptr;
            if (!manipulatorPath.isEmpty())
                manipulator = m_dockWidget->m_visual->m_scene->getManipulatorByPath(manipulatorPath); // copy-paste error from layout/visual.h?

            if (manipulator != nullptr)
                manipulator->setSelected(true);
        }
    }

    for (auto index : deselected.indexes()) {
        auto item = dynamic_cast<hierarchy_tree_item::LookNFeelHierarchyItem*>(model->itemFromIndex(index));

        if (item) {
            QString manipulatorPath = item->data(Qt::UserRole).toString();
            cegui::widgethelpers::Manipulator* manipulator = nullptr;
            if (!manipulatorPath.isEmpty())
                manipulator = m_dockWidget->m_visual->m_scene->getManipulatorByPath(manipulatorPath); // copy-paste error from layout/visual.h?

            if (manipulator != nullptr)
                manipulator->setSelected(false);
        }
    }
#endif
    m_dockWidget->m_visual->m_scene->m_ignoreSelectionChanges = false;
}

void LookNFeelHierarchyTreeView::setupContextMenu()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);

    m_contextMenu->addSeparator();

    m_cutAction = action::getAction("all_editors/cut");
    m_contextMenu->addAction(m_cutAction);
    m_copyAction = action::getAction("all_editors/copy");
    m_contextMenu->addAction(m_copyAction);
    m_pasteAction = action::getAction("all_editors/paste");
    m_contextMenu->addAction(m_pasteAction);
    m_deleteAction = action::getAction("all_editors/delete");
    m_contextMenu->addAction(m_deleteAction);
}

void LookNFeelHierarchyTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    auto selectedIndices = selectedIndexes();

    // TODO: since these actions enabled state depends on the selection,
    // move the enabling/disabling to a central "selection changed" handler.
    // The handler should be called on tab activations too because
    // activating a tab changes the selection, effectively.
    // We don't touch the cut/copy/paste actions because they're shared
    // among all editors and disabling them here would disable them
    // for the other editors too.
    bool haveSel = !selectedIndices.isEmpty();

    m_deleteAction->setEnabled(haveSel);

    m_contextMenu->exec(event->globalPos());
}


} // namespace hierarchy_tree_view
} // namespace looknfeel
} // namespace editors
} // namespace CEED
