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

#include "editor_layout_visual.h"

#include "action/action__init__.h"
#include "action/declaration.h"

#include "cegui/cegui_container.h"

#include "editors/layout/editor_layout_init.h"
#include "editors/layout/editor_layout_undo.h"

#include "mainwindow.h"
#include "project.h"

#include "ui_LayoutEditorHierarchyDockWidget.h"
#include "ui_LayoutEditorCreateWidgetDockWidget.h"

#include <QBuffer>
#include <QClipboard>
#include <QDrag>
#include <QInputDialog>
#include <QMenu>
#include <QToolBar>
#include <QtEvents>

namespace CEED {
namespace editors {
namespace layout {
namespace visual {

WidgetHierarchyItem::WidgetHierarchyItem(widgethelpers::Manipulator *manipulator)
    : QStandardItem(manipulator ? TO_QSTR(manipulator->m_widget->getName()) : QStringLiteral("<No widget>"))
    , m_manipulator(manipulator)
{
    if (manipulator != nullptr) {
        setToolTip("type: " + TO_QSTR(manipulator->m_widget->getType()));

        // interlink them so we can react on selection changes
        manipulator->m_treeItem = this;
    }

    refreshPathData(false);
    refreshOrderingData(false, false);

    setFlags(Qt::ItemIsEnabled |
                  Qt::ItemIsSelectable |
                  Qt::ItemIsEditable |
                  Qt::ItemIsDropEnabled |
                  Qt::ItemIsDragEnabled |
                  Qt::ItemIsUserCheckable);

    setData(Qt::Unchecked, Qt::CheckStateRole);
}

int WidgetHierarchyItem::getWidgetIdxInParent()
{
    // TODO: Move this to CEGUI::Window
    CEGUI::Window* widget = m_manipulator->m_widget;
    if (widget == nullptr)
        return -1;

    CEGUI::Window* parent = widget->getParent();
    if (parent == nullptr)
        return 0;

    for (int i = 0; i < parent->getChildCount(); i++) {
        if (parent->getChildAtIdx(i)->getNamePath() == widget->getNamePath())
            return i;
    }

    return -1;
}

void WidgetHierarchyItem::refreshPathData(bool recursive)
{
    // NOTE: We use widget path here because that's what QVariant can serialise and pass forth
    //       I have had weird segfaults when storing manipulator directly here, perhaps they
    //       are related to PySide, perhaps they were caused by my stupidity, we will never know!

    if (m_manipulator != nullptr) {
        setText(TO_QSTR(m_manipulator->m_widget->getName()));
        setData(TO_QSTR(m_manipulator->m_widget->getNamePath()), Qt::UserRole);

        if (recursive) {
            for (int i = 0; i < rowCount(); i++) {
                if (auto childManip = dynamic_cast<WidgetHierarchyItem*>(child(i))) {
                    childManip->refreshPathData(true);
                }
            }
        }
    }
}

void WidgetHierarchyItem::refreshOrderingData(bool resort, bool recursive)
{
    // resort=true with recursive=false makes no sense and is a bug
    Q_ASSERT(!resort || recursive);

    if (m_manipulator != nullptr) {
        setData(getWidgetIdxInParent(), Qt::UserRole + 1);

        if (recursive) {
            for (int i = 0; i < rowCount(); i++) {
                // we pass resort=false because sortChildren is recursive itself
                dynamic_cast<WidgetHierarchyItem*>(child(i))->refreshOrderingData(false, true);
            }
        }

        if (resort) {
            sortChildren(0);
        }
    }
}

void WidgetHierarchyItem::setData(const QVariant &value, int role)
{
    if ((role == Qt::CheckStateRole) && (m_manipulator != nullptr)) {
        // synchronise the manipulator with the lock state
        m_manipulator->setLocked(value == Qt::Checked);
    }

    return QStandardItem::setData(value, role);
}

void WidgetHierarchyItem::setLocked(bool locked, bool recursive)
{
    // we do it this way around to make sure the checkbox's check state
    // is always up to date
    setData(locked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

    if (recursive) {
        for (int i = 0; i < rowCount(); i++) {
            auto child = this->child(i);
            dynamic_cast<WidgetHierarchyItem*>(child)->setLocked(locked, true);
        }
    }
}

/////

bool WidgetHierarchyTreeModel::setData(const QModelIndex &index, const QVariant &value_, int role)
{
    if (role == Qt::EditRole) {
        auto item = static_cast<WidgetHierarchyItem*>(itemFromIndex(index));

        // if the new name is the same, cancel
        QString value = value_.toString();
        if (value == TO_QSTR(item->m_manipulator->m_widget->getName())) {
            return false;
        }

        // validate the new name, cancel if invalid
        value = widgethelpers::Manipulator::getValidWidgetName(value);
        if (value.isEmpty()) {
            QMessageBox msgBox;
            msgBox.setText("The name was not changed because the new name is invalid.");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return false;
        }

        // check if the new name is unique in the parent, cancel if not
        CEGUI::Window* parentWidget = item->m_manipulator->m_widget->getParent();
        if (parentWidget != nullptr && parentWidget->isChild(FROM_QSTR(value))) {
            QMessageBox msgBox;
            msgBox.setText("The name was not changed because the new name is in use by a sibling widget.");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return false;
        }

        // the name is good, apply it
        auto cmd = new undo::RenameCommand(m_dockWidget->m_visual, TO_QSTR(item->m_manipulator->m_widget->getNamePath()), value);
        m_dockWidget->m_visual->m_tabbedEditor->m_undoStack->push(cmd);

        // return false because the undo command has changed the text of the item already
        return false;
    }

    return QStandardItemModel::setData(index, value_, role);
}

bool WidgetHierarchyTreeModel::shouldManipulatorBeSkipped(widgethelpers::Manipulator *manipulator)
{
    return
            manipulator->m_widget->isAutoWindow() &&
            settings::getEntry("layout/visual/hide_deadend_autowidgets")->m_value.toBool() &&
            !manipulator->hasNonAutoWidgetDescendants();
}

WidgetHierarchyItem *WidgetHierarchyTreeModel::constructSubtree(widgethelpers::Manipulator *manipulator)
{
    auto* ret = new WidgetHierarchyItem(manipulator);

    QList<widgethelpers::Manipulator*> manipulatorChildren;

    for (QGraphicsItem* item_ : manipulator->childItems()) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            manipulatorChildren.append(item);
        }
    }

    for (auto* item : manipulatorChildren) {
        if (shouldManipulatorBeSkipped(item)) {
            // skip this branch as per settings
            continue;
        }

        WidgetHierarchyItem* childSubtree = constructSubtree(item);
        ret->appendRow(childSubtree);
    }

    return ret;
}

bool WidgetHierarchyTreeModel::synchroniseSubtree(WidgetHierarchyItem *hierarchyItem, widgethelpers::Manipulator *manipulator, bool recursive)
{
    if (hierarchyItem == nullptr || manipulator == nullptr) {
        // no manipulator = no hierarchy item, we definitely can't synchronise
        return false;
    }

    if (hierarchyItem->m_manipulator != manipulator) {
        // this widget hierarchy item itself will need to be recreated
        return false;
    }

    hierarchyItem->refreshPathData(false);

    if (recursive) {
        auto manipulatorsToRecreate = manipulator->getChildManipulators();

        int i = 0;
        // we knowingly do NOT use range in here, the rowCount might change
        // while we are processing!
        while (i < hierarchyItem->rowCount()) {
            auto* childHierarchyItem = static_cast<WidgetHierarchyItem*>(hierarchyItem->child(i));

            if (manipulatorsToRecreate.contains(childHierarchyItem->m_manipulator) &&
                    synchroniseSubtree(childHierarchyItem, childHierarchyItem->m_manipulator, true)) {
                manipulatorsToRecreate.removeOne(childHierarchyItem->m_manipulator);
                i += 1;

            } else {
                hierarchyItem->removeRow(i);
            }
        }

        for (auto childManipulator_ : manipulatorsToRecreate) {
            if (auto childManipulator = dynamic_cast<widgethelpers::Manipulator*>(childManipulator_)) {
                if (shouldManipulatorBeSkipped(childManipulator)) {
                    // skip this branch as per settings
                    continue;
                }

                hierarchyItem->appendRow(constructSubtree(childManipulator));
            }
        }
    }

    hierarchyItem->refreshOrderingData(true, true);
    return true;
}

QMimeData *WidgetHierarchyTreeModel::mimeData(const QModelIndexList &indexes) const
{
    // if the selection contains children of something that is also selected, we don't include that
    // (it doesn't make sense to move it anyways, it will be moved with its parent)

    QList<QStandardItem*> topItems;

    for (QModelIndex index : indexes) {
        QStandardItem* item = itemFromIndex(index);
        bool hasParent = false;

        for (QModelIndex parentIndex : indexes) {
            if (parentIndex == index) {
                continue;
            }

            QStandardItem* potentialParent = itemFromIndex(parentIndex);

            if (isChild(item, potentialParent)) {
                hasParent = true;
                break;
            }
        }

        if (!hasParent) {
            topItems.append(item);
        }
    }

    QStringList paths;
    for (QStandardItem* item : topItems) {
        QVariant data = item->data(Qt::UserRole);
        Q_ASSERT(data.type() == QVariant::String);
        paths.append(data.toString());
    }

    auto ret = new WidgetPathsMimeData();
    ret->paths = paths;
    ret->setData("application/x-ceed-widget-paths", QByteArray());
    return ret;
}

bool WidgetHierarchyTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action_, int row, int column, const QModelIndex &parent)
{
    if (data->hasFormat("application/x-ceed-widget-paths")) {
        QStringList widgetPaths = dynamic_cast<const WidgetPathsMimeData*>(data)->paths;
        QStringList targetWidgetPaths;

        auto newParent = itemFromIndex(parent);
        if (newParent == nullptr) {
            return false;
        }

        auto newParentManipulator = m_dockWidget->m_visual->m_scene->getManipulatorByPath(newParent->data(Qt::UserRole).toString());

        QSet<QString> usedNames;
        for (QString widgetPath : widgetPaths) {
            QString oldWidgetName = widgetPath.section('/', -1);
            // Prevent name clashes at the new parent
            // When a name clash occurs, we suggest a new name to the user and
            // ask them to confirm it/enter their own.
            // The tricky part is that we have to consider the other widget renames
            // too (in case we're reparenting and renaming more than one widget)
            // and we must prevent invalid names (i.e. containing "/")
            QString suggestedName = oldWidgetName;
            while (true) {
                QString error;
                // get a name that's not used in the new parent, trying to keep
                // the suggested name (which is the same as the old widget name at
                // the beginning)
                QString tempName = newParentManipulator->getUniqueChildWidgetName(suggestedName);
                // if the name we got is the same as the one we wanted...
                if (tempName == suggestedName) {
                    // ...we need to check our own usedNames list too, in case
                    // another widget we're reparenting has got this name.
                    int counter = 2;
                    while (usedNames.contains(suggestedName)) {
                        // When this happens, we simply add a numeric suffix to
                        // the suggested name. The result could theoretically
                        // collide in the new parent but it's OK because this
                        // is just a suggestion and will be checked again when
                        // the 'while' loops.
                        suggestedName = tempName + QString::number(counter);
                        counter += 1;
                        error = QString("Widget name is in use by another widget being %1").arg(QString::fromLatin1((action_ == Qt::CopyAction) ? "copied" : "moved"));
                    }

                    // if we had no collision, we can keep this name!
                    if (counter == 2) {
                        break;
                    }
                } else {
                    // The new parent had a child with that name already and so
                    // it gave us a new suggested name.
                    suggestedName = tempName;
                    error = "Widget name already exists in the new parent";
                }

                // Ask the user to confirm our suggested name or enter a new one
                // We do this in a loop because we validate the user input
                while (true) {
                    bool ok;
                    suggestedName = QInputDialog::getText(
                                m_dockWidget,
                                error,
                                QString("New name for '%1':").arg(oldWidgetName),
                                QLineEdit::Normal,
                                suggestedName,
                                &ok);
                    // Abort everything if the user cancels the dialog
                    if (!ok) {
                        return false;
                    }
                    // Validate the entered name
                    suggestedName = widgethelpers::Manipulator::getValidWidgetName(suggestedName);
                    if (!suggestedName.isEmpty()) {
                        break;
                    }
                    error = "Invalid name, please try again";
                }
            }

            usedNames.insert(suggestedName);
            targetWidgetPaths.append(newParent->data(Qt::UserRole).toString() + "/" + suggestedName);
        }

        if (action_ == Qt::MoveAction) {
            auto cmd = new undo::ReparentCommand(m_dockWidget->m_visual, widgetPaths, targetWidgetPaths);
            // FIXME: unreadable
            m_dockWidget->m_visual->m_tabbedEditor->m_undoStack->push(cmd);

            return true;

        } else if (action_ == Qt::CopyAction) {
            // FIXME: TODO
            return false;
        }

    } else if (data->hasFormat("application/x-ceed-widget-type")) {
        auto wtmd = dynamic_cast<const WidgetTypeMimeData*>(data);
        Q_ASSERT(wtmd != nullptr);
        QString widgetType = wtmd->type;
        auto parentItem = itemFromIndex(parent);
        // if the drop was at empty space (parentItem== nullptr) the parentItemPath
        // should be "" if no root item exists, otherwise the name of the root item
        QString parentItemPath;
        if (parentItem != nullptr)
            parentItemPath = parentItem->data(Qt::UserRole).toString();
        else if (m_dockWidget->m_visual->m_scene->m_rootManipulator != nullptr)
            m_dockWidget->m_visual->m_scene->m_rootManipulator->m_widget->getName();
        auto parentManipulator = parentItemPath.isEmpty() ? nullptr : m_dockWidget->m_visual->m_scene->getManipulatorByPath(parentItemPath);
        QString uniqueName;
        if (parentManipulator != nullptr)
            uniqueName = parentManipulator->getUniqueChildWidgetName(widgetType.section('/', -1));
        else
            uniqueName = widgetType.section('/', -1);

        auto cmd = new undo::CreateCommand(m_dockWidget->m_visual, parentItemPath, widgetType, uniqueName);
        m_dockWidget->m_visual->m_tabbedEditor->m_undoStack->push(cmd);

        return true;
    }

    return false;
}

/////

void WidgetHierarchyTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{

    QTreeView::selectionChanged(selected, deselected);

    // we are running synchronization the other way, this prevents infinite loops and recursion
    if (m_dockWidget->m_ignoreSelectionChanges) {
        return;
    }

    m_dockWidget->m_visual->m_scene->m_ignoreSelectionChanges = true;

    for (QModelIndex index : selected.indexes()) {
        auto item_ = m_dockWidget->m_model->itemFromIndex(index);

        if (auto item = dynamic_cast<WidgetHierarchyItem*>(item_)) {
            QString manipulatorPath = item->data(Qt::UserRole).toString();
            widgethelpers::Manipulator* manipulator = nullptr;
            if (!manipulatorPath.isEmpty()) {
                manipulator = m_dockWidget->m_visual->m_scene->getManipulatorByPath(manipulatorPath);
            }
            if (manipulator != nullptr) {
                manipulator->setSelected(true);
            }
        }
    }

    for (QModelIndex index : deselected.indexes()) {
        auto item_ = m_dockWidget->m_model->itemFromIndex(index);

        if (auto item = dynamic_cast<WidgetHierarchyItem*>(item_)) {
            QString manipulatorPath = item->data(Qt::UserRole).toString();
            widgethelpers::Manipulator* manipulator = nullptr;
            if (!manipulatorPath.isEmpty()) {
                manipulator = m_dockWidget->m_visual->m_scene->getManipulatorByPath(manipulatorPath);
            }
            if (manipulator != nullptr) {
                manipulator->setSelected(false);
            }
        }
    }

    m_dockWidget->m_visual->m_scene->m_ignoreSelectionChanges = false;
}

void WidgetHierarchyTreeView::setupContextMenu()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    m_contextMenu = new QMenu(this);

    m_renameAction = action::getAction("layout/rename");
    m_contextMenu->addAction(m_renameAction);
    connect(m_renameAction, &QAction::triggered, this, &WidgetHierarchyTreeView::editSelectedWidgetName);

    m_contextMenu->addSeparator();

    m_lockAction = action::getAction("layout/lock_widget");
    m_contextMenu->addAction(m_lockAction);
    m_unlockAction = action::getAction("layout/unlock_widget");
    m_contextMenu->addAction(m_unlockAction);
    m_recursivelyLockAction = action::getAction("layout/recursively_lock_widget");
    m_contextMenu->addAction(m_recursivelyLockAction);
    m_recursivelyUnlockAction = action::getAction("layout/recursively_unlock_widget");
    m_contextMenu->addAction(m_recursivelyUnlockAction);

    m_contextMenu->addSeparator();

    m_cutAction = action::getAction("all_editors/cut");
    m_contextMenu->addAction(m_cutAction);
    m_copyAction = action::getAction("all_editors/copy");
    m_contextMenu->addAction(m_copyAction);
    m_pasteAction = action::getAction("all_editors/paste");
    m_contextMenu->addAction(m_pasteAction);
    m_deleteAction = action::getAction("all_editors/delete");
    m_contextMenu->addAction(m_deleteAction);

    m_contextMenu->addSeparator();

    m_copyNamePathAction = action::getAction("layout/copy_widget_path");
    m_contextMenu->addAction(m_copyNamePathAction);
}

void WidgetHierarchyTreeView::contextMenuEvent(QContextMenuEvent *event)
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
    m_copyNamePathAction->setEnabled(haveSel);
    m_renameAction->setEnabled(haveSel);

    m_lockAction->setEnabled(haveSel);
    m_unlockAction->setEnabled(haveSel);
    m_recursivelyLockAction->setEnabled(haveSel);
    m_recursivelyUnlockAction->setEnabled(haveSel);

    m_deleteAction->setEnabled(haveSel);

    m_contextMenu->exec(event->globalPos());
}

void WidgetHierarchyTreeView::editSelectedWidgetName()
{
    auto selectedIndices = selectedIndexes();
    if (selectedIndices.isEmpty()) {
        return;
    }
    setCurrentIndex(selectedIndices[0]);
    edit(selectedIndices[0]);
}

void WidgetHierarchyTreeView::copySelectedWidgetPaths()
{
    auto selectedIndices = selectedIndexes();
    if (selectedIndices.isEmpty()) {
        return;
    }

    QStringList paths;
    for (QModelIndex index : selectedIndices) {
        if (auto item = dynamic_cast<WidgetHierarchyItem*>(m_dockWidget->m_model->itemFromIndex(index))) {
            if (item->m_manipulator != nullptr) {
                paths.append(TO_QSTR(item->m_manipulator->m_widget->getNamePath()));
            }
        }
    }

    if (!paths.isEmpty()) {
        // sort (otherwise the order is the item selection order)
        qSort(paths);
        QApplication::clipboard()->setText(paths.join("\n")); // FIXME: or \r\n or \r
    }
}

void WidgetHierarchyTreeView::setSelectedWidgetsLocked(bool locked, bool recursive)
{
    auto selectedIndices = selectedIndexes();
    if (selectedIndices.isEmpty()) {
        return;
    }

    // It is possible that we will make superfluous lock actions if user
    // selects widgets in a hierarchy (parent & child) and then does
    // a recursive lock. This doesn't do anything harmful so we don't
    // have any logic to prevent that.

    for (QModelIndex index : selectedIndices) {
        auto item_ = m_dockWidget->m_model->itemFromIndex(index);
        if (auto item = dynamic_cast<WidgetHierarchyItem*>(item_)) {
            if (item->m_manipulator != nullptr) {
                item->setLocked(locked, recursive);
            }
        }
    }
}

/////

HierarchyDockWidget::HierarchyDockWidget(VisualEditing *visual):
    QDockWidget()
{
    m_visual = visual;

    m_ui = new Ui_LayoutEditorHierarchyDockWidget();
    m_ui->setupUi(this);

    m_ignoreSelectionChanges = false;

    m_model = new WidgetHierarchyTreeModel(this);
    m_treeView = m_ui->treeView;
    m_treeView->m_dockWidget = this;

    // setModel() calls m_treeView->selectionChanged()
    m_ignoreSelectionChanges = true;
    m_treeView->setModel(m_model);
    m_ignoreSelectionChanges = false;

    m_rootWidgetManipulator = nullptr;
}

void HierarchyDockWidget::setRootWidgetManipulator(widgethelpers::Manipulator *root)
{
    m_rootWidgetManipulator = root;
    m_model->setRootManipulator(root);
    m_treeView->expandToDepth(0);
}

void HierarchyDockWidget::refresh()
{
    // this will resynchronise the entire model
    m_model->setRootManipulator(m_rootWidgetManipulator);
}

void HierarchyDockWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        bool handled = m_visual->m_scene->deleteSelectedWidgets();

        if (handled) {
            return;
        }
    }

    QDockWidget::keyReleaseEvent(event);
}

/////

WidgetMultiPropertyWrapper::WidgetMultiPropertyWrapper(propertytree::properties::Property *templateProperty,
                                                       const QList<propertytree::properties::Property *> &innerProperties,
                                                       const QList<Variant> &allValues,
                                                       const QList<Variant> &allDefaultValues,
                                                       bool takeOwnership,
                                                       CEGUI::Property *ceguiProperty,
                                                       const QList<CEGUI::PropertySet *> &ceguiSets,
                                                       VisualEditing *visual_)
    : propertytree::properties::MultiPropertyWrapper(templateProperty, innerProperties, allValues, allDefaultValues, takeOwnership)
{
    m_ceguiProperty = ceguiProperty;
    m_ceguiSets = ceguiSets;
    m_visual = visual_;
}

bool WidgetMultiPropertyWrapper::tryUpdateInner(const CEED::Variant &newValue, propertytree::properties::ChangeValueReason reason)
{
    if (propertytree::properties::MultiPropertyWrapper::tryUpdateInner(newValue, reason)) {
        QString ceguiValue = newValue.toString();

        // create and execute command
        QStringList widgetPaths;
        QMap<QString, CEED::Variant> undoOldValues;

        // set the properties where applicable
        for (auto ceguiSet : m_ceguiSets) {
            QString widgetPath = TO_QSTR(dynamic_cast<CEGUI::NamedElement*>(ceguiSet)->getNamePath());
            widgetPaths.append(widgetPath);
            undoOldValues[widgetPath] = m_ceguiProperty->get(ceguiSet);
        }

        // create the undoable command
        // but tell it not to trigger the property changed callback
        // on first run because our on value has already changed,
        // we just want to sync the widget value now.
        auto cmd = new undo::PropertyEditCommand(m_visual, TO_QSTR(m_ceguiProperty->getName()), widgetPaths, undoOldValues, ceguiValue, true);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);

        // make sure to redraw the scene to preview the property
        m_visual->m_scene->update();

        return true;
    }

    return false;
}

propertysetinspector::CEGUIPropertyManager::MultiPropertyWrapper *CEGUIWidgetPropertyManager::createMultiPropertyWrapper(
        propertysetinspector::CEGUIPropertyManager::Property *templateProperty,
        const QList<propertysetinspector::CEGUIPropertyManager::Property *> &innerProperties,
        bool takeOwnership)
{
    return MultiPropertyWrapper::create(templateProperty, innerProperties, takeOwnership,
                                        [](Property*p, const QList<Property*>&i, const QList<CEED::Variant>&a, const QList<CEED::Variant>&d, bool t)
    {
        return new WidgetMultiPropertyWrapper(p, i, a, d, t);
    });
}

propertytree::properties::MultiPropertyWrapper *CEGUIWidgetPropertyManager::createProperty(
        CEGUI::Property *ceguiProperty,
        const QList<CEGUI::PropertySet *> &ceguiSets)
{
    auto prop_ = propertysetinspector::CEGUIPropertyManager::createProperty(ceguiProperty, ceguiSets);
    auto prop = dynamic_cast<WidgetMultiPropertyWrapper*>(prop_);
    prop->m_ceguiProperty = ceguiProperty;
    prop->m_ceguiSets = ceguiSets;
    prop->m_visual = m_visual;

    return prop;
}

QString CEGUIWidgetPropertyManager::getSortKey(const QString &t)
{
    QString name = t;

    if (name == "Element") {
        return "000Element";
    } else if (name == "NamedElement") {
        return "001NamedElement";
    } else if (name == "Window") {
        return "002Window";
    } else if (name.startsWith("CEGUI/")) {
        return name.mid(6);
    } else if (name == "Unknown") {
        return "ZZZUnknown";
    } else {
        return name;
    }
}

OrderedMap<QString, propertytree::properties::BasePropertyCategory *> CEGUIWidgetPropertyManager::buildCategories(
        const QList<CEGUI::PropertySet *> &ceguiPropertySets)
{
    auto categories = propertysetinspector::CEGUIPropertyManager::buildCategories(ceguiPropertySets);

    // sort categories by name but keep some special categories on top
    QList<QString> sorted = categories.keys();
    std::sort(sorted.begin(), sorted.end(), [=](const QString& a, const QString& b) {
        return getSortKey(a) < getSortKey(b);
    });

    OrderedMap<QString, propertytree::properties::BasePropertyCategory*> ret;
    for (QString name : sorted) {
        ret[name] = categories[name];
    }

    return ret;
}

/////

PropertiesDockWidget::PropertiesDockWidget(VisualEditing *visual):
    QDockWidget()
{
    setObjectName("PropertiesDockWidget");
    m_visual = visual;

    setWindowTitle("Selection Properties");
    // Make the dock take as much space as it can vertically
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_inspector = new propertysetinspector::PropertyInspectorWidget();
    m_inspector->m_ptree->setupRegistry(new propertytree::editors::PropertyEditorRegistry(true));

    setWidget(m_inspector);
}

/////

void WidgetTypeTreeWidget::startDrag(Qt::DropActions supportedActions)
{
    // shamelessly stolen from CELE2 by Paul D Turner (GPLv3)

    QTreeWidgetItem* item = currentItem();
    QString widgetType = item->text(0);

    QString look;
    if (item->parent())
        look = item->parent()->text(0);

    auto mimeData = new WidgetTypeMimeData();
    mimeData->type = look.isEmpty() ? widgetType : (look + "/" + widgetType);
    mimeData->setData("application/x-ceed-widget-type", QByteArray());

    QPixmap pixmap = QPixmap(75,40);
    QPainter painter(&pixmap);
    painter.eraseRect(0, 0, 75, 40);
    painter.setBrush(Qt::DiagCrossPattern);
    painter.drawRect(0, 0, 74, 39);
    painter.end();

    QDrag drag(this);
    drag.setMimeData(mimeData);
    drag.setPixmap(pixmap);
    drag.setHotSpot(QPoint(0, 0));

    drag.exec(Qt::CopyAction);
}

bool WidgetTypeTreeWidget::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        // TODO: The big question is whether to reuse cached previews or always render them again.
        //       I always render them again for now to avoid all sorts of caching issues
        //       (when scheme/looknfeel editing is in place, etc...)

        QTreeWidgetItem* item = itemAt(dynamic_cast<QHelpEvent*>(event)->pos());

        if ((item != nullptr) && (item->childCount() == 0)) {
            QString skin = item->parent() ? item->parent()->text(0) : "__no_skin__";
            QString widgetType = item->text(0);
            QString fullWidgetType = item->parent() ? skin + "/" + widgetType : widgetType;
            QString tooltipText = "";
            try {
                if (skin == "__no_skin__")
                    tooltipText = "Unskinned widgetType";

                else if (widgetType == "TabButton")
                    tooltipText = "Can't render a preview as this is an auto widgetType, requires parent to be rendered.";

                else {
                    QByteArray ba;
                    QBuffer buffer(&ba);
                    buffer.open(QIODevice::WriteOnly);

                    mainwindow::MainWindow::instance->ceguiInstance->getWidgetPreviewImage(fullWidgetType).save(&buffer, "PNG");

                    tooltipText = QString("<img src=\"data:image/png;base64,%1\" />").arg(QString::fromLatin1(ba.toBase64(), ba.toBase64().length()));
                }
            } catch (CEGUI::Exception e) {
                tooltipText = QString("Couldn't render a widgetType preview... (exception: %1)").arg(QString::fromUtf8(e.what()));
            } catch (Exception e) {
                tooltipText = QString("Couldn't render a widgetType preview... (exception: %1)").arg(QString::fromUtf8(e.what()));
            }

            item->setToolTip(0, tr("<small>Drag to the layout to create!</small><br />%1").arg(tooltipText));
        }
    }

    return QTreeWidget::viewportEvent(event);
}

////

CreateWidgetDockWidget::CreateWidgetDockWidget(VisualEditing *visual)
    : QDockWidget()
{
    m_visual = visual;

    m_ui = new Ui_LayoutEditorCreateWidgetDockWidget();
    m_ui->setupUi(this);

    m_tree = m_ui->tree;
    m_tree->setVisual(visual);
}

void CreateWidgetDockWidget::populate()
{
    m_tree->clear();

    OrderedMap<QString, QStringList> wl = mainwindow::MainWindow::instance->ceguiInstance->getAvailableWidgetsBySkin();

    for (auto it = wl.begin(); it != wl.end(); it++) {
        QString skin = it.key();
        QStringList widgets = it.value();
        QTreeWidgetItem* skinItem = nullptr;

        if (skin == "__no_skin__")
            skinItem = m_tree->invisibleRootItem();
        else {
            skinItem = new QTreeWidgetItem();
            skinItem->setText(0, skin);
            // this makes sure the skin item isn't draggable
            skinItem->setFlags(Qt::ItemIsEnabled);
            m_tree->addTopLevelItem(skinItem);
        }

        // skinItem now represents the skin node, we add all widgets in that skin to it

        for (QString& widget : widgets) {
            auto* widgetItem = new QTreeWidgetItem();
            widgetItem->setText(0, widget);
            skinItem->addChild(widgetItem);
        }
    }
}

/////

EditingScene::EditingScene(VisualEditing *visual)
    : super(mainwindow::MainWindow::instance->ceguiInstance)
{
    m_visual = visual;
    m_rootManipulator = nullptr;

    m_ignoreSelectionChanges = false;
    connect(this, &EditingScene::selectionChanged, this, &EditingScene::slot_selectionChanged);
}

void EditingScene::setRootWidgetManipulator(widgethelpers::Manipulator *manipulator)
{
    clear();

    m_rootManipulator = manipulator;

    if (m_rootManipulator != nullptr) {
        // root manipulator changed, perform a full update
        m_rootManipulator->updateFromWidget(true);

        addItem(m_rootManipulator);
    }
}

widgethelpers::Manipulator *EditingScene::getManipulatorByPath(const QString &widgetPath)
{
    QString path0 = widgetPath.section('/', 0, 0);
    QString path1 = widgetPath.section('/', 1);
    Q_ASSERT(!path0.isEmpty());

    if (path1.isEmpty()) {
        Q_ASSERT(path0 == TO_QSTR(m_rootManipulator->m_widget->getName()));
        return m_rootManipulator;

    } else {
        // path[1] is the remainder of the path
        return dynamic_cast<widgethelpers::Manipulator*>(m_rootManipulator->getManipulatorByPath(path1));
    }
}

void EditingScene::setCEGUIDisplaySize(int width, int height, bool lazyUpdate)
{
    // overridden to keep the manipulators in sync

    super::setCEGUIDisplaySize(width, height, lazyUpdate);

    // FIXME: this won't do much with lazyUpdate = false
    if (/*hasattr(self, "rootManipulator") &&*/ m_rootManipulator != nullptr) {
        m_rootManipulator->updateFromWidget();
    }
}

bool EditingScene::deleteSelectedWidgets()
{
    QStringList widgetPaths;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            widgetPaths.append(TO_QSTR(item->m_widget->getNamePath()));
        }
    }

    if (!widgetPaths.isEmpty()) {
        auto* cmd = new undo::DeleteCommand(m_visual, widgetPaths);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
        return true;
    }

    return false;
}

void EditingScene::alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment)
{
    QStringList widgetPaths;
    QList<CEGUI::HorizontalAlignment> oldAlignments;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());
            widgetPaths += widgetPath;
            oldAlignments += item->m_widget->getHorizontalAlignment();
        }
    }

    if (!widgetPaths.isEmpty()) {
        auto cmd = new undo::HorizontalAlignCommand(m_visual, widgetPaths, oldAlignments, alignment);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::alignSelectionVertically(CEGUI::VerticalAlignment alignment)
{
    QStringList widgetPaths;
    QList<CEGUI::VerticalAlignment> oldAlignments;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());
            widgetPaths += widgetPath;
            oldAlignments += item->m_widget->getVerticalAlignment();
        }
    }

    if (!widgetPaths.isEmpty()) {
        auto cmd = new undo::VerticalAlignCommand(m_visual, widgetPaths, oldAlignments, alignment);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::normalisePositionOfSelectedWidgets()
{
    QStringList widgetPaths;
    QMap<QString, CEGUI::UVector2> oldPositions;

    // if there will be no non-zero offsets, we will normalise to absolute
    bool absolute = true;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());

            widgetPaths.append(widgetPath);
            oldPositions[widgetPath] = item->m_widget->getPosition();

            // if we find any non-zero offset, normalise to relative
            if ((item->m_widget->getPosition().d_x.d_offset != 0) || (item->m_widget->getPosition().d_y.d_offset != 0)) {
                absolute = false;
            }
        }
    }

    if (!widgetPaths.isEmpty()) {
        undo::NormalisePositionCommand* cmd = absolute ?
                    (undo::NormalisePositionCommand*) new undo::NormalisePositionToAbsoluteCommand(m_visual, widgetPaths, oldPositions) :
                    (undo::NormalisePositionCommand*) new undo::NormalisePositionToRelativeCommand(m_visual, widgetPaths, oldPositions);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::normaliseSizeOfSelectedWidgets()
{
    QStringList widgetPaths;
    QMap<QString, CEGUI::UVector2> oldPositions;
    QMap<QString, CEGUI::USize> oldSizes;

    // if there will be no non-zero offsets, we will normalise to absolute
    bool absolute = true;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());

            widgetPaths.append(widgetPath);
            oldPositions[widgetPath] = item->m_widget->getPosition();
            oldSizes[widgetPath] = item->m_widget->getSize();

            // if we find any non-zero offset, normalise to relative
            if ((item->m_widget->getSize().d_width.d_offset != 0) || (item->m_widget->getSize().d_height.d_offset != 0)) {
                absolute = false;
            }
        }
    }

    if (!widgetPaths.isEmpty()) {
        undo::NormaliseSizeCommand* cmd = absolute ?
                    (undo::NormaliseSizeCommand*) new undo::NormaliseSizeToAbsoluteCommand(m_visual, widgetPaths, oldPositions, oldSizes) :
                    (undo::NormaliseSizeCommand*) new undo::NormaliseSizeToRelativeCommand(m_visual, widgetPaths, oldPositions, oldSizes);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::roundPositionOfSelectedWidgets()
{
    QStringList widgetPaths;
    QMap<QString, CEGUI::UVector2> oldPositions;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());

            widgetPaths.append(widgetPath);
            oldPositions[widgetPath] = item->m_widget->getPosition();
        }
    }

    if (!widgetPaths.isEmpty()) {
        auto cmd = new undo::RoundPositionCommand(m_visual, widgetPaths, oldPositions);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::roundSizeOfSelectedWidgets()
{
    QStringList widgetPaths;
    QMap<QString, CEGUI::UVector2> oldPositions;
    QMap<QString, CEGUI::USize> oldSizes;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            QString widgetPath = TO_QSTR(item->m_widget->getNamePath());

            widgetPaths.append(widgetPath);
            oldPositions[widgetPath] = item->m_widget->getPosition();
            oldSizes[widgetPath] = item->m_widget->getSize();
        }
    }

    if (!widgetPaths.isEmpty()) {
        auto cmd = new undo::RoundSizeCommand(m_visual, widgetPaths, oldPositions, oldSizes);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::moveSelectedWidgetsInParentWidgetLists(int delta)
{
    QStringList widgetPaths;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        auto item = dynamic_cast<widgethelpers::Manipulator*>(item_);
        if (item == nullptr) {
            continue;
        }

        auto parentItem = dynamic_cast<widgethelpers::Manipulator*>(item->parentItem());
        if (parentItem == nullptr) {
            continue;
        }

        auto widget = dynamic_cast<CEGUI::SequentialLayoutContainer*>(parentItem->m_widget);
        if (widget == nullptr) {
            continue;
        }

        size_t potentialPosition = widget->getPositionOfChild(item->m_widget) + delta;
        if (potentialPosition < 0 || potentialPosition > widget->getChildCount() - 1) {
            continue;
        }

        QString widgetPath = TO_QSTR(item->m_widget->getNamePath());
        widgetPaths.append(widgetPath);
    }

    // TODO: We currently only support moving one widget at a time.
    //       Fixing this involves sorting the widgets by their position in
    //       the parent widget and then either working from the "right" side
    //       if delta > 0 or from the left side if delta < 0.
    if (widgetPaths.length() == 1) {
        auto cmd = new undo::MoveInParentWidgetListCommand(m_visual, widgetPaths, delta);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::ensureParentIsExpanded(WidgetHierarchyTreeView *view, WidgetHierarchyItem *treeItem)
{
    view->expand(treeItem->index());

    if (treeItem->parent()) {
        ensureParentIsExpanded(view, dynamic_cast<WidgetHierarchyItem*>(treeItem->parent()));
    }
}

void EditingScene::slot_selectionChanged()
{
    QList<CEGUI::PropertySet*> sets;

    auto selection = selectedItems();
    for (QGraphicsItem* item_ : selection) {
        CEGUI::Window* wdt = nullptr;

        if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
            wdt = item->m_widget;

        } else if (auto item = dynamic_cast<resizable::ResizingHandle*>(item_)) {
            if (auto parentManip = dynamic_cast<widgethelpers::Manipulator*>(item->m_parentResizable)) {
                wdt = parentManip->m_widget;
            }
        }

        if (wdt != nullptr && !sets.contains(wdt)) {
            sets += wdt;
        }
    }

    m_visual->m_propertiesDockWidget->m_inspector->setSource(sets);

    // we always sync the properties dock widget, we only ignore the hierarchy synchro if told so
    if (!m_ignoreSelectionChanges) {
        m_visual->m_hierarchyDockWidget->m_ignoreSelectionChanges = true;

        m_visual->m_hierarchyDockWidget->m_treeView->clearSelection();
        WidgetHierarchyItem* lastTreeItem = nullptr;
        for (QGraphicsItem* item_ : selection) {
            if (auto item = dynamic_cast<widgethelpers::Manipulator*>(item_)) {
                if (/*hasattr(item, "treeItem") && */item->m_treeItem != nullptr) {
                    m_visual->m_hierarchyDockWidget->m_treeView->selectionModel()->select(item->m_treeItem->index(), QItemSelectionModel::Select);
                    ensureParentIsExpanded(m_visual->m_hierarchyDockWidget->m_treeView, item->m_treeItem);
                    lastTreeItem = item->m_treeItem;
                }
            }
        }

        if (lastTreeItem != nullptr) {
            m_visual->m_hierarchyDockWidget->m_treeView->scrollTo(lastTreeItem->index());
        }

        m_visual->m_hierarchyDockWidget->m_ignoreSelectionChanges = false;
    }
}

void EditingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    super::mouseReleaseEvent(event);

    QStringList movedWidgetPaths;
    QMap<QString, CEGUI::UVector2> movedOldPositions;
    QMap<QString, CEGUI::UVector2> movedNewPositions;

    QStringList resizedWidgetPaths;
    QMap<QString, CEGUI::UVector2> resizedOldPositions;
    QMap<QString, CEGUI::USize> resizedOldSizes;
    QMap<QString, CEGUI::UVector2> resizedNewPositions;
    QMap<QString, CEGUI::USize> resizedNewSizes;

    // we have to "expand" the items, adding parents of resizing handles
    // instead of the handles themselves
    QList<widgethelpers::Manipulator*> expandedSelectedItems;
    for (QGraphicsItem* selectedItem_ : selectedItems()) {
        if (auto selectedItem = dynamic_cast<widgethelpers::Manipulator*>(selectedItem_)) {
            expandedSelectedItems.append(selectedItem);
        } else if (auto selectedItem = static_cast<resizable::ResizingHandle*>(selectedItem_)) {
            if (auto parentItem = dynamic_cast<widgethelpers::Manipulator*>(selectedItem->parentItem())) {
                expandedSelectedItems.append(parentItem);
            }
        }
    }

    for (widgethelpers::Manipulator* item : expandedSelectedItems) {
        /*if (isinstance(item, widgethelpers.Manipulator))*/ {
            if (item->m_preMovePos) {
                QString widgetPath = TO_QSTR(item->m_widget->getNamePath());
                movedWidgetPaths.append(widgetPath);
                movedOldPositions[widgetPath] = *item->m_preMovePos;
                movedNewPositions[widgetPath] = item->m_widget->getPosition();

                // it won't be needed anymore so we use this to mark we picked this item up
                item->m_preMovePos = nonstd::nullopt;
            }

            if (item->m_preResizePos && item->m_preResizeSize) {
                QString widgetPath = TO_QSTR(item->m_widget->getNamePath());
                resizedWidgetPaths.append(widgetPath);
                resizedOldPositions[widgetPath] = *item->m_preResizePos;
                resizedOldSizes[widgetPath] = *item->m_preResizeSize;
                resizedNewPositions[widgetPath] = item->m_widget->getPosition();
                resizedNewSizes[widgetPath] = item->m_widget->getSize();

                // it won't be needed anymore so we use this to mark we picked this item up
                item->m_preResizePos = nonstd::nullopt;
                item->m_preResizeSize = nonstd::nullopt;
            }
        }
    }

    if (!movedWidgetPaths.isEmpty()) {
        auto cmd = new undo::MoveCommand(m_visual, movedWidgetPaths, movedOldPositions, movedNewPositions);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }

    if (!resizedWidgetPaths.isEmpty()) {
        auto cmd = new undo::ResizeCommand(m_visual, resizedWidgetPaths, resizedOldPositions, resizedOldSizes, resizedNewPositions, resizedNewSizes);
        cmd->postConstruct();
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void EditingScene::keyReleaseEvent(QKeyEvent *event)
{
    bool handled = false;

    if (event->key() == Qt::Key_Delete) {
        handled = deleteSelectedWidgets();
    }

    if (!handled) {
        super::keyReleaseEvent(event);

    } else {
        event->accept();
    }
}

void EditingScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    // if the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (m_rootManipulator != nullptr) {
        super::dragEnterEvent(event);

    } else {
        // otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type")) {
            event->acceptProposedAction();
        }
    }
}

void EditingScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    // if the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (m_rootManipulator != nullptr) {
        super::dragMoveEvent(event);

    } else {
        // otherwise we should accept a new root widget to the empty layout if it's a new widget
        if (event->mimeData()->hasFormat("application/x-ceed-widget-type")) {
            event->acceptProposedAction();
        }
    }
}

void EditingScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    // if the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (m_rootManipulator != nullptr) {
        super::dragEnterEvent(event);

    } else {
    }
}

void EditingScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    // if the root manipulator is in place the QGraphicsScene machinery will take care of drag n drop
    // the graphics items (manipulators in fact) have that implemented already
    if (m_rootManipulator != nullptr) {
        super::dropEvent(event);

    } else {
        auto wtmd = dynamic_cast<const WidgetTypeMimeData*>(event->mimeData());
        Q_ASSERT(wtmd != nullptr);
        if (wtmd) {
            QString widgetType = wtmd->type;

            auto cmd = new undo::CreateCommand(m_visual, "", widgetType, widgetType.section('/', -1));
            m_visual->m_tabbedEditor->m_undoStack->push(cmd);

            event->acceptProposedAction();

        } else {
            event->ignore();
        }
    }
}

/////

VisualEditing::VisualEditing(LayoutTabbedEditor *tabbedEditor)
    : QWidget()
    , multi::EditMode()
{
    m_tabbedEditor = tabbedEditor;

    m_hierarchyDockWidget = new HierarchyDockWidget(this);
    m_propertiesDockWidget = new PropertiesDockWidget(this);
    m_createWidgetDockWidget = new CreateWidgetDockWidget(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_scene = new EditingScene(this);

    setupActions();
    setupToolBar();
    m_hierarchyDockWidget->m_treeView->setupContextMenu();

    m_oldViewState = nullptr;
}

void VisualEditing::setupActions()
{
    m_connectionGroup = new action::ConnectionGroup(action::ActionManager::instance);

    // horizontal alignment actions
    m_alignHLeftAction = action::getAction("layout/align_hleft");
    m_connectionGroup->add(m_alignHLeftAction, [=](){ m_scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::HA_LEFT); });
    m_alignHCentreAction = action::getAction("layout/align_hcentre");
    m_connectionGroup->add(m_alignHCentreAction, [=](){m_scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::HA_CENTRE); });
    m_alignHRightAction = action::getAction("layout/align_hright");
    m_connectionGroup->add(m_alignHRightAction, [=](){ m_scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::HA_RIGHT); });

    // vertical alignment actions
    m_alignVTopAction = action::getAction("layout/align_vtop");
    m_connectionGroup->add(m_alignVTopAction, [=](){m_scene->alignSelectionVertically(CEGUI::VerticalAlignment::VA_TOP); });
    m_alignVCentreAction = action::getAction("layout/align_vcentre");
    m_connectionGroup->add(m_alignVCentreAction, [=](){ m_scene->alignSelectionVertically(CEGUI::VerticalAlignment::VA_CENTRE); });
    m_alignVBottomAction = action::getAction("layout/align_vbottom");
    m_connectionGroup->add(m_alignVBottomAction, [=](){ m_scene->alignSelectionVertically(CEGUI::VerticalAlignment::VA_BOTTOM); });

    m_focusPropertyInspectorFilterBoxAction = action::getAction("layout/focus_property_inspector_filter_box");
    m_connectionGroup->add(m_focusPropertyInspectorFilterBoxAction, [=](){ focusPropertyInspectorFilterBox(); });

    // normalise actions
    m_connectionGroup->add("layout/normalise_position", [=](){ m_scene->normalisePositionOfSelectedWidgets(); });
    m_connectionGroup->add("layout/normalise_size", [=](){ m_scene->normaliseSizeOfSelectedWidgets(); });

    // rounding position and size actions
    m_connectionGroup->add("layout/round_position", [=](){ m_scene->roundPositionOfSelectedWidgets(); });
    m_connectionGroup->add("layout/round_size", [=](){ m_scene->roundSizeOfSelectedWidgets(); });

    // moving in parent widget list
    m_connectionGroup->add("layout/move_backward_in_parent_list", [=](){ m_scene->moveSelectedWidgetsInParentWidgetLists(-1); });
    m_connectionGroup->add("layout/move_forward_in_parent_list", [=](){ m_scene->moveSelectedWidgetsInParentWidgetLists(1); });
}

void VisualEditing::setupToolBar()
{
    m_toolBar = new QToolBar("Layout");
    m_toolBar->setObjectName("LayoutToolbar");
    m_toolBar->setIconSize(QSize(32, 32));

    m_toolBar->addAction(m_alignHLeftAction);
    m_toolBar->addAction(m_alignHCentreAction);
    m_toolBar->addAction(m_alignHRightAction);
    m_toolBar->addSeparator(); // ---------------------------
    m_toolBar->addAction(m_alignVTopAction);
    m_toolBar->addAction(m_alignVCentreAction);
    m_toolBar->addAction(m_alignVBottomAction);
    m_toolBar->addSeparator(); // ---------------------------
    m_toolBar->addAction(action::getAction("layout/snap_grid"));
    m_toolBar->addAction(action::getAction("layout/absolute_mode"));
    m_toolBar->addAction(action::getAction("layout/abs_integers_mode"));
    m_toolBar->addAction(action::getAction("layout/normalise_position"));
    m_toolBar->addAction(action::getAction("layout/normalise_size"));
    m_toolBar->addAction(action::getAction("layout/round_position"));
    m_toolBar->addAction(action::getAction("layout/round_size"));
    m_toolBar->addSeparator(); // ---------------------------
    m_toolBar->addAction(action::getAction("layout/move_backward_in_parent_list"));
    m_toolBar->addAction(action::getAction("layout/move_forward_in_parent_list"));
}

void VisualEditing::rebuildEditorMenu(QMenu *editorMenu)
{
    // similar to the toolbar, includes the focus filter box action
    editorMenu->addAction(m_alignHLeftAction);
    editorMenu->addAction(m_alignHCentreAction);
    editorMenu->addAction(m_alignHRightAction);
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(m_alignVTopAction);
    editorMenu->addAction(m_alignVCentreAction);
    editorMenu->addAction(m_alignVBottomAction);
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(action::getAction("layout/snap_grid"));
    editorMenu->addAction(action::getAction("layout/absolute_mode"));
    editorMenu->addAction(action::getAction("layout/abs_integers_mode"));
    editorMenu->addAction(action::getAction("layout/normalise_position"));
    editorMenu->addAction(action::getAction("layout/normalise_size"));
    editorMenu->addAction(action::getAction("layout/round_position"));
    editorMenu->addAction(action::getAction("layout/round_size"));
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(action::getAction("layout/move_backward_in_parent_list"));
    editorMenu->addAction(action::getAction("layout/move_forward_in_parent_list"));
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(m_focusPropertyInspectorFilterBoxAction);
}

void VisualEditing::initialise(CEGUI::Window *rootWidget)
{
    auto* pmap = mainwindow::MainWindow::instance->m_project->m_propertyMap;
    m_propertiesDockWidget->m_inspector->setPropertyManager(new CEGUIWidgetPropertyManager(pmap, this));

    setRootWidget(rootWidget);
    m_createWidgetDockWidget->populate();
}

CEGUI::Window *VisualEditing::getCurrentRootWidget()
{
    return m_scene->m_rootManipulator ? m_scene->m_rootManipulator->m_widget : nullptr;
}

void VisualEditing::setRootWidgetManipulator(widgethelpers::Manipulator *manipulator)
{
    CEGUI::Window* oldRoot = getCurrentRootWidget();

    m_scene->setRootWidgetManipulator(manipulator);
    m_hierarchyDockWidget->setRootWidgetManipulator(m_scene->m_rootManipulator);

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(getCurrentRootWidget());

    if (oldRoot) {
        CEGUI::WindowManager::getSingleton().destroyWindow(oldRoot);
    }

    // cause full redraw of the default GUI context to ensure nothing gets stuck
    CEGUI::System::getSingleton().getDefaultGUIContext().markAsDirty();
}

void VisualEditing::setRootWidget(CEGUI::Window *widget)
{
    if (widget == nullptr) {
        setRootWidgetManipulator(nullptr);

    } else {
        auto manipulator = new widgethelpers::Manipulator(this, nullptr, widget);
        manipulator->createChildManipulators(true, false);
        setRootWidgetManipulator(manipulator);
    }
}

void VisualEditing::notifyWidgetManipulatorsAdded(const QList<widgethelpers::Manipulator *> &manipulators)
{
    m_hierarchyDockWidget->refresh();
}

void VisualEditing::notifyWidgetManipulatorsRemoved(const QStringList &widgetPaths)
{
    m_hierarchyDockWidget->refresh();
}

void VisualEditing::showEvent(QShowEvent *event)
{
    mainwindow::MainWindow::instance->ceguiContainerWidget->activate(this, m_scene);
    mainwindow::MainWindow::instance->ceguiContainerWidget->setViewFeatures(true,
                                                                            true,
                                                                            settings::getEntry("layout/visual/continuous_rendering")->m_value.toBool());

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(getCurrentRootWidget());

    m_hierarchyDockWidget->setEnabled(true);
    m_propertiesDockWidget->setEnabled(true);
    m_createWidgetDockWidget->setEnabled(true);
    m_toolBar->setEnabled(true);
    if (m_tabbedEditor->editorMenu() != nullptr) {
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(true);
    }

    // make sure all the manipulators are in sync to matter what
    // this is there mainly for the situation when you switch to live preview, then change resolution, then switch
    // back to visual editing and all manipulators are of different size than they should be
    if (m_scene->m_rootManipulator != nullptr) {
        m_scene->m_rootManipulator->updateFromWidget();
    }

    // connect all our actions
    m_connectionGroup->connectAll();

    if (m_oldViewState != nullptr) {
        mainwindow::MainWindow::instance->ceguiContainerWidget->setViewState(m_oldViewState);
    }

    QWidget::showEvent(event);
}

void VisualEditing::hideEvent(QHideEvent *event)
{
    // remember our view transform
    delete m_oldViewState;
    m_oldViewState = mainwindow::MainWindow::instance->ceguiContainerWidget->getViewState();

    // disconnected all our actions
    m_connectionGroup->disconnectAll();

    m_hierarchyDockWidget->setEnabled(false);
    m_propertiesDockWidget->setEnabled(false);
    m_createWidgetDockWidget->setEnabled(false);
    m_toolBar->setEnabled(false);
    if (m_tabbedEditor->editorMenu() != nullptr) {
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(false);
    }

    mainwindow::MainWindow::instance->ceguiContainerWidget->deactivate(this);

    QWidget::hideEvent(event);
}

void VisualEditing::focusPropertyInspectorFilterBox()
{
    auto* filterBox = m_propertiesDockWidget->m_inspector->m_filterBox;
    // selects all contents of the filter so that user can replace that with their search phrase
    filterBox->selectAll();
    // sets focus so that typing puts text into the filter box without clicking
    filterBox->setFocus();
}

bool VisualEditing::performCut()
{
    bool ret = performCopy();
    m_scene->deleteSelectedWidgets();

    return ret;
}

bool VisualEditing::performCopy()
{
    QList<widgethelpers::Manipulator*> topMostSelected;

    for (QGraphicsItem* item_ : m_scene->selectedItems()) {
        auto item = dynamic_cast<widgethelpers::Manipulator*>(item_);
        if (item == nullptr) {
            continue;
        }

        bool hasAncestorSelected = false;

        for (auto item2_ : m_scene->selectedItems()) {
            auto item2 = dynamic_cast<widgethelpers::Manipulator*>(item2_);
            if (item2 == nullptr) {
                continue;
            }

            if (item == item2) {
                continue;
            }

            if (item2->isAncestorOf(item)) {
                hasAncestorSelected = true;
                break;
            }
        }

        if (!hasAncestorSelected) {
            topMostSelected.append(item);
        }
    }

    if (topMostSelected.isEmpty()) {
        return false;
    }

    // now we serialise the top most selected widgets (and thus their entire hierarchies)
    QList<widgethelpers::SerialisationData*> topMostSerialisationData;
    for (auto wdt : topMostSelected) {
        auto serialisationData = new widgethelpers::SerialisationData(this, wdt->m_widget);
        serialisationData->serialiseChildren(wdt->m_widget);
        // we set the visual to None because we can't pickle QWidgets (also it would prevent copying across editors)
        // we will set it to the correct visual when we will be pasting it back
        serialisationData->setVisual(nullptr);

        topMostSerialisationData.append(serialisationData);
    }

    auto data = new WidgetHeirarchyListMimeData();
    data->serialisationList = topMostSerialisationData;
    data->setData("application/x-ceed-widget-hierarchy-list", QByteArray());
    QApplication::clipboard()->setMimeData(data);

    return true;
}

bool VisualEditing::performPaste()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();

    if (!data->hasFormat("application/x-ceed-widget-hierarchy-list")) {
        return false;
    }

    auto whlmd = dynamic_cast<const WidgetHeirarchyListMimeData*>(data);

    QList<widgethelpers::SerialisationData*> topMostSerialisationData = whlmd->serialisationList;

    if (topMostSerialisationData.isEmpty()) {
        return false;
    }

    widgethelpers::Manipulator* targetManipulator = nullptr;
    for (QGraphicsItem* item_ : m_scene->selectedItems()) {
        auto item = dynamic_cast<widgethelpers::Manipulator*>(item_);
        if (item == nullptr) {
            continue;
        }

        // multiple targets, we can't decide!
        if (targetManipulator != nullptr) {
            return false;
        }

        targetManipulator = item;
    }

    if (targetManipulator == nullptr) {
        return false;
    }

    for (auto serialisationData : topMostSerialisationData) {
        serialisationData->setVisual(this);
    }

    auto cmd = new undo::PasteCommand(this, topMostSerialisationData, TO_QSTR(targetManipulator->m_widget->getNamePath()));
    m_tabbedEditor->m_undoStack->push(cmd);

    // select the topmost pasted widgets for convenience
    m_scene->clearSelection();
    for (auto serialisationData : topMostSerialisationData) {
        auto manipulator = targetManipulator->getManipulatorByPath(TO_QSTR(serialisationData->m_name));
        manipulator->setSelected(true);
    }

    return true;
}

bool VisualEditing::performDelete()
{
    m_scene->deleteSelectedWidgets();
    return true;
}



} // visual
} // layout
} // editors
} // CEED
