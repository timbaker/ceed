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

#include "metaimageset_inputs_imageset.h"

#include "metaimageset/metaimageset_init.h"

#include "editors/imageset/editor_imageset_elements.h"
#include "compatibility/imageset/compat_imageset_init.h"

#include "editors/imageset/editor_imageset_visual.h"

#include "ceed_paths.h"
#include "elementtree.h"

#include <QFile>

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace imageset {

class FakeImagesetEntry : public editors::imageset::elements::ImagesetEntry
{
public:
    class FakeVisual : public editors::imageset::visual::FakeVisual
    {
    public:
        void refreshSceneRect()
        {
        }
    };

    QString m_filePath;

    FakeImagesetEntry(const QString& filePath)
        : editors::imageset::elements::ImagesetEntry(new FakeVisual())
    {
        m_filePath = filePath;
    }

    QString getAbsoluteImageFile()
    {
        return os.path.join(os.path.dirname(m_filePath), m_imageFile);
    }
};

void Imageset::loadFromElement(ElementTree::Element *element)
{
    m_filePath = os.path.join(os.path.dirname(m_metaImageset->m_filePath), element->get("path", ""));

    QFile file(m_filePath);
    Q_ASSERT(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray rawData = file.readAll();
    file.close();
    QString nativeData = compatibility::imageset::manager->transformTo(compatibility::imageset::manager->EditorNativeType, QString::fromUtf8(rawData), m_filePath);

    element = ElementTree::fromstring(nativeData);

    m_imagesetEntry = new FakeImagesetEntry(m_filePath);
    m_imagesetEntry->loadFromElement(element);
}

ElementTree::Element *Imageset::saveToElement()
{
    auto ret = new ElementTree::Element("Imageset");
    ret->set("path", m_filePath);

    return ret;
}

QString Imageset::getDescription()
{
    return QString("Imageset '%1'").arg(m_filePath);
}

QList<Image *> Imageset::buildImages()
{
    Q_ASSERT(m_imagesetEntry != nullptr);

    QList<Image*> ret;

    QImage entireImage = QImage(m_imagesetEntry->getAbsoluteImageFile());

    for (auto imageEntry : m_imagesetEntry->m_imageEntries) {
        QImage subImage = entireImage.copy(imageEntry->xpos(), imageEntry->ypos(), imageEntry->width(), imageEntry->height());

        ret.append(new inputs::Image(m_imagesetEntry->m_name + "/" + imageEntry->name(), subImage, imageEntry->xoffset(), imageEntry->yoffset()));
    }

    return ret;
}


} // namesapce imageset
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED
