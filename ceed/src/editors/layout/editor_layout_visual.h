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

#ifndef CEED_editors_layout_visual_
#define CEED_editors_layout_visual_

#include "CEEDBase.h"

#include "resizable.h"
#include "editors/editors_multi.h"
#include "cegui/cegui_widgethelpers.h"

#include "editors/layout/editor_layout_widgethelpers.h"

#include "propertysetinspector.h"

#include "propertytree/propertytree_init.h"
#include "propertytree/propertytree_editors.h"

#include <QMimeData>
#include <QTreeWidget>

class Ui_LayoutEditorHierarchyDockWidget;
class Ui_LayoutEditorCreateWidgetDockWidget;

class QToolBar;

namespace CEED {
namespace editors {
namespace layout {
namespace visual {

class WidgetHierarchyItem : public QStandardItem
{
public:
    widgethelpers::Manipulator* m_manipulator;

    WidgetHierarchyItem(widgethelpers::Manipulator* manipulator);

    QStandardItem* clone() const override
    {
        auto* ret = new WidgetHierarchyItem(m_manipulator);
        ret->setData(data(Qt::CheckStateRole), Qt::CheckStateRole);
        return ret;
    }

    int getWidgetIdxInParent();

    /**Updates the stored path data for the item and its children
    */
    void refreshPathData(bool recursive = true);

    /**Updates the stored ordering data for the item and its children
    if resort is true the children are sorted according to their order in CEGUI::Window
    */
    void refreshOrderingData(bool resort = true, bool recursive = true);

    void setData(const QVariant &value, int role) override;

    /**Locks or unlocks this item.

    locked - if true this item gets locked = user won't be able to move it
             in the visual editing mode.
    recursive - if true, all children of this item will also get affected
                They will get locked or unlocked depending on the "locked"
                argument, independent of their previous lock state.
    */
    void setLocked(bool locked, bool recursive = false);
};

class WidgetHeirarchyListMimeData : public QMimeData
{
public:
    QList<widgethelpers::SerialisationData*> serialisationList;
};

class WidgetPathsMimeData : public QMimeData
{
public:
    QStringList paths;
};

class WidgetTypeMimeData : public QMimeData
{
public:
    QString type;
};

class WidgetHierarchyTreeModel : public QStandardItemModel
{
public:
    HierarchyDockWidget* m_dockWidget;

    WidgetHierarchyTreeModel(HierarchyDockWidget* dockWidget)
        : QStandardItemModel()
    {
        m_dockWidget = dockWidget;
        setSortRole(Qt::UserRole + 1);
        setItemPrototype(new WidgetHierarchyItem(nullptr));
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        return QStandardItemModel::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value_, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return QStandardItemModel::flags(index);
    }

    bool shouldManipulatorBeSkipped(widgethelpers::Manipulator* manipulator);

    WidgetHierarchyItem* constructSubtree(widgethelpers::Manipulator* manipulator);

    /**Attempts to synchronise subtree with given widget manipulator.
    If such a thing isn't possible it returns false.

    recursive - If true the synchronisation will recurse, trying to
                unify child widget hierarchy items with child manipulators.
                (This is generally what you want to do)
    */
    bool synchroniseSubtree(WidgetHierarchyItem* hierarchyItem, widgethelpers::Manipulator* manipulator, bool recursive = true);

    WidgetHierarchyItem* getRootHierarchyItem()
    {
        if (rowCount() > 0) {
            return dynamic_cast<WidgetHierarchyItem*>(item(0));

        } else {
            return nullptr;
        }
    }

    void setRootManipulator(widgethelpers::Manipulator* rootManipulator)
    {
        if (!synchroniseSubtree(getRootHierarchyItem(), rootManipulator)) {
            clear();

            if (rootManipulator != nullptr) {
                appendRow(constructSubtree(rootManipulator));
            }
        }
    }

    bool isChild(QStandardItem* parent, QStandardItem* potentialChild) const
    {
        int i = 0;
        // DFS, Qt doesn't have helper methods for this it seems to me :-/
        while (i < parent->rowCount()) {
            QStandardItem* child = parent->child(i);

            if (child == potentialChild) {
                return true;
            }

            if (isChild(child, potentialChild)) {
                return true;
            }

            i += 1;
        }

        return false;
    }

    QMimeData* mimeData(const QModelIndexList &indexes) const override;

    QStringList mimeTypes() const override
    {
        return { "application/x-ceed-widget-paths", "application/x-ceed-widget-type" };
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
};


/*!
\brief WidgetHierarchyTreeView

The actual widget hierarchy tree widget - what a horrible name
    This is a Qt widget that does exactly the same as QTreeWidget for now,
    it is a placeholder that will be put to use once the need arises - and it will.

*/
class WidgetHierarchyTreeView : public QTreeView
{
public:
    HierarchyDockWidget* m_dockWidget;
    QMenu* m_contextMenu;

    WidgetHierarchyTreeView(QWidget* parent = nullptr)
        : QTreeView(parent)
    {
        m_dockWidget = nullptr;
    }

    /**Synchronizes tree selection with scene selection.
        */
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    action::declaration::Action* m_renameAction;
    action::declaration::Action* m_lockAction;
    action::declaration::Action* m_unlockAction;
    action::declaration::Action* m_recursivelyLockAction;
    action::declaration::Action* m_recursivelyUnlockAction;
    action::declaration::Action* m_cutAction;
    action::declaration::Action* m_copyAction;
    action::declaration::Action* m_pasteAction;
    action::declaration::Action* m_deleteAction;
    action::declaration::Action* m_copyNamePathAction;

    void setupContextMenu();

    void contextMenuEvent(QContextMenuEvent *event) override;

    void editSelectedWidgetName();

    void copySelectedWidgetPaths();

    void setSelectedWidgetsLocked(bool locked, bool recursive = false);
};


/*!
\brief HierarchyDockWidget

Displays and manages the widget hierarchy. Contains the WidgetHierarchyTreeWidget.

*/
class HierarchyDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    Ui_LayoutEditorHierarchyDockWidget* m_ui;
    bool m_ignoreSelectionChanges;
    WidgetHierarchyTreeModel* m_model;
    WidgetHierarchyTreeView* m_treeView;
    widgethelpers::Manipulator* m_rootWidgetManipulator;

    HierarchyDockWidget(VisualEditing* visual);

    /**Sets the widget manipulator that is at the root of our observed hierarchy.
    Uses getTreeItemForManipulator to recursively populate the tree.
    */
    void setRootWidgetManipulator(widgethelpers::Manipulator* root);

    /**Refreshes the entire hierarchy completely from scratch*/
    void refresh();

    void keyReleaseEvent(QKeyEvent *event) override;
};


/*!
\brief WidgetMultiPropertyWrapper

Overrides the default MultiPropertyWrapper to update the 'inner properties'
    and then create undo commands to update the CEGUI widgets.

*/
class WidgetMultiPropertyWrapper : public propertytree::properties::MultiPropertyWrapper
{
public:
    CEGUI::Property* m_ceguiProperty;
    QList<CEGUI::PropertySet*> m_ceguiSets;
    VisualEditing* m_visual;

    WidgetMultiPropertyWrapper(propertytree::properties::Property* templateProperty,
                               const QList<propertytree::properties::Property*>& innerProperties,
                               const QList<CEED::Variant>& allValues,
                               const QList<CEED::Variant>& allDefaultValues,
                               bool takeOwnership,
                               CEGUI::Property* ceguiProperty = nullptr,
                               const QList<CEGUI::PropertySet*>& ceguiSets = QList<CEGUI::PropertySet*>(),
                               VisualEditing* visual_ = nullptr);

    bool tryUpdateInner(const Variant &newValue, propertytree::properties::ChangeValueReason reason = propertytree::properties::ChangeValueReason::Unknown) override;
};


/*!
\brief CEGUIWidgetPropertyManager

Customises the CEGUIPropertyManager by binding to a 'visual'
    so it can manipulate the widgets via undo commands.

    It also customises the sorting of the categories.

*/
class CEGUIWidgetPropertyManager : public propertysetinspector::CEGUIPropertyManager
{
public:
    VisualEditing* m_visual;

    CEGUIWidgetPropertyManager(propertymapping::PropertyMap* propertyMap, VisualEditing* visual)
        : propertysetinspector::CEGUIPropertyManager(propertyMap)
    {
        m_visual = visual;
    }

    MultiPropertyWrapper* createMultiPropertyWrapper(Property* templateProperty, const QList<Property*>& innerProperties, bool takeOwnership) override;

    propertytree::properties::MultiPropertyWrapper* createProperty(CEGUI::Property* ceguiProperty, const QList<CEGUI::PropertySet*>& ceguiSets) override;

    QString getSortKey(const QString& t);

    OrderedMap<QString, propertytree::properties::BasePropertyCategory*> buildCategories(
            const QList<CEGUI::PropertySet *> &ceguiPropertySets) override;
};

/*!
\brief PropertiesDockWidget

Lists and allows editing of properties of the selected widget(s).

*/
class PropertiesDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    propertysetinspector::PropertyInspectorWidget* m_inspector;

    PropertiesDockWidget(VisualEditing* visual);
};


/*!
\brief WidgetTypeTreeWidget

Represents a single available widget for creation (it has a mapping in the scheme or is
    a stock special widget - like DefaultWindow).

    Also provides previews for the widgets

*/
class WidgetTypeTreeWidget : public QTreeWidget
{
public:
    VisualEditing* m_visual;

    WidgetTypeTreeWidget(QWidget* parent = nullptr)
        : QTreeWidget(parent)
    {
        setDragEnabled(true);
    }

    void setVisual(VisualEditing* visual)
    {
        m_visual = visual;
    }

    void startDrag(Qt::DropActions supportedActions) override;

    bool viewportEvent(QEvent *event) override;
};

/*!
\brief CreateWidgetDockWidget

This lists available widgets you can create and allows their creation (by drag N drop)

*/
class CreateWidgetDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    Ui_LayoutEditorCreateWidgetDockWidget *m_ui;
    WidgetTypeTreeWidget* m_tree;

    CreateWidgetDockWidget(VisualEditing* visual);

    void populate();
};


/*!
\brief EditingScene

This scene contains all the manipulators users want to interact with. You can visualise it as the
    visual editing centre screen where CEGUI is rendered.

    It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

*/
class EditingScene : public cegui::widgethelpers::GraphicsScene
{
    typedef cegui::widgethelpers::GraphicsScene super;
public:
    VisualEditing* m_visual;
    widgethelpers::Manipulator* m_rootManipulator;
    bool m_ignoreSelectionChanges;

    EditingScene(VisualEditing* visual);

    void setRootWidgetManipulator(widgethelpers::Manipulator* manipulator);

    widgethelpers::Manipulator* getManipulatorByPath(const QString& widgetPath);

    void setCEGUIDisplaySize(int width, int height, bool lazyUpdate = true);

    bool deleteSelectedWidgets();

    void alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment);

    void alignSelectionVertically(CEGUI::VerticalAlignment alignment);

    void normalisePositionOfSelectedWidgets();

    void normaliseSizeOfSelectedWidgets();

    void roundPositionOfSelectedWidgets();

    void roundSizeOfSelectedWidgets();

    void moveSelectedWidgetsInParentWidgetLists(int delta);

    void ensureParentIsExpanded(WidgetHierarchyTreeView* view, WidgetHierarchyItem* treeItem);

    void slot_selectionChanged();

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;

    void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;

    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;

    void dropEvent(QGraphicsSceneDragDropEvent *event) override;
};

class VisualEditing : public QWidget, public multi::EditMode
{
public:
    LayoutTabbedEditor* m_tabbedEditor;
    HierarchyDockWidget* m_hierarchyDockWidget;
    PropertiesDockWidget* m_propertiesDockWidget;
    CreateWidgetDockWidget* m_createWidgetDockWidget;
    EditingScene* m_scene;
    cegui::container::ViewState* m_oldViewState;
    action::ConnectionGroup* m_connectionGroup;
    action::declaration::Action* m_alignHLeftAction;
    action::declaration::Action* m_alignHCentreAction;
    action::declaration::Action* m_alignHRightAction;
    action::declaration::Action* m_alignVTopAction;
    action::declaration::Action* m_alignVCentreAction;
    action::declaration::Action* m_alignVBottomAction;
    action::declaration::Action* m_focusPropertyInspectorFilterBoxAction;
    QToolBar* m_toolBar;

    /**This is the default visual editing mode

    see ceed.editors.multi.EditMode
    */
    VisualEditing(LayoutTabbedEditor* tabbedEditor);

    void setupActions();

    void setupToolBar();

    /**Adds actions to the editor menu*/
    void rebuildEditorMenu(QMenu* editorMenu);

    void initialise(CEGUI::Window* rootWidget);

    CEGUI::Window* getCurrentRootWidget();

    void setRootWidgetManipulator(widgethelpers::Manipulator* manipulator);

    /**Sets the root widget we want to edit
    */
    void setRootWidget(CEGUI::Window* widget);

    void notifyWidgetManipulatorsAdded(const QList<widgethelpers::Manipulator*>& manipulators);

    /**We are passing widget paths because manipulators might be destroyed at this point*/
    void notifyWidgetManipulatorsRemoved(const QStringList& widgetPaths);

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    /**Focuses into property set inspector filter

    This potentially allows the user to just press a shortcut to find properties to edit,
    instead of having to reach for a mouse.
    */
    void focusPropertyInspectorFilterBox();

    bool performCut();

    bool performCopy();

    bool performPaste();

    bool performDelete();
};

} // visual
} // layout
} // editors
} // CEED

// needs to be at the end, import to get the singleton
//from ceed import mainwindow
//from ceed import settings
//from ceed import action

#endif
