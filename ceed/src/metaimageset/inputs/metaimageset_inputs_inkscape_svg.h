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

#ifndef CEED_metaimageset_inputs_inkscape_svg_
#define CEED_metaimageset_inputs_inkscape_svg_

#include "CEEDBase.h"

/**Implements the more advanced SVG input of metaimageset (using Inkscape).
*/

#include "metaimageset/inputs/metaimageset_inputs_init.h"

namespace CEED {
namespace editors {
namespace metaimageset {
namespace inputs {
namespace inkscape_svg {

class InkscapeSVG;

const QString INKSCAPE_PATH = "inkscape";

/**Retrieves all Inkscape layers defined in given SVG.

Note: I couldn't figure out how to do this with inkscape CLI
*/
QStringList getAllSVGLayers(const QString& svgPath);

void showOnlySVGLayers(const QString& svgPath, const QStringList& layers, QIODevice* targetSvg);

void exportSVG(const QString& svgPath, const QStringList& layers, const QString& targetPngPath);

class ComponentBase
{
public:
    virtual ElementTree::Element* saveToElement() = 0;
    virtual QList<Image*> buildImages() = 0;
};

class Component : public ComponentBase
{
public:
    InkscapeSVG* m_svg;
    QString m_name;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    int m_xOffset;
    int m_yOffset;
    QStringList m_layers;
    QList<inputs::Image*>* m_cachedImages;

    Component(InkscapeSVG* svg, const QString& name = "", int x = 0, int y = 0, int width = 1, int height = 1, const QString& layers = "", int xOffset = 0, int yOffset = 0);

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    QImage generateQImage();

    QList<Image*> buildImages();
};

class FrameComponent : public ComponentBase
{
public:
    InkscapeSVG* m_svg;
    QString m_name;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    int m_cornerWidth;
    int m_cornerHeight;
    QStringList m_layers;
    QStringList m_skip;
    QList<inputs::Image*>* m_cachedImages;
    QImage* m_cachedQImage;

    FrameComponent(InkscapeSVG* svg, const QString& name = "", int x = 0, int y = 0, int width = 1, int height = 1,
                   int cornerWidth = 1, int cornerHeight = 1, const QString& layers = "", const QString& skip = "");

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    QImage generateQImage(int x, int y, int width, int height);

    QList<Image*> buildImages();
};

/*!
\brief InkscapeSVG

Just one particular SVGs, support advanced features and renders everything
    using Inkscape, the output should be of higher quality than the SVGTiny renderer
    above. Requires Inkscape to be installed and the inkscape binary to be in $PATH.

    Each component is one "image" to be passed to the metaimageset compiler.

*/
class InkscapeSVG : public inputs::Input
{
public:
    QString m_path;
    QList<ComponentBase*> m_components;

    InkscapeSVG(MetaImageset* metaImageset)
        : inputs::Input(metaImageset)
    {
        m_path = "";
//        m_components = [];
    }

    void loadFromElement(ElementTree::Element *element) override;

    ElementTree::Element* saveToElement() override;

    QString getDescription() override;

    QList<inputs::Image*> buildImages();
};

} // namesapce inkscape_svg
} // namespace inputs
} // namespace metaimageset
} // namespace editors
} // namespace CEED

#endif
