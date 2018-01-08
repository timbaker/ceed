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

#include "editor_looknfeel_settings_decl.h"

#include "settings/settings_init.h"

#include <QColor>
#include <QPen>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace settings_decl {

void declare(settings::Settings *settings_)
{
    settings::declaration::Category* category = settings_->createCategory("looknfeel", "Look n' Feel editing");

    auto& visual_ = *category->createSection("visual", "Visual editing");

    // FIXME: Only applies to newly switched to visual modes!
    visual_.createEntry("continuous_rendering", QVariant::Bool, false, "Continuous rendering",
                       "Check this if you are experiencing redraw issues (your skin contains an idle animation or such).\nOnly applies to newly switched to visual modes so switch to Code mode or back or restart the application for this to take effect.",
                       "checkbox",
                       -1);

    visual_.createEntry("prevent_manipulator_overlap", QVariant::Bool, false, "Prevent manipulator overlap",
                       "Only enable if you have a very fast computer and only edit small Look n' Feels. Very performance intensive!",
                       "checkbox",
                       0);

    visual_.createEntry("normal_outline", QVariant::Pen, QPen(QColor(255, 255, 0, 255)), "Normal outline",
                       "Pen for normal outline.",
                       "pen",
                       1);

    visual_.createEntry("hover_outline", QVariant::Pen, QPen(QColor(0, 255, 255, 255)), "Hover outline",
                       "Pen for hover outline.",
                       "pen",
                       2);

    visual_.createEntry("resizing_outline", QVariant::Pen, QPen(QColor(255, 0, 255, 255)), "Outline while resizing",
                       "Pen for resizing outline.",
                       "pen",
                       3);

    visual_.createEntry("moving_outline", QVariant::Pen, QPen(QColor(255, 0, 255, 255)), "Outline while moving",
                       "Pen for moving outline.",
                       "pen",
                       4);

    visual_.createEntry("snap_grid_x", QVariant::Double, 5, "Snap grid cell width (X)",
                       "Snap grid X metric.",
                       "float",
                       5);

    visual_.createEntry("snap_grid_y", QVariant::Double, 5, "Snap grid cell height (Y)",
                       "Snap grid Y metric.",
                       "float",
                       6);

    visual_.createEntry("snap_grid_point_colour", QVariant::Color, QColor(255, 255, 255, 192), "Snap grid point colour",
                       "Color of snap grid points.",
                       "colour",
                       7);

    visual_.createEntry("snap_grid_point_shadow_colour", QVariant::Color, QColor(64, 64, 64, 192), "Snap grid point shadow colour",
                       "Color of snap grid points (shadows).",
                       "colour",
                       8);

    // TODO: Full restart is not actually needed, just a refresh on all Look n' Feel visual editing modes
    visual_.createEntry("hide_deadend_autowidgets", QVariant::Bool, true, "Hide deadend auto widgets",
                       "Should auto widgets with no non-auto widgets descendants be hidden in the widget hierarchy?",
                       "checkbox",
                       9, true);

    // FIXME: Only applies to newly refreshed visual modes!
    visual_.createEntry("auto_widgets_selectable", QVariant::Bool, false, "Make auto widgets selectable",
                       "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to deal with them at all. Only for EXPERT use! Will make CEED crash in cases where you don't know what you are doing!",
                       "checkbox",
                       9);

    // FIXME: Only applies to newly refreshed visual modes!
    visual_.createEntry("auto_widgets_show_outline", QVariant::Bool, false, "Show outline of auto widgets",
                       "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to deal with them at all. Only use if you know what you are doing! This might clutter the interface a lot.",
                       "checkbox",
                       10);
}

} // namespace settings_decl
} // namespace looknfeel
} // namespace editors
} // namespace CEED
