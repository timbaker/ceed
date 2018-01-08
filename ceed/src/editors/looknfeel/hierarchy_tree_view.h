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

#ifndef CEED_editors_looknfeel_hierarchy_tree_view_
#define CEED_editors_looknfeel_hierarchy_tree_view_

#include "CEEDBase.h"

#include "action/action__init__.h"

#include "editors/looknfeel/hierarchy_tree_item.h"

#include <QTreeView>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_view {

/*!
\brief LookNFeelHierarchyTreeView

The WidgetLookFeel hierarchy tree definition
    This is based on the QTreeWidget widget.

*/
class LookNFeelHierarchyTreeView : public QTreeView
{
    typedef QTreeView super;
public:
    hierarchy_dock_widget::LookNFeelHierarchyDockWidget* m_dockWidget;
    QColor m_originalBackgroundColour;
    QMenu* m_contextMenu;
    action::declaration::Action* m_cutAction;
    action::declaration::Action* m_copyAction;
    action::declaration::Action* m_pasteAction;
    action::declaration::Action* m_deleteAction;

    LookNFeelHierarchyTreeView(QWidget* parent=nullptr);

    /**Draws alternating background colours for the items, changing the colour depending on the category.
    */
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;

    /**Reacts on selection changes in the hierarchy tree by notifying the Falagrd element editor of the new selection.
    */
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;


    void setupContextMenu();

    void contextMenuEvent(QContextMenuEvent *event) override;
};

} // namespace hierarchy_tree_view
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
