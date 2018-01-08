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

#include "metaimageset_inputs_qsvg.h"

#include "metaimageset/metaimageset_init.h"

#include "ceed_paths.h"
#include "elementtree.h"

#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QtSvg/QSvgRenderer>

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace qsvg {

void QSVG::loadFromElement(ElementTree::Element *element)
{
    m_path = element->get("path", "");
    m_xOffset = element->get("xOffset", "0").toInt();
    m_yOffset = element->get("yOffset", "0").toInt();
}

ElementTree::Element *QSVG::saveToElement()
{
    auto ret = new ElementTree::Element("QSVG");
    ret->set("path", m_path);
    ret->set("xOffset", QString::number(m_xOffset));
    ret->set("yOffset", QString::number(m_yOffset));

    return ret;
}

QString QSVG::getDescription()
{
    return QString("QSvg '%1'").arg(m_path);
}

QList<Image *> QSVG::buildImages()
{
#if 1
    QStringList paths = glob.glob(os.path.dirname(m_metaImageset->m_filePath), m_path);
#else
    QStringList paths = glob.glob(os.path.join(os.path.dirname(m_metaImageset->m_filePath), m_path));
#endif

    QList<inputs::Image*> images;
    for (QString path : paths) {
#if 1
        QString name = QFileInfo(path).completeBaseName();
#else
        QStringList pathSplit = path.rsplit(".", 1);
        QString name = os.path.basename(pathSplit[0]);
#endif

        QSvgRenderer svgRenderer(path);
        QImage qimage(svgRenderer.defaultSize().width(), svgRenderer.defaultSize().height(), QImage::Format_ARGB32);
        qimage.fill(0);
        QPainter painter;
        painter.begin(&qimage);
        svgRenderer.render(&painter);
        painter.end();

        auto image = new inputs::Image(name, qimage, m_xOffset, m_yOffset);
        images.append(image);
    }

    return images;
}


} // namesapce qsvg
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED
