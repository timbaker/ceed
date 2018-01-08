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

#include "editor_layout_action_decl.h"

#include "CEEDBase.h"

#include "action/action__init__.h"
#include "action/declaration.h"

namespace CEED {
namespace editors {
namespace layout {
namespace action_decl {

void declare(action::ActionManager *actionManager)
{
    action::declaration::ActionCategory* cat = actionManager->createCategory("layout", "Layout Editor");

    cat->createAction("align_hleft", "Align &Left (horizontally)",
                      "Sets horizontal alignment of all selected widgets to left.",
                      QIcon(":/icons/layout_editing/align_hleft.png"));
    cat->createAction("align_hcentre", "Align Centre (&horizontally)",
                      "Sets horizontal alignment of all selected widgets to centre.",
                      QIcon(":/icons/layout_editing/align_hcentre.png"));
    cat->createAction("align_hright", "Align &Right (horizontally)",
                      "Sets horizontal alignment of all selected widgets to right.",
                      QIcon(":/icons/layout_editing/align_hright.png"));

    cat->createAction("align_vtop", "Align &Top (vertically)",
                      "Sets vertical alignment of all selected widgets to top.",
                      QIcon(":/icons/layout_editing/align_vtop.png"));
    cat->createAction("align_vcentre", "Align Centre (&vertically)",
                      "Sets vertical alignment of all selected widgets to centre.",
                      QIcon(":/icons/layout_editing/align_vcentre.png"));
    cat->createAction("align_vbottom", "Align &Bottom (vertically)",
                      "Sets vertical alignment of all selected widgets to bottom.",
                      QIcon(":/icons/layout_editing/align_vbottom.png"));

    cat->createAction("snap_grid", "Snap to &Grid",
                      "When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see settings for snap grid related entries), also shows the snap grid if checked.",
                      QIcon(":/icons/layout_editing/snap_grid.png"),
                      QKeySequence(Qt::Key_Space))->setCheckable(true);

    auto* absolute_mode = cat->createAction(
                "absolute_mode", "&Absolute Resizing && Moving Deltas",
                "When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                QIcon(":/icons/layout_editing/absolute_mode.png"),
                QKeySequence(Qt::Key_A));
    absolute_mode->setCheckable(true);
    absolute_mode->setChecked(true);

    auto* abs_integers_mode = cat->createAction(
                "abs_integers_mode", "Only Increase/Decrease by Integers When Moving or Resizing",
                "If checked, while resizing or moving widgets in the editor only integer values (e.g. no"
                "0.25 or 0.5 etc.) will be added to the current absolute values. This is only relevant if editing"
                "in zoomed-in view while 'Absolute Resizing and Moving' is activated.",
                QIcon(":/icons/layout_editing/abs_integers_mode.png"),
                QKeySequence(Qt::Key_Q));
    abs_integers_mode->setCheckable(true);
    abs_integers_mode->setChecked(true);

    cat->createAction("normalise_position", "Normalise &Position (cycle)",
                      "If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                      QIcon(":/icons/layout_editing/normalise_position.png"),
                      QKeySequence(Qt::Key_D));

    cat->createAction("normalise_size", "Normalise &Size (cycle)",
                      "If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                      QIcon(":/icons/layout_editing/normalise_size.png"),
                      QKeySequence(Qt::Key_S));

    cat->createAction("round_position", "Rounds the absolute position to nearest integer",
                      "The value of the absolute position will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                      QIcon(":/icons/layout_editing/round_position.png"),
                      QKeySequence(Qt::Key_M));
    cat->createAction("round_size", "Rounds the absolute size to nearest integer",
                      "The value of the absolute size will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                      QIcon(":/icons/layout_editing/round_size.png"),
                      QKeySequence(Qt::Key_N));

    cat->createAction("move_backward_in_parent_list", "Moves widget -1 step in the parent's widget list",
                      "Moves selected widget(s) one step backward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                      QIcon(":/icons/layout_editing/move_backward_in_parent_list.png"));
    cat->createAction("move_forward_in_parent_list", "Moves widget +1 step in the parent's widget list",
                      "Moves selected widget(s) one step forward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                      QIcon(":/icons/layout_editing/move_forward_in_parent_list.png"));

    cat->createAction("focus_property_inspector_filter_box", "&Focus Property Inspector Filter Box",
                      "This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                      QIcon(":/icons/layout_editing/focus_property_inspector_filter_box.png"),
                      QKeySequence(QKeySequence::Find));

    cat->createAction("copy_widget_path", "C&opy Widget Paths",
                      "Copies the 'NamePath' properties of the selected widgets to the clipboard.",
                      QIcon(":/icons/actions/copy.png"));

    cat->createAction("rename", "&Rename Widget",
                      "Edits the selected widget's name.",
                      QIcon(":/icons/layout_editing/rename.png"));

    cat->createAction("lock_widget", "&Lock Widget",
                      "Locks the widget for moving and resizing in the visual editing mode.",
                      QIcon(":/icons/layout_editing/lock_widget.png"));

    cat->createAction("unlock_widget", "&Unlock Widget",
                      "Unlocks the widget for moving and resizing in the visual editing mode.",
                      QIcon(":/icons/layout_editing/unlock_widget.png"));

    cat->createAction("recursively_lock_widget", "&Lock Widget (recursively)",
                      "Locks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                      QIcon(":/icons/layout_editing/lock_widget_recursively.png"));

    cat->createAction("recursively_unlock_widget", "&Unlock Widget (recursively)",
                      "Unlocks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                      QIcon(":/icons/layout_editing/unlock_widget_recursively.png"));
}

} // namespae action_decl
} // namespace layout
} // namespace editors
} // namespace CEED
