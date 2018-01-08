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

#ifndef CEED_metaimageset_inputs___init___
#define CEED_metaimageset_inputs___init___

#include "CEEDBase.h"

#include "elementtree.h"

#include <QImage>

/**Defines interfaces for metaimageset inputs and the images that are returned
from these inputs.
*/

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {

/*!
\brief Image

Instance of the image, containing a bitmap (QImage)
    and xOffset and yOffset

*/
class Image
{
public:
    QString m_name;
    QImage m_qimage;
    int m_xOffset;
    int m_yOffset;

    Image(const QString& name, QImage qimage, int xOffset = 0, int yOffset = 0)
    {
        m_name = name;

        m_qimage = qimage;

        // X and Y offsets are related to the "crosshair" effect for images
        // moving the origin of image around...
        // These have nothing to do with packing and the final x, y position
        // on the texture atlas.
        m_xOffset = xOffset;
        m_yOffset = yOffset;
    }
};

/*!
\brief Input

Describes any input image source for the meta imageset.

    This can be imageset, bitmap image, SVG image, ...

*/
class Input
{
public:
    MetaImageset* m_metaImageset;

    /**metaImageset - the parent MetaImageset class*/
    Input(MetaImageset* metaImageset)
    {
        m_metaImageset = metaImageset;
    }

    virtual void loadFromElement(ElementTree::Element* element) = 0;

    virtual ElementTree::Element* saveToElement() = 0;

    virtual QString getDescription() = 0;

    /**Retrieves list of Image objects each containing a bitmap representation
    of some image this input provided, xOffset and yOffset.

    For simple images, this will return [ImageInstance(QImage(m_path))],
    For imagesets, this will return list of all images in the imageset
    (Each QImage containing only the specified portion of the underlying image)
    */
    virtual QList<Image*> buildImages() = 0;
};

} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
