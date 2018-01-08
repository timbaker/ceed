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

#ifndef CEED_metaimageset_inputs_imageset_
#define CEED_metaimageset_inputs_imageset_

#include "CEEDBase.h"

/**Implements CEGUI imagesets as inputs of metaimageset.
*/

#include "metaimageset/inputs/metaimageset_inputs_init.h"

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace imageset {

class Imageset : public inputs::Input
{
public:
    QString m_filePath;
    editors::imageset::elements::ImagesetEntry* m_imagesetEntry;

    Imageset(MetaImageset* metaImageset):
        inputs::Input(metaImageset)
    {
        m_filePath = "";
        m_imagesetEntry = nullptr;
    }

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    QString getDescription();

    QList<Image*> buildImages();
};

} // namesapce imageset
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
