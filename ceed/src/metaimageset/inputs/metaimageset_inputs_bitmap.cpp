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

#include "metaimageset_inputs_bitmap.h"

#include "metaimageset/metaimageset_init.h"

#include "ceed_paths.h"
#include "elementtree.h"

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace bitmap {

void Bitmap::loadFromElement(ElementTree::Element *element)
{
    m_path = element->get("path", "");
    m_xOffset = element->get("xOffset", "0").toInt();
    m_yOffset = element->get("yOffset", "0").toInt();
}

ElementTree::Element *Bitmap::saveToElement()
{
    auto ret = new ElementTree::Element("Bitmap");
    ret->set("path", m_path);
    ret->set("xOffset", QString::number(m_xOffset));
    ret->set("yOffset", QString::number(m_yOffset));

    return ret;
}

QString Bitmap::getDescription()
{
    return QString("Bitmap image(s) '%1'").arg(m_path);
}

QList<Image *> Bitmap::buildImages()
{
#if 1
    QStringList paths = glob.glob(os.path.dirname(m_metaImageset->m_filePath), m_path);
#else
    QStringList paths = glob.glob(os.path.join(os.path.dirname(m_metaImageset->m_filePath), m_path));
#endif

    QList<Image*> images;
    for (QString path : paths) {
#if 1
        QString name = QFileInfo(path).completeBaseName();
#else
        QStringList pathSplit = path.rsplit(".", 1);
        QString name = os.path.basename(pathSplit[0]);
#endif

        auto image = new inputs::Image(name, QImage(path), m_xOffset, m_yOffset);
        images.append(image);
    }

    return images;
}

} // namespace bitmap
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED
