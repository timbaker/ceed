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

#ifndef CEED_compatibility_layout_cegui_
#define CEED_compatibility_layout_cegui_

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/compatibility_ceguihelpers.h"

#include "elementtree.h"

namespace CEED {
namespace compatibility {
namespace cegui {

const QString CEGUILayout2 = "CEGUI layout 2";
const QString CEGUILayout3 = "CEGUI layout 3";
const QString CEGUILayout4 = "CEGUI layout 4";

class Layout2TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUILayout2;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "layout" };
    }

    bool matches(const QString &data, const QString &extension) override
    {
        if (extension != "" && extension != "layout") {
            return false;
        }

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)
        return ceguihelpers::checkDataVersion("GUILayout", "", data);
    }
};

class Layout3TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUILayout3;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "layout" };
    }

    bool matches(const QString &data, const QString &extension) override
    {
        if (extension != "" && extension != "layout") {
            return false;
        }

        // todo: we should be at least a bit more precise
        // (implement XSD based TypeDetector?)
        return ceguihelpers::checkDataVersion("GUILayout", "", data);
    }
};

class Layout4TypeDetector : public compatibility::TypeDetector
{
public:
    QString getType() override
    {
        return CEGUILayout4;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "layout" };
    }

    bool matches(const QString &data, const QString &extension) override
    {
        if (extension != "" && extension != "layout") {
            return false;
        }

        return ceguihelpers::checkDataVersion("GUILayout", "4", data);
    }
};

class Layout3To4Layer : public compatibility::Layer
{

public:
    QString getSourceType() override
    {
        return CEGUILayout3;
    }

    QString getTargetType() override
    {
        return CEGUILayout4;
    }

#if 0 // TODO
    QString transformAttribute(ElementTree::Element* element, const QString& attribute)
    {
        QString sourceAttributeName = attribute[0].toUpper() + attribute.mid(1);
        QString targetAttributeName = sourceAttributeName[0].toLower() + sourceAttributeName.mid(1);

        if (element->get(sourceAttributeName) != nullptr) {
            element->set(targetAttributeName, element.get(sourceAttributeName));
            delete element->attrib[sourceAttributeName];
        }
    }

    QString convertToRelativeNames(ElementTree::Element* window, const QString& leadingName = "")
    {
        QString ret = "";

        QString name = window->get("Name", "");

        if (name.startsWith(leadingName + "/")) {
            name = name.mid((leadingName + "/").length());

            for (auto childWindow : window->findall("Window"))
                ret += convertToRelativeNames(childWindow, leadingName + "/" + name);
        } else {

            for (auto childWindow : window->findall("Window"))
                ret += convertToRelativeNames(childWindow, name);
        }

        int delimiterPosition = name.rfind("/");
        if (delimiterPosition != -1) {
            oldName = name;
            name = name.mid(delimiterPosition + 1);

            ret += "Warning: Renaming '%s' to '%s' because it contains '/' even after prefix stripping (and '/' is a disallowed character!)\n" % (oldName, name);
        }

        window->set("Name", name);

        return ret;
    }

    QString convertAutoWindowSuffix(ElementTree::Element* autoWindow)
    {
        if (autoWindow->has("NameSuffix")) {
            autoWindow->set("namePath", autoWindow->get("NameSuffix"));
            autoWindow->remove("NameSuffix");

        for (auto childAutoWindow : autoWindow->findall("AutoWindow"))
            convertAutoWindowSuffix(childAutoWindow);
    }

    @classmethod
    def transformPropertiesOf(cls, element, tag = "Property", nameAttribute = "Name", valueAttribute = "Value", windowType = None)
    {
        if (windowType == None) {
            windowType = element.get("Type");
            if (windowType == None)
                windowType = "";
                //raise RuntimeError("Can't figure out windowType when transforming properties, tried attribute 'Type'")
        }

        // convert the properties that had 'Unified' prefix
        for (property_ : element.findall(tag)) {
            name = property_.get(nameAttribute, "");

            if (name == "ZOrderChangeEnabled")
                name = "ZOrderingEnabled";

            else if (name == "MouseButtonDownAutoRepeat")
                name = "MouseAutoRepeatEnabled";

            else if (name == "CustomTooltipType")
                name = "TooltipType";
            else if (name == "Tooltip")
                name = "TooltipText";

            else if (name == "RiseOnClick")
                name = "RiseOnClickEnabled";

            else if (name == "UnifiedAreaRect")
                name = "Area";
            else if (name == "UnifiedPosition")
                name = "Position";
            else if (name == "UnifiedXPosition")
                name = "XPosition";
            else if (name == "UnifiedYPosition")
                name = "YPosition";
            else if (name == "UnifiedSize")
                name = "Size";
            else if (name == "UnifiedWidth")
                name = "Width";
            else if (name == "UnifiedHeight")
                name = "Height";
            else if (name == "UnifiedMinSize")
                name = "MinSize";
            else if (name == "UnifiedMaxSize")
                name = "MaxSize";

            if (name != "")
                property_.set(nameAttribute, name);

            def convertImagePropertyToName(property_)
            {
                value = property_.get(valueAttribute);
                if (value is None)
                    value = property_.text;

                split = value.split("image:", 1);
                if (split.length() != 2)
                    throw RuntimeError("Failed parsing value '%s' as 0.7 image reference" % (value));

                split[0] = split[0].mid(4); // get rid of "set:"

                // strip both of whitespaces left and right
                split[0] = split[0].strip();
                split[1] = split[1].strip();

                property_.set(valueAttribute, "%s/%s" % (split[0], split[1]));
            }

            if (windowType.endswith("StaticImage")) {
                if (name == "Image")
                    convertImagePropertyToName(property_);

            } else if (windowType.endswith("ImageButton")) {
                if (name in ["NormalImage", "HoverImage", "PushedImage"])
                    convertImagePropertyToName(property_);

            } else if (windowType.endswith("FrameWindow")) {
                if (name.endswith("SizingCursorImage"))
                    convertImagePropertyToName(property_);

            } else if (windowType.endswith("ListHeaderSegment")) {
                if (name in ["MovingCursorImage", "SizingCursorImage"])
                    convertImagePropertyToName(property_);

            } else {
                // we have done all explicit migrations, at this point the best we can do is guess
                // if a property name ends with Image, it is most likely an image
                if (name.endswith("Image")) {
                    try {
                        convertImagePropertyToName(property_);
                    } catch {
                        // best effort only, we don't have enough info
                    }
                }
            }
        }
    }

    QString applyChangesRecursively(window)
    {
        QString ret = "";

        for (layoutImport in window.findall("LayoutImport")) {
            transformAttribute(layoutImport, "filename");
            transformAttribute(layoutImport, "resourceGroup");
            if (layoutImport.get("Prefix") != nullptr) {
                // no such thing is available in layout version 4
                del layoutImport.attrib["Prefix"];
            }
        }

        Layout3To4Layer.transformPropertiesOf(window)
        for (property_ : window.findall("Property")) {
            property_.set("name", property_.get("Name"));
            del property_.attrib["Name"];

            if (property_.get("Value") is not None) {
                property_.set("value", property_.get("Value"));
                del property_.attrib["Value"];
            }
        }

        for (event : window.findall("Event")) {
            self.transformAttribute(event, "name");
            self.transformAttribute(event, "function");
        }

        for (childWindow : window.findall("Window")) {
            self.applyChangesRecursively(childWindow);
        }

        for (autoWindow : window.findall("AutoWindow")) {
            self.convertAutoWindowSuffix(autoWindow);
            self.applyChangesRecursively(autoWindow);
        }

        if (window.get("Name") != nullptr) {
            window.set("name", window.get("Name"));
            del window.attrib["Name"];
        }

        if (window.get("Type") != nullptr) {
            window.set("type", window.get("Type"));
            del window.attrib["Type"];
        }

        return ret;
    }

    QString transform(QByteArray& data) override
    {
        QString log = "";

        root = ElementTree.fromstring(data.encode("utf-8"));

        // version 4 has a version attribute
        root.set("version", "4");

        // no parent attribute in layout version 4 (CEGUI 0.8+)
        if (root.get("Parent") != nullptr)
            del root.attrib["Parent"];

        for (window : root.findall("Window")) {
            // should be only one window

            // first make the names relative to parent
            log += self.convertToRelativeNames(window);

            // apply other changes
            log += self.applyChangesRecursively(window);
        }

        return unicode(ceguihelpers.prettyPrintXMLElement(root), encoding = "utf-8");
    }
#endif // TODO
};

class Layout4To3Layer : public compatibility::Layer
{
public:
    QString getSourceType() override
    {
        return CEGUILayout4;
    }

    QString getTargetType() override
    {
        return CEGUILayout3;
    }

#if 0 // TODO
    QString transformAttribute(element, const QString& attribute)
    {
        QString targetAttributeName = attribute[0].toUpper() + attribute.mid(1);
        QString sourceAttributeName = targetAttributeName[0].toLower() + targetAttributeName.mid(1);

        if (element.get(sourceAttributeName) != nullptr) {
            element.set(targetAttributeName, element.get(sourceAttributeName));
            delete element.attrib[sourceAttributeName];
        }
    }

    QString convertToAbsoluteNames(window, const QString& leadingName = "")
    {
        QString ret = "";

        QString name = window.get("name", "");
        if (leadingName != "")
            name = leadingName + "/" + name;

        for (childWindow : window.findall("Window"))
            ret += convertToAbsoluteNames(childWindow, name);

        window.set("name", name);

        return ret;
    }

    QString convertAutoWindowSuffix(autoWindow)
    {
        if (autoWindow.get("namePath") != nullptr) {
            autoWindow.set("NameSuffix", autoWindow.get("namePath"));
            delete autoWindow.attrib["namePath"];
        }

        for (childAutoWindow : autoWindow.findall("AutoWindow")) {
            convertAutoWindowSuffix(childAutoWindow);
        }
    }

    @classmethod
    def transformPropertiesOf(cls, element, tag = "Property", nameAttribute = "name", valueAttribute = "value", windowType = None)
    {
        if windowType is None:
            windowType = element.get("type")
            if windowType is None:
                windowType = ""
                //raise RuntimeError("Can't figure out windowType when transforming properties, tried attribute 'Type'")

        // convert the properties that had 'Unified' prefix in 0.7
        for property_ in element.findall(tag):
            name = property_.get(nameAttribute, "")

            if name == "ZOrderingEnabled":
                name = "ZOrderChangeEnabled"

            else if (name == "MouseAutoRepeatEnabled":
                name = "MouseButtonDownAutoRepeat"

            else if (name == "TooltipType":
                name = "CustomTooltipType"
            else if (name == "TooltipText":
                name = "Tooltip"

            else if (name == "RiseOnClickEnabled":
                name = "RiseOnClick"

            else if (name == "Area":
                name = "UnifiedAreaRect"
            else if (name == "Position":
                name = "UnifiedPosition"
            else if (name == "XPosition":
                name = "UnifiedXPosition"
            else if (name == "YPosition":
                name = "UnifiedYPosition"
            else if (name == "Size":
                name = "UnifiedSize"
            else if (name == "Width":
                name = "UnifiedWidth"
            else if (name == "Height":
                name = "UnifiedHeight"
            else if (name == "MinSize":
                name = "UnifiedMinSize"
            else if (name == "MaxSize":
                name = "UnifiedMaxSize"

            if name != "":
                property_.set(nameAttribute, name)

            def convertImagePropertyToImagesetImage(property_):
                value = property_.get(valueAttribute)
                if value is None:
                    value = property_.text

                split = value.split("/", 1)
                if len(split) != 2:
                    raise RuntimeError("Failed parsing value '%s' as CEGUI 0.8+ image reference" % (value))
                property_.set(valueAttribute, "set:%s image:%s" % (split[0], split[1]))

            if windowType.endswith("StaticImage"):
                if name == "Image":
                    convertImagePropertyToImagesetImage(property_)

            else if (windowType.endswith("ImageButton"):
                if name in ["NormalImage", "HoverImage", "PushedImage"]:
                    convertImagePropertyToImagesetImage(property_)
                    return

            else if (windowType.endswith("FrameWindow"):
                if name.endswith("SizingCursorImage"):
                    convertImagePropertyToImagesetImage(property_)

            else if (windowType.endswith("ListHeaderSegment"):
                if name in ["MovingCursorImage", "SizingCursorImage"]:
                    convertImagePropertyToImagesetImage(property_)

            else:
                // we have done all explicit migrations, at this point the best we can do is guess
                // if a property name ends with Image, it is most likely an image
                if name.endswith("Image"):
                    try:
                        convertImagePropertyToImagesetImage(property_)

                    except:
                        // best effort only, we don't have enough info
                        pass
    }

    QString applyChangesRecursively(window)
    {
        QString ret = "";

        for (layoutImport : window.findall("LayoutImport")) {
            transformAttribute(layoutImport, "filename");
            transformAttribute(layoutImport, "resourceGroup");
        }

        transformPropertiesOf(window);
        for (property_ : window.findall("Property")) {
            property_.set("Name", property_.get("name"));
            del property_.attrib["name"];

            if (property_.get("value") != nullptr) {
                property_.set("Value", property_.get("value"));
                del property_.attrib["value"];
            }
        }

        for (event : window.findall("Event")) {
            self.transformAttribute(event, "name");
            self.transformAttribute(event, "function");
        }

        for (childWindow : window.findall("Window")) {
            self.applyChangesRecursively(childWindow);
        }

        for (autoWindow : window.findall("AutoWindow")) {
            self.convertAutoWindowSuffix(autoWindow);
            self.applyChangesRecursively(autoWindow);
        }

        for (_e : window.findall("UserString")) {
            throw NotImplementedError("Can't migrate, UserString element is not supported in layout version 3 (before CEGUI 0.7)");
        }

        if (window.get("name") != nullptr) {
            window.set("Name", window.get("name"));
            del window.attrib["name"];
        }

        if (window.get("type") != nullptr) {
            window.set("Type", window.get("type"));
            del window.attrib["type"];
        }

        return ret;
        }

    def transform(self, data):
        log = ""

        root = ElementTree.fromstring(data.encode("utf-8"))

        // version 3 must not have a version attribute
        del root.attrib["version"]

        for window in root.findall("Window"):
            // should be only one window

            // first make the names relative to parent
            log += self.convertToAbsoluteNames(window)

            // apply other changes
            log += self.applyChangesRecursively(window)

        return unicode(ceguihelpers.prettyPrintXMLElement(root), encoding = "utf-8")
  #endif // Layout4to3Layer
};

} // namespace cegui
} // namespace layout
} // namespace compatibility

#endif
