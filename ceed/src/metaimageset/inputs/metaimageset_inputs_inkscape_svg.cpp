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

#include "metaimageset_inputs_inkscape_svg.h"

#include "metaimageset/metaimageset_init.h"

#include "ceed_paths.h"
#include "elementtree.h"

#include <QImage>
#include <QProcess>
#include <QTemporaryFile>

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace inkscape_svg {

QStringList getAllSVGLayers(const QString &svgPath)
{
    QStringList ret;

    ElementTree::ElementTree doc(svgPath);
    for (auto g : doc.findall(".//{http://www.w3.org/2000/svg}g")) {
        if (g->get("{http://www.inkscape.org/namespaces/inkscape}groupmode") == "layer") {
            ret.append(g->get("{http://www.inkscape.org/namespaces/inkscape}label"));
        }
    }

    return ret;
}

void showOnlySVGLayers(const QString &svgPath, const QStringList &layers, QIODevice *targetSvg)
{
    ElementTree::ElementTree doc(svgPath);
    for (auto g : doc.findall(".//{http://www.w3.org/2000/svg}g")) {
        if (g->get("{http://www.inkscape.org/namespaces/inkscape}groupmode") == "layer") {
            if (layers.contains(g->get("{http://www.inkscape.org/namespaces/inkscape}label")))
                g->set("style", "display:inline");
            else
                g->set("style", "display:none");
        }
    }

    doc.write(*targetSvg, "utf-8");
}

void exportSVG(const QString &svgPath, const QStringList &layers, const QString &targetPngPath)
{
    QSet<QString> allLayers = getAllSVGLayers(svgPath).toSet();
    for (QString layer : layers) {
        if (!allLayers.contains(layer)) {
            throw RuntimeError(QString("Can't export with layer \"%1\", it isn't defined in the SVG \"%2\"!").arg(layer).arg(svgPath));
        }
    }

    QTemporaryFile temporarySvg("ceed-XXXXXX.svg");
    Q_ASSERT(temporarySvg.open() == true);
    showOnlySVGLayers(svgPath, layers, &temporarySvg);
    temporarySvg.close();

    QStringList cmdLine = { QString("--file=%1").arg(temporarySvg.fileName()), QString("--export-png=%1").arg(targetPngPath) };
    QProcess p;
    p.start(INKSCAPE_PATH, cmdLine);
    if (p.waitForStarted()) {
        if (p.waitForFinished()) {
            QByteArray output = p.readAll();
        }
    }
    // FIXME: debug logging of stdout?
}

/////

Component::Component(InkscapeSVG *svg, const QString &name, int x, int y, int width, int height, const QString &layers, int xOffset, int yOffset)
{
    m_svg = svg;

    m_name = name;

    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;

    m_xOffset = xOffset;
    m_yOffset = yOffset;

    m_layers = layers.split(" ");

    m_cachedImages = nullptr;
}

void Component::loadFromElement(ElementTree::Element *element)
{
    m_name = element->get("name", "");

    m_x = element->get("x", "0").toInt();
    m_y = element->get("y", "0").toInt();
    m_width = element->get("width", "1").toInt();
    m_height = element->get("height", "1").toInt();

    m_xOffset = element->get("xOffset", "0").toInt();
    m_yOffset = element->get("yOffset", "0").toInt();

    m_layers = element->get("layers", "").split(" ");
}

ElementTree::Element *Component::saveToElement()
{
    auto ret = new ElementTree::Element("Component");

    ret->set("name", m_name);

    ret->set("x", QString::number(m_x));
    ret->set("y", QString::number(m_y));
    ret->set("width", QString::number(m_width));
    ret->set("height", QString::number(m_height));

    ret->set("xOffset", QString::number(m_xOffset));
    ret->set("yOffset", QString::number(m_yOffset));

    ret->set("layers", m_layers.join(" "));

    return ret;
}

QImage Component::generateQImage()
{
    QString full_path = os.path.join(os.path.dirname(m_svg->m_metaImageset->m_filePath), m_svg->m_path);

    QTemporaryFile temporaryPng("ceed-XXXXXX.png");
    Q_ASSERT(temporaryPng.open() == true);
    exportSVG(full_path, m_layers, temporaryPng.fileName());
    temporaryPng.close();

    QImage qimage(temporaryPng.fileName());
    return qimage.copy(m_x, m_y, m_width, m_height);
}

QList<Image *> Component::buildImages()
{
    // FIXME: This is a really nasty optimisation, it can be done way better
    //        but would probably require a slight redesign of inputs.Input
    if (m_cachedImages == nullptr)
        m_cachedImages = new QList<inputs::Image*>( { new inputs::Image(m_name, generateQImage(), m_xOffset, m_yOffset) });

    return *m_cachedImages;
}

/////

FrameComponent::FrameComponent(InkscapeSVG *svg, const QString &name, int x, int y, int width, int height, int cornerWidth, int cornerHeight, const QString &layers, const QString &skip)
{
    m_svg = svg;

    m_name = name;

    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;

    m_cornerWidth = cornerWidth;
    m_cornerHeight = cornerHeight;

    m_layers = layers.split(" ");
    m_skip = skip.split(" ");

    m_cachedImages = nullptr;
    m_cachedQImage = nullptr;
}

void FrameComponent::loadFromElement(ElementTree::Element *element)
{
    m_name = element->get("name", "");

    m_x = element->get("x", "0").toInt();
    m_y = element->get("y", "0").toInt();
    m_width = element->get("width", "1").toInt();
    m_height = element->get("height", "1").toInt();

    m_cornerWidth = element->get("cornerWidth", "1").toInt();
    m_cornerHeight = element->get("cornerHeight", "1").toInt();

    m_layers = element->get("layers", "").split(" ");
    m_skip = element->get("skip", "").split(" ");
}

ElementTree::Element *FrameComponent::saveToElement()
{
    auto ret = new ElementTree::Element("Component");

    ret->set("name", m_name);

    ret->set("x", QString::number(m_x));
    ret->set("y", QString::number(m_y));
    ret->set("width", QString::number(m_width));
    ret->set("height", QString::number(m_height));

    ret->set("cornerWidth", QString::number(m_cornerWidth));
    ret->set("cornerHeight", QString::number(m_cornerHeight));

    ret->set("layers", m_layers.join(" "));
    ret->set("skip", m_layers.join(" "));

    return ret;
}

QImage FrameComponent::generateQImage(int x, int y, int width, int height)
{
    if (m_cachedQImage == nullptr) {
        QString full_path = os.path.join(os.path.dirname(m_svg->m_metaImageset->m_filePath), m_svg->m_path);

        QTemporaryFile temporaryPng("ceed-XXXXXX.png");
        Q_ASSERT(temporaryPng.open() == true);
        exportSVG(full_path, m_layers, temporaryPng.fileName());

        m_cachedQImage = new QImage(temporaryPng.fileName());
    }

    return m_cachedQImage->copy(x, y, width, height);
}

QList<Image *> FrameComponent::buildImages()
{
    // FIXME: This is a really nasty optimisation, it can be done way better
    //        but would probably require a slight redesign of inputs.Input
    if (m_cachedImages == nullptr) {
        m_cachedImages = new QList<inputs::Image*>();

        if (!m_skip.contains("centre")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "Centre",
                                          generateQImage(
                                              m_x + m_cornerWidth + 1, m_y + m_cornerHeight + 1,
                                              m_width - 2 * m_cornerWidth - 2, m_height - 2 * m_cornerHeight - 2),
                                          0, 0)
                        );
        }

        if (!m_skip.contains("top")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "Top",
                                          generateQImage(
                                              m_x + m_cornerWidth + 1, m_y,
                                              m_width - 2 * m_cornerWidth - 2, m_cornerHeight),
                                          0, 0)
                        );
        }

        if (!m_skip.contains("bottom")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "Bottom",
                                          generateQImage(
                                              m_x + m_cornerWidth + 1, m_y + m_height - m_cornerHeight,
                                              m_width - 2 * m_cornerWidth - 2, m_cornerHeight),
                                          0, 0)
                        );
        }

        if (!m_skip.contains("left")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "Left",
                                          generateQImage(
                                              m_x, m_y + m_cornerHeight + 1,
                                              m_cornerWidth, m_height - 2 * m_cornerHeight - 2),
                                          0, 0)
                        );
        }

        if (!m_skip.contains("right")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "Right",
                                          generateQImage(
                                              m_x + m_width - m_cornerWidth, m_y + m_cornerHeight + 1,
                                              m_cornerWidth, m_height - 2 * m_cornerHeight - 2),
                                          0, 0)
                        );
        }

        if (!m_skip.contains("topLeft")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "TopLeft",
                                      generateQImage(
                                          m_x, m_y,
                                          m_cornerWidth, m_cornerHeight),
                                      0, 0)
                        );
        }

        if (!m_skip.contains("topRight")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "TopRight",
                                      generateQImage(
                                          m_x + m_width - m_cornerWidth, m_y,
                                          m_cornerWidth, m_cornerHeight),
                                      0, 0)
                        );
        }

        if (!m_skip.contains("bottomLeft")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "BottomLeft",
                                      generateQImage(
                                          m_x, m_y + m_height - m_cornerHeight,
                                          m_cornerWidth, m_cornerHeight),
                                      0, 0)
                        );
        }

        if (!m_skip.contains("bottomRight")) {
            m_cachedImages->append(
                        new inputs::Image(m_name + "BottomRight",
                                      generateQImage(
                                          m_x + m_width - m_cornerWidth, m_y + m_height - m_cornerHeight,
                                          m_cornerWidth, m_cornerHeight),
                                      0, 0)
                        );
        }
    }

    return *m_cachedImages;
}

void InkscapeSVG::loadFromElement(ElementTree::Element *element)
{
    m_path = element->get("path", "");

    for (auto componentElement : element->findall("Component")) {
        auto component = new Component(this);
        component->loadFromElement(componentElement);

        m_components.append(component);
    }

    // FrameComponent is a shortcut to avoid having to type out 9 components
    for (auto componentElement : element->findall("FrameComponent")) {
        auto component = new FrameComponent(this);
        component->loadFromElement(componentElement);

        m_components.append(component);
    }
}

ElementTree::Element *InkscapeSVG::saveToElement()
{
    auto ret = new ElementTree::Element("InkscapeSVG");
    ret->set("path", m_path);

    for (auto component : m_components)
        ret->append(component->saveToElement());

    return ret;
}

QString InkscapeSVG::getDescription()
{
    return QString("Inkscape SVG '%1' with %2 components").arg(m_path).arg(m_components.length());
}

QList<Image *> InkscapeSVG::buildImages()
{
    QList<inputs::Image*> ret;

    for (auto component : m_components)
        ret += component->buildImages();

    return ret;
}


} // namesapce inkscape_svg
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED
