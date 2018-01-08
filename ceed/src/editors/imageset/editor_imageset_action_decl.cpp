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

#include "editor_imageset_action_decl.h"

#include "action/action__init__.h"

namespace CEED {
namespace editors {
namespace imageset {
namespace action_decl {

void declare(action::ActionManager *actionManager)
{
    action::declaration::ActionCategory* cat = actionManager->createCategory("imageset", "Imageset Editor");

    cat->createAction("edit_offsets", "Edit &Offsets",
                      "When you select an image definition, a crosshair will appear in it representing it's offset centrepoint.",
                      QIcon(":icons/imageset_editing/edit_offsets.png"),
                      QKeySequence(Qt::Key_Space))->setCheckable(true);

    cat->createAction("cycle_overlapping", "Cycle O&verlapping Image Definitions",
                      "When images definition overlap in such a way that makes it hard/impossible to select the definition you want, this allows you to select on of them and then just cycle until the right one is selected.",
                      QIcon(":icons/imageset_editing/cycle_overlapping.png"),
                      QKeySequence(Qt::Key_Q));

    cat->createAction("create_image", "&Create Image Definition",
                      "Creates a new image definition at the current cursor position, sized 50x50 pixels.",
                      QIcon(":icons/imageset_editing/create_image.png"));

    cat->createAction("duplicate_image", "&Duplicate Image Definition",
                      "Duplicates selected image definitions.",
                      QIcon(":icons/imageset_editing/duplicate_image.png"));

    cat->createAction("focus_image_list_filter_box", "&Focus Image Definition List Filter Box",
                      "This allows you to easily press a shortcut and immediately search through image definitions without having to reach for a mouse.",
                      QIcon(":icons/imageset_editing/focus_image_list_filter_box.png"),
                      QKeySequence(QKeySequence::Find));
}


} // namespace action_decl
} // namespace imageset
} // namespace editors
} // namespace CEED
