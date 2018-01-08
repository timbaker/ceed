/*
   created:    25th June 2014
   author:     Lukas E Meindl
*/

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

#ifndef CEED_editors_looknfeel_hierarchy_dock_widget_
#define CEED_editors_looknfeel_hierarchy_dock_widget_

#include "CEEDBase.h"

#include "editors/layout/editor_layout_visual.h"

#include "editors/looknfeel/hierarchy_tree_model.h"
#include "editors/looknfeel/hierarchy_tree_view.h"
#include "editors/looknfeel/tabbed_editor.h"

#include "cegui/cegui_widgethelpers.h"

#include "ui_LookNFeelEditorHierarchyDockWidget.h"

#include <QDockWidget>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_dock_widget {

/*!
\brief LookNFeelHierarchyDockWidget

Displays and manages the widget hierarchy. Contains the LookNFeelHierarchyTreeWidget.

*/
class LookNFeelHierarchyDockWidget : public QDockWidget
{
public:
    visual::LookNFeelVisualEditing* m_visual;
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    Ui_LookNFeelHierarchyDockWidget* m_ui;
    bool m_ignoreSelectionChanges;
    hierarchy_tree_model::LookNFeelHierarchyTreeModel* m_model;
    QLabel* m_widgetLookNameLabel;
    QComboBox* m_displayStateCombobox;
    hierarchy_tree_view::LookNFeelHierarchyTreeView* m_treeView;

    LookNFeelHierarchyDockWidget(visual::LookNFeelVisualEditing* visual_, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    void updateToNewWidgetLook(const QString &targetWidgetLook);

    /**
    We populate the combobox with a show-all option and display options for the different states
    in the WidgetLookFeel
    :return:
    */
    void updateStateCombobox();

    /**
    Updates the hierarchy based on the WidgetLookFeel which was selected and optionally limiting
    the display to a specific StateImagery and its referenced elements and their children
    :return:
    */
    void updateHierarchy();

    void slot_displayStateComboboxCurrentIndexChanged()
    {
        updateHierarchy();
    }
#if 0 // copy paste error from layout/visual.h ?
    QVariant data(const QModelIndex& index, Qt::ItemDataRole role = Qt::DisplayRole)
    {
        return QDockWidget::data(index, role);
    }

    bool setData(self, index, value, role = Qt::EditRole)
    {
        if role == Qt::EditRole:
            item = self.itemFromIndex(index)

            // if the new name is the same, cancel
            if value == item.manipulator.widget.getName():
                return false

            // return false because the undo command has changed the text of the item already
            return false

        return super(LookNFeelHierarchyTreeModel, self).setData(index, value, role);
    }

    Qt::ItemFlags flags(self, index)
    {
        return QDockWidget::flags(index);
    }


    /**Attempts to synchronise subtree with given widget manipulator.
    If such a thing isn't possible it returns false.

    recursive - If true the synchronisation will recurse, trying to
                unify child widget hierarchy items with child manipulators.
                (This is generally what you want to do)
    */
    bool synchroniseSubtree(layout::visual::WidgetHierarchyItem* hierarchyItem, cegui::widgethelpers::Manipulator* manipulator, bool recursive = true);


    hierarchy_tree_item::LookNFeelHierarchyItem* getRootHierarchyItem()
    {
        if (rowCount() > 0)
            return self.item(0);
        else
            return None;
    }

    void setRootManipulator(cegui::widgethelpers::Manipulator* rootManipulator)
    {
        if (!synchroniseSubtree(getRootHierarchyItem(), rootManipulator)) {
            clear();

            if (rootManipulator != nullptr)
                sappendRow(constructSubtree(rootManipulator));
        }
    }

    bool keyReleaseEvent(QKeyEvent *event) override
    {
        if (event.key() == Qt::Key_Delete) {
            handled = m_visual->m_scene->deleteSelectedWidgets();

            if (handled)
                return true;
        }

        return QDockWidget::keyReleaseEvent(event);
    }
#endif
};

} // namespace hierarchy_dock_widget
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
