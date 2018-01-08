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

#include "metaimageset_inputs_registry.h"

#include "metaimageset/metaimageset_init.h"

#include "metaimageset/inputs/metaimageset_inputs_bitmap.h"
#include "metaimageset/inputs/metaimageset_inputs_imageset.h"
#include "metaimageset/inputs/metaimageset_inputs_qsvg.h"
#include "metaimageset/inputs/metaimageset_inputs_inkscape_svg.h"

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace registry {

void loadFromElement(MetaImageset *metaImageset, ElementTree::Element *element)
{
    for (auto childElement : element->findall("Imageset")) {
        auto im = new imageset::Imageset(metaImageset);
        im->loadFromElement(childElement);

        metaImageset->m_inputs.append(im);
    }

    for (auto childElement : element->findall("Bitmap")) {
        auto b = new bitmap::Bitmap(metaImageset);
        b->loadFromElement(childElement);

        metaImageset->m_inputs.append(b);
    }

    for (auto childElement : element->findall("QSVG")) {
        auto svg = new qsvg::QSVG(metaImageset);
        svg->loadFromElement(childElement);

        metaImageset->m_inputs.append(svg);
    }

    for (auto childElement : element->findall("InkscapeSVG")) {
        auto svg = new inkscape_svg::InkscapeSVG(metaImageset);
        svg->loadFromElement(childElement);

        metaImageset->m_inputs.append(svg);
    }
}


} // namesapce registry
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED
