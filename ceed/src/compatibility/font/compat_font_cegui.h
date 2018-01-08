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

#ifndef CEED_compatibility_font_cegui_
#define CEED_compatibility_font_cegui_

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/compatibility_ceguihelpers.h"
#include "compatibility/imageset/compat_imageset_cegui.h"

#include "elementtree.h"

namespace CEED {
namespace compatibility {
namespace font {
namespace cegui {

const QString CEGUIFont1 = "CEGUI Font 1";
const QString CEGUIFont2 = "CEGUI Font 2";
const QString CEGUIFont3 = "CEGUI Font 3";
const QString CEGUIFont4 = "CEGUI Font 4";

class Font2TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUIFont2;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "font" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "font")
            return false;

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)

        return ceguihelpers::checkDataVersion("Font", "", data);
    }
};

class Font3TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType()
    {
        return CEGUIFont3;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "font" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "font")
            return false;

        return ceguihelpers::checkDataVersion("Font", "3", data);
    }
};

class Font4TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUIFont4;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "font" };
    }

    bool matches(const QString& data, const QString& extension) override
    {
        if (extension != "" && extension != "font")
            return false;

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)

        return ceguihelpers::checkDataVersion("Fonts", "4", data);
    }
};

class Font2ToFont3Layer : public compatibility::Layer
{
public:
    QString getSourceType() override
    {
        return CEGUIFont2;
    }

    QString getTargetType() override
    {
        return CEGUIFont3;
    }

    void transformAttribute(ElementTree::Element* element, const QString& attribute)
    {
        QString sourceAttributeName = attribute[0].toUpper() + attribute.mid(1);
        QString targetAttributeName = sourceAttributeName[0].toLower() + sourceAttributeName.mid(1);

        if (element->has(sourceAttributeName)) {
            element->set(targetAttributeName, element->get(sourceAttributeName));
            element->remove(sourceAttributeName);
        }
    }

    QString transform(const QString& data)
    {
        auto& root = *ElementTree::fromstring(data);
        root.set("version", "3");

        for (auto attr : {"name", "filename", "resourceGroup", "type", "size", "nativeHorzRes", "nativeVertRes", "autoScaled", "antiAlias", "lineScaling"}) {
            transformAttribute(&root, QString::fromLatin1(attr));
        }

        for (auto mapping : root.findall("Mapping")) {
            for (auto attr : {"codepoint", "image", "horzAdvance"})
                transformAttribute(mapping, QString::fromLatin1(attr));
        }

        return ceguihelpers::prettyPrintXMLElement(&root);
    }
};

class Font3ToFont2Layer : public compatibility::Layer
{
public:
    QString getSourceType()
    {
        return CEGUIFont3;
    }

    QString getTargetType() override
    {
        return CEGUIFont2;
    }

#if 0
    def transformAttribute(self, element, attribute):
        targetAttributeName = attribute[0].upper() + attribute[1:]
        sourceAttributeName = targetAttributeName[0].lower() + targetAttributeName[1:]

        if element.get(sourceAttributeName) is not None:
            element.set(targetAttributeName, element.get(sourceAttributeName))
            del element.attrib[sourceAttributeName]

    def transform(self, data):
        root = ElementTree.fromstring(data)
        del root.attrib["version"]

        for attr in ["name", "filename", "resourceGroup", "type", "size", "nativeHorzRes", "nativeVertRes", "autoScaled", "antiAlias", "lineScaling"]:
            self.transformAttribute(root, attr)

        if root.get("AutoScaled") is not None:
            root.set("AutoScaled", imageset_cegui_compat.CEGUI2ToCEGUI1Layer.autoScaledToBoolean(root.get("AutoScaled")))

        for mapping in root.findall("Mapping"):
            for attr in ["codepoint", "image", "horzAdvance"]:
                self.transformAttribute(mapping, attr)

        return ceguihelpers.prettyPrintXMLElement(root)
#endif
};

} // namespace cegui
} // namespace font
} // namespace compatibility
} // namespace CEED

#endif
