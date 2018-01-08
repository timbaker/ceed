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

#ifndef CEED_compatibility_scheme_cegui_
#define CEED_compatibility_scheme_cegui_

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"

#include "compatibility/compatibility_ceguihelpers.h"

#include "elementtree.h"

namespace CEED {
namespace compatibility {
namespace scheme {
namespace cegui {

const QString CEGUIScheme1 = "CEGUI scheme 1";
const QString CEGUIScheme2 = "CEGUI scheme 2";
const QString CEGUIScheme3 = "CEGUI scheme 3";
const QString CEGUIScheme4 = "CEGUI scheme 4";
const QString CEGUIScheme5 = "CEGUI scheme 5";

#if 0 // TODO
class Scheme4TypeDetector : public compatibility::TypeDetector
{
public:
    def getType()
        return CEGUIScheme4

    def getPossibleExtensions()
        return set(["scheme"])

    def matches(self, data, extension):
        if extension not in ["", "scheme"]:
            return false

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)
        return ceguihelpers.checkDataVersion("GUIScheme", None, data)
};
#endif // TODO

class Scheme5TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUIScheme5;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "scheme" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "scheme")
            return false;

        return ceguihelpers::checkDataVersion("GUIScheme", "5", data);
    }
};

#if 0 // TODO
class CEGUI4ToCEGUI5Layer : public compatibility::Layer
    def getSourceType()
        return CEGUIScheme4

    def getTargetType()
        return CEGUIScheme5

    def transformAttribute(self, element, attribute):
        sourceAttributeName = attribute[0].upper() + attribute[1:]
        targetAttributeName = sourceAttributeName[0].lower() + sourceAttributeName[1:]

        if element.get(sourceAttributeName) is not None:
            element.set(targetAttributeName, element.get(sourceAttributeName))
            del element.attrib[sourceAttributeName]

    def transformNamedType(self, element):
        self.transformAttribute(element, "name")
        self.transformAttribute(element, "filename")
        self.transformAttribute(element, "resourceGroup")

    def transform(self, data):
        root = ElementTree.fromstring(data)
        root.set("version", "5")

        self.transformAttribute(root, "name")

        for imageset in root.findall("Imageset"):
            self.transformNamedType(imageset)
        for imagesetFromFile in root.findall("ImagesetFromFile"):
            self.transformNamedType(imagesetFromFile)
        for font in root.findall("Font"):
            self.transformNamedType(font)
        for looknfeel in root.findall("LookNFeel"):
            self.transformNamedType(looknfeel)

        for windowSet in root.findall("WindowSet"):
            self.transformAttribute(windowSet, "filename")

            for windowFactory in windowSet.findall("WindowFactory"):
                self.transformAttribute(windowFactory, "name")

        for windowRendererSet in root.findall("WindowRendererSet"):
            self.transformAttribute(windowRendererSet, "filename")

            # this particular window renderer set got renamed
            if windowRendererSet.get("filename", "") == "CEGUIFalagardWRBase":
                windowRendererSet.set("filename", "CEGUICoreWindowRendererSet")

            for windowRendererFactory in windowRendererSet.findall("WindowRendererFactory"):
                self.transformAttribute(windowRendererFactory, "name")

        for windowAlias in root.findall("WindowAlias"):
            self.transformAttribute(windowAlias, "alias")
            self.transformAttribute(windowAlias, "target")

            if windowAlias.get("target", "") == "CEGUI/Checkbox":
                windowAlias.set("target", "CEGUI/ToggleButton")

        for falagardMapping in root.findall("FalagardMapping"):
            for attr in ["windowType", "targetType", "renderer", "lookNFeel", "renderEffect"]:
                self.transformAttribute(falagardMapping, attr)

            if falagardMapping.get("targetType", "") == "CEGUI/Checkbox":
                falagardMapping.set("targetType", "CEGUI/ToggleButton")

            if falagardMapping.get("renderer") is not None:
                rendererValue = falagardMapping.get("renderer")
                # system button got removed in CEGUI 0.8
                # no need for reverse action in the backwards layer
                if rendererValue == "Falagard/SystemButton":
                    rendererValue = "Falagard/Button"

                if rendererValue.startswith("Falagard/"):
                    falagardMapping.set("renderer", "Core/%s" % (rendererValue[9:]))

        return ceguihelpers.prettyPrintXMLElement(root)


class CEGUI5ToCEGUI4Layer : public compatibility::Layer
    def getSourceType()
        return CEGUIScheme5

    def getTargetType()
        return CEGUIScheme4

    def transformAttribute(self, element, attribute):
        targetAttributeName = attribute[0].upper() + attribute[1:]
        sourceAttributeName = targetAttributeName[0].lower() + targetAttributeName[1:]

        if element.get(sourceAttributeName) is not None:
            element.set(targetAttributeName, element.get(sourceAttributeName))
            del element.attrib[sourceAttributeName]

    def transformNamedType(self, element):
        self.transformAttribute(element, "name")
        self.transformAttribute(element, "filename")
        self.transformAttribute(element, "resourceGroup")

    def transform(self, data):
        root = ElementTree.fromstring(data)
        del root.attrib["version"]

        self.transformAttribute(root, "name")

        for imageset in root.findall("Imageset"):
            self.transformNamedType(imageset)
        for imagesetFromFile in root.findall("ImagesetFromFile"):
            self.transformNamedType(imagesetFromFile)
        for font in root.findall("Font"):
            self.transformNamedType(font)
        for looknfeel in root.findall("LookNFeel"):
            self.transformNamedType(looknfeel)

        for windowSet in root.findall("WindowSet"):
            self.transformAttribute(windowSet, "filename")

            for windowFactory in windowSet.findall("WindowFactory"):
                self.transformAttribute(windowFactory, "name")

        for windowRendererSet in root.findall("WindowRendererSet"):
            self.transformAttribute(windowRendererSet, "filename")

            # this particular window renderer set got renamed
            if windowRendererSet.get("Filename", "") == "CEGUICoreWindowRendererSet":
                windowRendererSet.set("Filename", "CEGUIFalagardWRBase")

            for windowRendererFactory in windowRendererSet.findall("WindowRendererFactory"):
                self.transformAttribute(windowRendererFactory, "name")

        for windowAlias in root.findall("WindowAlias"):
            self.transformAttribute(windowAlias, "alias")
            self.transformAttribute(windowAlias, "target")

            if windowAlias.get("Target", "") == "CEGUI/ToggleButton":
                windowAlias.set("Target", "CEGUI/Checkbox")

        for falagardMapping in root.findall("FalagardMapping"):
            for attr in ["windowType", "targetType", "renderer", "lookNFeel", "renderEffect"]:
                self.transformAttribute(falagardMapping, attr)

            if falagardMapping.get("TargetType", "") == "CEGUI/ToggleButton":
                falagardMapping.set("TargetType", "CEGUI/Checkbox")

            if falagardMapping.get("Renderer") is not None:
                rendererValue = falagardMapping.get("Renderer")
                if rendererValue.startswith("Core/"):
                    falagardMapping.set("Renderer", "Falagard/%s" % (rendererValue[5:]))

        return ceguihelpers.prettyPrintXMLElement(root)
#endif // TODO

} // namespace cegui
} // namespace scheme
} // namespace compatibility
} // namespace CEED

#endif
