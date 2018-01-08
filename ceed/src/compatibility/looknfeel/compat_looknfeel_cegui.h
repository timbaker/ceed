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

#ifndef CEED_compatibility_looknfeel_cegui_
#define CEED_compatibility_looknfeel_cegui_

#include "CEEDBase.h"

#include "compatibility/compatibility_init.h"
#include "compatibility/layout/compat_layout_init.h"
#include "compatibility/compatibility_ceguihelpers.h"

namespace CEED {
namespace compatibility {
namespace looknfeel {
namespace cegui {

const QString CEGUILookNFeel1 = "CEGUI looknfeel 1";
// superseded by LookNFeel 2 in 0.5b
//CEGUILookNFeel2 = "CEGUI looknfeel 2"
const QString CEGUILookNFeel3 = "CEGUI looknfeel 3";
const QString CEGUILookNFeel4 = "CEGUI looknfeel 4";
// superseded by LookNFeel 6 in 0.7
//CEGUILookNFeel5 = "CEGUI looknfeel 5"
const QString CEGUILookNFeel6 = "CEGUI looknfeel 6";
const QString CEGUILookNFeel7 = "CEGUI looknfeel 7";

#if 0 // TODO

class LookNFeel6TypeDetector : public compatibility.TypeDetector
    def getType()
        return CEGUILookNFeel6

    def getPossibleExtensions()
        return set(["looknfeel"])

    def matches(self, data, extension):
        if extension not in ["", "looknfeel"]:
            return false

        # todo: we should be at least a bit more precise
        # (implement XSD based TypeDetector?)
        return ceguihelpers.checkDataVersion("Falagard", None, data)
#endif // TODO

class LookNFeel7TypeDetector : public compatibility::TypeDetector
{
    QString getType() override
    {
        return CEGUILookNFeel7;
    }

    QSet<QString> getPossibleExtensions() override
    {
        return { "looknfeel" };
    }

    bool matches(const QString &data, const QString &extension) override
    {
        if (extension != "" && extension != "looknfeel")
            return false;

        return ceguihelpers::checkDataVersion("Falagard", "7", data);
    }
};

#if 0 // TODO
class LookNFeel6To7Layer : public compatibility.Layer
    def getSourceType()
        return CEGUILookNFeel6

    def getTargetType()
        return CEGUILookNFeel7

    @classmethod
    def convertToOperatorDim(cls, dimOpDim):
        # DimOperator is a tail-tree, the left node is always a leaf
        # the right node can be a leaf or it's another DimOperator.

        # We will do the most straightforward strategy there is:
        # Dim(x, DimOp("+", Dim(y))) means "x + y"
        # --->
        # OpDim(x, "+", y)

        # So we basically move the operator "outside" and put it as the new
        # local root node.

        # We may get very unweighted binary evaluation trees (overweight
        # on the right) but we don't care about that at this point.

        dimOp = None

        for op in dimOpDim:
            if op.tag != "DimOperator":
                continue

            assert(dimOp is None)
            dimOp = op

        # bottom of the tail recursion, no operator inside
        if dimOp is None:
            return dimOpDim

        tailDim = None
        for tail in dimOp:
            tailDim = tail

        dimOpDim.remove(dimOp)
        if tailDim is not None:
            dimOp.remove(tailDim)

        dimOp.tag = "OperatorDim"
        dimOp.append(dimOpDim)
        dimOp.append(LookNFeel6To7Layer.convertToOperatorDim(tailDim))
        dimOp.text = None
        dimOpDim.text = None

        return dimOp

    def transform(self, data):
        root = ElementTree.fromstring(data)
        # Fix for Python < 2.7.
        if not hasattr(root, "iter"):
            root.iter = root.getiterator

        # version 7 has a version attribute
        root.set("version", "7")

        # New image addressing: Imageset/Image
        def convertImageElementToName(element):
            imageset = element.get("imageset")
            image = element.get("image")

            assert(imageset is not None and image is not None)

            del element.attrib["imageset"]
            del element.attrib["image"]

            element.set("name", "%s/%s" % (imageset, image))

        for element in root.iter("Image"):
            convertImageElementToName(element)

            typeAttr = element.get("type")
            if typeAttr is not None:
                element.set("component", typeAttr)
                del element.attrib["type"]

        for element in root.iter("ImageDim"):
            convertImageElementToName(element)

        parentMap = dict((c, p) for p in root.iter() for c in p)

        # Lets convert DimOperators to OperatorDims
        dimopIter = root.iter("DimOperator")
        while true:
            try:
                dimop = dimopIter.next()

            except StopIteration:
                break

            try:
                tailTree = parentMap[dimop]

            except KeyError:
                raise RuntimeError("Failed to find parent of '%s'. This is most "
                                   "likely caused by invalid data, please validate "
                                   "them using XSD to check.\n"
                                   "In case they are valid, this is a bug in the "
                                   "compatibility layer, please report it!", dimop)

            tailTreeCopy = copy.deepcopy(tailTree)

            newTree = LookNFeel6To7Layer.convertToOperatorDim(tailTreeCopy)

            # a trick that replaces tailTree with newTree "in place"
            tailTree.clear()
            tailTree.text = None #newTree.text
            tailTree.tail = newTree.tail
            tailTree.tag = newTree.tag
            tailTree.attrib = newTree.attrib
            tailTree[:] = newTree[:]
            # end of trick

            dimopIter = root.iter("DimOperator")

        # Carat was rightfully renamed to Caret
        for element in root.iter("ImagerySection"):
            if element.get("name") == "Carat":
                # TODO: We should check that parent is Editbox or MultilineEditbox, however that would be very complicated with all the mappings
                element.set("name", "Caret")

        # transform properties
        for element in root.iter("WidgetLook"):
            compatibility_layout.cegui.Layout3To4Layer.transformPropertiesOf(element, nameAttribute = "name", valueAttribute = "value", windowType = element.get("name"))

            for childElement in element.iter("Child"):
                compatibility_layout.cegui.Layout3To4Layer.transformPropertiesOf(childElement, nameAttribute = "name", valueAttribute = "value", windowType = childElement.get("type"))

        return ceguihelpers.prettyPrintXMLElement(root)


class LookNFeel7To6Layer : public compatibility.Layer
    def getSourceType()
        return CEGUILookNFeel7

    def getTargetType()
        return CEGUILookNFeel6

    def transform(self, data):
        root = ElementTree.fromstring(data)

        # version 6 must not have a version attribute
        del root.attrib["version"]

        # New image addressing: Imageset/Image
        def convertImageElementToImagesetImage(element):
            name = element.get("name")
            split = name.split("/", 1)
            assert(len(split) == 2)

            del element.attrib["name"]
            element.set("imageset", split[0])
            element.set("image", split[1])

        for element in root.iter("Image"):
            convertImageElementToImagesetImage(element)

            componentAttr = element.get("component")
            if componentAttr is not None:
                element.set("type", componentAttr)
                del element.attrib["component"]

        # TODO: Convert OperatorDims to DimOperators

        for element in root.iter("ImageDim"):
            convertImageElementToImagesetImage(element)

        # Carat was rightfully renamed to Caret
        for element in root.iter("ImagerySection"):
            if element.get("name") == "Caret":
                # TODO: We should check that parent is Editbox or MultilineEditbox, however that would be very complicated with all the mappings
                element.set("name", "Carat")

        # transform properties
        for element in root.iter("WidgetLook"):
            compatibility_layout.cegui.Layout4To3Layer.transformPropertiesOf(element, nameAttribute = "name", valueAttribute = "value", windowType = element.get("name"))

            for childElement in element.iter("Child"):
                compatibility_layout.cegui.Layout4To3Layer.transformPropertiesOf(childElement, nameAttribute = "name", valueAttribute = "value", windowType = childElement.get("type"))

        return ceguihelpers.prettyPrintXMLElement(root)

#endif // TODO

} // namespace cegui
} // namespace looknfeel
} // namespace compatibility
} // namespace CEED

#endif
