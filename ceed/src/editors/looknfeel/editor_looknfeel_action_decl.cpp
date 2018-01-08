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

#include "editor_looknfeel_action_decl.h"

#include "action/action__init__.h"
#include "action/declaration.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace action_decl {

void declare(action::ActionManager *actionManager)
{
    auto& cat = *actionManager->createCategory(/*name=*/"looknfeel", /*label=*/"Look n' Feel Editor");
#if 0 // copy/pasted from layout editor
    cat.createAction(/*name=*/"align_hleft", /*label=*/"Align &Left (horizontally)",
                     /*help=*/"Sets horizontal alignment of all selected widgets to left.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_hleft.png"));
    cat.createAction(/*name=*/"align_hcentre", /*label=*/"Align Centre (&horizontally)",
                     /*help=*/"Sets horizontal alignment of all selected widgets to centre.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_hcentre.png"));
    cat.createAction(/*name=*/"align_hright", /*label=*/"Align &Right (horizontally)",
                     /*help=*/"Sets horizontal alignment of all selected widgets to right.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_hright.png"));

    cat.createAction(/*name=*/"align_vtop", /*label=*/"Align &Top (vertically)",
                     /*help=*/"Sets vertical alignment of all selected widgets to top.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_vtop.png"));
    cat.createAction(/*name=*/"align_vcentre", /*label=*/"Align Centre (&vertically)",
                     /*help=*/"Sets vertical alignment of all selected widgets to centre.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_vcentre.png"));
    cat.createAction(/*name=*/"align_vbottom", /*label=*/"Align &Bottom (vertically)",
                     /*help=*/"Sets vertical alignment of all selected widgets to bottom.",
                     /*icon=*/QIcon("icons/looknfeel_editing/align_vbottom.png"));

    cat.createAction(/*name=*/"snap_grid", /*label=*/"Snap to &Grid",
                     /*help=*/"When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see settings for snap grid related entries), also shows the snap grid if checked.",
                     /*icon=*/QIcon("icons/looknfeel_editing/snap_grid.png"),
                     /*defaultShortcut = */QKeySequence(Qt::Key_Space))->setCheckable(true);

    auto absolute_mode = cat.createAction(
                /*name=*/"absolute_mode", /*label=*/"&Absolute Resizing && Moving Deltas",
                /*help=*/"When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                /*icon=*/QIcon("icons/looknfeel_editing/absolute_mode.png"),
                /*defaultShortcut = */QKeySequence(Qt::Key_A));
    absolute_mode->setCheckable(true);
    absolute_mode->setChecked(true);

    cat.createAction(/*name=*/"normalise_position", /*label=*/"Normalise &Position (cycle)",
                     /*help=*/"If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                     /*icon=*/QIcon("icons/looknfeel_editing/normalise_position.png"),
                     /*defaultShortcut = */QKeySequence(Qt::Key_D));

    cat.createAction(/*name=*/"normalise_size", /*label=*/"Normalise &Size (cycle)",
                     /*help=*/"If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                     /*icon=*/QIcon("icons/looknfeel_editing/normalise_size.png"),
                     /*defaultShortcut = */QKeySequence(Qt::Key_S));
#endif
    cat.createAction(/*name=*/"focus_element_editor_filter_box", /*label=*/"&Focus Element Editor Filter Box",
                     /*help=*/"This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                     /*icon=*/QIcon("icons/looknfeel_editing/focus_property_inspector_filter_box.png"),
                     /*defaultShortcut = */QKeySequence(QKeySequence::Find));
}

} // namespace action_decl
} // namespace looknfeel
} // namespace editors
} // namespace CEED
