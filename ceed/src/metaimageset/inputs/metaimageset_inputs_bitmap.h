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

#ifndef CEED_metaimageset_inputs_bitmap_
#define CEED_metaimageset_inputs_bitmap_

#include "CEEDBase.h"

#include "metaimageset/inputs/metaimageset_inputs_init.h"

/**Implements simple bitmaps as inputs of metaimageset.
*/

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace bitmap {

class Bitmap : public inputs::Input
{
public:
    QString m_path;
    int m_xOffset;
    int m_yOffset;

    Bitmap(MetaImageset* metaImageset):
        inputs::Input(metaImageset)
    {
        m_path = "";

        m_xOffset = 0;
        m_yOffset = 0;
    }

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    QString getDescription();

    QList<Image*> buildImages();
};

} // namespace bitmap
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
