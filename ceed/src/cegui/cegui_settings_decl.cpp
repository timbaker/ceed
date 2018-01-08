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

#include "cegui_settings_decl.h"

#include "settings/settings_init.h"
#include "settings/settings_declaration.h"

#include <QColor>

namespace CEED {
namespace cegui {
namespace settings_decl {

void declare(settings::declaration::Settings *settings)
{
    using CEED::settings::declaration::Section;
    using CEED::settings::declaration::Category;
    using CEED::settings::declaration::Entry;

    Category* category = settings->createCategory("cegui", "Embedded CEGUI");

    Section* background = category->createSection("background", "Rendering background (checkerboard)");

    background->createEntry("checker_width", QVariant::Int, 10, "Width of the checkers",
                            "Width of one of the checkers.",
                            "int",
                            1);

    background->createEntry("checker_height", QVariant::Int, 10, "Height of the checkers",
                            "Height of one of the checkers.",
                            "int",
                            2);

    background->createEntry("first_colour", QVariant::Color, QColor(Qt::darkGray), "First colour",
                            "First of the alternating colours to use.",
                            "colour",
                            3);

    background->createEntry("second_colour", QVariant::Color, QColor(Qt::lightGray), "Second colour",
                            "Second of the alternating colours to use. (use the same as first to get a solid background)",
                            "colour",
                            4);
}


} // naespace settings_decl
} // namspace cegui
} // namespace CEED
