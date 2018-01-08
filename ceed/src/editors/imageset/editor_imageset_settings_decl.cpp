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

#include "editor_imageset_settings_decl.h"

#include "settings/settings_init.h"

namespace CEED {
namespace editors {
namespace imageset {
namespace settings_decl {

void declare(settings::Settings *settings)
{
    settings::declaration::Category* category = settings->createCategory("imageset", "Imageset editing");

    settings::declaration::Section* visual = category->createSection("visual", "Visual editing");

    visual->createEntry("overlay_image_labels", QVariant::Bool, true, "Show overlay labels of images",
                        "Show overlay labels of images.",
                        "checkbox",
                        1);

    visual->createEntry("partial_updates", QVariant::Bool, false, "Use partial drawing updates",
                        "Will use partial 2D updates using accelerated 2D machinery. The performance of this is very dependent on your platform and hardware. MacOSX handles partial updates much better than Linux it seems. If you have a very good GPU, don't tick this.",
                        "checkbox",
                        2, true);
}

} // namespace settings_decl
} // namespace imageset
} // namespace editors
} // namespace CEED
