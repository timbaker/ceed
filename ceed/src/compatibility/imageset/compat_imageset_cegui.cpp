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

#include "compat_imageset_cegui.h"

#include "elementtree.h"

namespace CEED {
namespace compatibility {
namespace imageset {
namespace cegui {

QString CEGUI1ToCEGUI2Layer::transform(const QString &data)
{
    auto& root = *ElementTree::fromstring(data);
    root.set("version", "2");

    root.set("imagefile", root.get("Imagefile", ""));
    root.remove("Imagefile");

    if (root.has("ResourceGroup")) {
        root.set("resourceGroup", root.get("ResourceGroup", ""));
        root.remove("ResourceGroup");
    }

    root.set("name", root.get("Name", ""));
    root.remove("Name");

    if (root.has("NativeHorzRes")) {
        root.set("nativeHorzRes", root.get("NativeHorzRes", "640"));
        root.remove("NativeHorzRes");
    }

    if (root.has("NativeVertRes")) {
        root.set("nativeVertRes", root.get("NativeVertRes", "480"));
        root.remove("NativeVertRes");
    }

    if (root.has("AutoScaled")) {
        root.set("autoScaled", root.get("AutoScaled", "false"));
        root.remove("AutoScaled");
    }

    for (auto image_ : root.findall("Image")) {
        auto& image = *image_;
        image.set("name", image.get("Name", ""));
        image.remove("Name");

        // TODO: We only deal with basic image here
        if (image.get("type", "BasicImage") == "BasicImage") {
            if (image.has("XPos")) {
                image.set("xPos", image.get("XPos", "0"));
                image.remove("XPos");
            }

            if (image.has("YPos")) {
                image.set("yPos", image.get("YPos", "0"));
                image.remove("YPos");
            }

            if (image.has("Width")) {
                image.set("width", image.get("Width", "1"));
                image.remove("Width");
            }

            if (image.has("Height")) {
                image.set("height", image.get("Height", "1"));
                image.remove("Height");
            }

            if (image.has("XOffset")) {
                image.set("xOffset", image.get("XOffset", "0"));
                image.remove("xOffset");
            }

            if (image.has("YOffset")) {
                image.set("yOffset", image.get("YOffset", "0"));
                image.remove("yOffset");
            }
        }
    }

    return ceguihelpers::prettyPrintXMLElement(&root);
}

QString CEGUI2ToCEGUI1Layer::transform(const QString &data)
{
#if 1
    return QString(); // TODO
#else
    auto& root = *ElementTree::fromstring(data);
    root.remove("version") // imageset version 1 has no version attribute!

            root.set("Imagefile", root.get("imagefile", ""));
    del root.attrib["imagefile"];

    if (root.get("resourceGroup") is not None) {
        root.set("ResourceGroup", root.get("resourceGroup", ""));
        del root.attrib["resourceGroup"];
    }

    root.set("Name", root.get("name", ""));
    del root.attrib["name"];

    if (root.get("nativeHorzRes") is not None) {
        root.set("NativeHorzRes", root.get("nativeHorzRes", "640"));
        del root.attrib["nativeHorzRes"];
    }

    if (root.get("nativeVertRes") is not None) {
        root.set("NativeVertRes", root.get("nativeVertRes", "480"));
        del root.attrib["nativeVertRes"];
    }

    if (root.get("autoScaled") is not None) {
        root.set("AutoScaled", CEGUI2ToCEGUI1Layer.autoScaledToBoolean(root.get("autoScaled", "false")));
        del root.attrib["autoScaled"];
    }

    for (auto image : root.findall("Image")) {
        image.set("Name", image.get("name", ""));
        del image.attrib["name"];

        if (image.get("type", "BasicImage") != "BasicImage") {
            throw NotImplementedError("Can't convert non-BasicImage in imageset version 2 (CEGUI 0.8+) to older version, such stuff wasn't supported in imagesets version 1 (everything up to CEGUI 0.7)");
        }

        // TODO: We only deal with basic image here
        if (image.get("Type", "BasicImage") == "BasicImage") {
            if (image.get("xPos") is not None) {
                image.set("XPos", image.get("xPos", "0"));
                del image.attrib["xPos"];
            }

            if (image.get("yPos") is not None) {
                image.set("YPos", image.get("yPos", "0"));
                del image.attrib["yPos"];
            }

            if (image.get("width") is not None) {
                image.set("Width", image.get("width", "1"));
                del image.attrib["width"];
            }

            if (image.get("height") is not None) {
                image.set("Height", image.get("height", "1"));
                del image.attrib["height"];
            }

            if (image.get("xOffset") is not None) {
                image.set("XOffset", image.get("xOffset", "0"));
                del image.attrib["XOffset"];
            }

            if (image.get("yOffset") is not None) {
                image.set("YOffset", image.get("yOffset", "0"));
                del image.attrib["YOffset"];
            }
        }
    }

    return ceguihelpers::prettyPrintXMLElement(root);
#endif
}


} // namespace cegui
} // namespace imageset
} // namespace compatibilty
} // namespace CEED
