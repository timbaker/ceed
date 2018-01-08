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

#include "falagard_element_interface.h"

#include "editors/looknfeel/falagard_element_inspector.h"
#include "editors/looknfeel/hierarchy_tree_model.h"

#include "propertysetinspector.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_interface {

QString FalagardElementInterface::getFalagardElementTypeAsString(FalagardElement falagardElement)
{
    if (falagardElement.toPropertyDefinitionBase()) {
        return "PropertyDefinitionBase";
    } else if (falagardElement.toPropertyInitialiser()) {
        return "PropertyInitialiser";
    } else if (falagardElement.toNamedArea()) {
        return "NamedArea";
    } else if (falagardElement.toImagerySection()) {
        return "ImagerySection";
    } else if (falagardElement.toStateImagery()) {
        return "StateImagery";
    } else if (falagardElement.toWidgetComponent()) {
        return "WidgetComponent";
    } else if (falagardElement.toImageryComponent()) {
        return "ImageryComponent";
    } else if (falagardElement.toTextComponent()) {
        return "TextComponent";
    } else if (falagardElement.toFrameComponent()) {
        return "FrameComponent";
    } else if (falagardElement.toLayerSpecification()) {
        return "LayerSpecification";
    } else if (falagardElement.toSectionSpecification()) {
        return "SectionSpecification";
    } else if (falagardElement.toComponentArea()) {
        return "ComponentArea";
    } else if (falagardElement.toImage()) {
        return "Image";
    } else if (falagardElement.isType(FalagardElement::Type::ColourRect)) {
        return "ColourRect";
#if 0 // TODO
    } else if (isinstance(falagardElement, LookNFeelHierarchyTreeModel)) {
        return "";
#endif
    } else if (falagardElement.isType(FalagardElement::Type::NONE)) {
        return "";
    } else {
        throw Exception("Unknown Falagard element used in FalagardElementInterface::getFalagardElementTypeAsString");
    }
}

QStringList FalagardElementInterface::getListOfAttributes(FalagardElement falagardElement)
{
    QStringList PROPERTY_DEFINITION_BASE_ATTRIBUTES = { "name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help" };
    QStringList PROPERTY_INITIALISER_ATTRIBITES = { "name", "value" };

    QStringList NAMED_AREA_ATTRIBUTES = { "name" };
    QStringList IMAGERY_SECTION_ATTRIBUTES = { "name", "Colour", "ColourProperty" };
    QStringList STATE_IMAGERY_ATTRIBUTES = { "name", "clipped" };

    QStringList SECTION_SPECIFICATION_ATTRIBUTES = { "section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty" };
    QStringList LAYER_SPECIFICATION_ATTRIBUTES = { "priority" };

    QStringList WIDGET_COMPONENT_ATTRIBUTES = { "nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment" };

    QStringList IMAGERY_COMPONENT_ATTRIBUTES = { "Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty" };
    QStringList TEXT_COMPONENT_ATTRIBUTES = { "Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat",
                                              "HorzFormatProperty" };
    QStringList FRAME_COMPONENT_ATTRIBUTES = { "Colour", "ColourProperty", "TopLeftCorner", "TopRightCorner", "BottomLeftCorner",
                                               "BottomRightCorner", "LeftEdge", "RightEdge", "TopEdge", "BottomEdge", "Background" };

    QStringList COMPONENT_AREA_ATTRIBUTES = { "AreaProperty", "NamedAreaSource <look>", "NamedAreaSource <name>" };

    if (falagardElement.toPropertyDefinitionBase()) {
        return PROPERTY_DEFINITION_BASE_ATTRIBUTES;
    } else if (falagardElement.toPropertyInitialiser()) {
        return PROPERTY_INITIALISER_ATTRIBITES;
    } else if (falagardElement.toNamedArea()) {
        return NAMED_AREA_ATTRIBUTES;
    } else if (falagardElement.toImagerySection()) {
        return IMAGERY_SECTION_ATTRIBUTES;
    } else if (falagardElement.toStateImagery()) {
        return STATE_IMAGERY_ATTRIBUTES;
    } else if (falagardElement.toWidgetComponent()) {
        return WIDGET_COMPONENT_ATTRIBUTES;
    } else if (falagardElement.toImageryComponent()) {
        return IMAGERY_COMPONENT_ATTRIBUTES;
    } else if (falagardElement.toTextComponent()) {
        return TEXT_COMPONENT_ATTRIBUTES;
    } else if (falagardElement.toFrameComponent()) {
        return FRAME_COMPONENT_ATTRIBUTES;
    } else if (falagardElement.toLayerSpecification()) {
        return LAYER_SPECIFICATION_ATTRIBUTES;
    } else if (falagardElement.toSectionSpecification()) {
        return SECTION_SPECIFICATION_ATTRIBUTES;
    } else if (falagardElement.toComponentArea()) {
        return COMPONENT_AREA_ATTRIBUTES;
    } else {
        throw Exception("Unknown Falagard element used in FalagardElementInterface::getListOfAttributes");
    }
}

QPair<CEED::Variant, CEED::VariantType> FalagardElementInterface::getAttributeValue(FalagardElement falagardElement_, const QString &attributeName,
                                                                                    tabbed_editor::LookNFeelTabbedEditor *tabbedEditor)
{
    QStringList attributeList = FalagardElementInterface::getListOfAttributes(falagardElement_);

    CEED::Variant attributeValue;

    // Elements that can be children of a WidgetLookFeel:
    if (auto falagardElement = falagardElement_.toPropertyDefinitionBase()) {
        // "name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getPropertyName();
        } else if (attributeName == attributeList[1]) {
            if (auto property = dynamic_cast<CEGUI::Property*>(falagardElement))
                attributeValue = property->getDataType();
            else
                Q_ASSERT(false);
        } else if (attributeName == attributeList[2]) {
            return FalagardElementInterface::getPropertyDefinitionBaseValueAsCeguiType(falagardElement);
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->isLayoutOnWrite();
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->isRedrawOnWrite();
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getEventFiredOnWrite();
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getHelpString();
        }

    } else if (auto falagardElement = falagardElement_.toPropertyInitialiser()) {
        // "name", "value"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getTargetPropertyName();
        } else if (attributeName == attributeList[1]) {
            return FalagardElementInterface::getPropertyInitialiserValueAsCeguiType(falagardElement, tabbedEditor);
        }
    }

    if (auto falagardElement = falagardElement_.toNamedArea()) {
        // "name"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getName();
        }

    } else if (auto falagardElement = falagardElement_.toImagerySection()) {
        // "name", "Colour", "ColourProperty"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getName();
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getMasterColours();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getMasterColoursPropertySource();
        }

    } else if (auto falagardElement = falagardElement_.toStateImagery()) {
        // "name", "clipped"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getName();
        }
        if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->isClippedToDisplay();
        }

    } else if (auto falagardElement = falagardElement_.toWidgetComponent()) {
        // "nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getWidgetName();
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getBaseWidgetType();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getWindowRendererType();
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->getWidgetLookName();
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->isAutoWindow();
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getVerticalWidgetAlignment();
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getHorizontalWidgetAlignment();
        }

        // Elements that can be children of an ImagerySection:
    } else if (auto falagardElement = falagardElement_.toImageryComponent()) {
        // "Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getImage();
#if 0
            if (attributeValue.toCEGUI_Image() != nullptr) {
                return CEED::Variant(), CEED::VariantType::CEGUI_Image;
            }
#endif
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getImagePropertySource();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getColours();
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->getColoursPropertySource();
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->getVerticalFormattingFromComponent();
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getVerticalFormattingPropertySource();
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getHorizontalFormattingFromComponent();
        } else if (attributeName == attributeList[7]) {
            attributeValue = falagardElement->getHorizontalFormattingPropertySource();
        }

    } else if (auto falagardElement = falagardElement_.toTextComponent()) {
        // ""Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat",
        // "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getText();
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getTextPropertySource();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getFont();
#if 0 // getFont() returns a CEGUI::String, not a CEGUI::Font*
            if (attributeValue.toCEGUI_String().empty()) {
                return CEED::Variant(), CEGUI::Font;
            }
#endif
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->getFontPropertySource();
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->getColours();
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getColoursPropertySource();
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getVerticalFormattingFromComponent();
        } else if (attributeName == attributeList[7]) {
            attributeValue = falagardElement->getVerticalFormattingPropertySource();
        } else if (attributeName == attributeList[8]) {
            attributeValue = falagardElement->getHorizontalFormattingFromComponent();
        } else if (attributeName == attributeList[9]) {
            attributeValue = falagardElement->getHorizontalFormattingPropertySource();
        }

    } else if (auto falagardElement = falagardElement_.toFrameComponent()) {
        // "Colour", "ColourProperty", "TopLeftCorner", "TopRightCorner", "BottomLeftCorner",
        // "BottomRightCorner", "LeftEdge", "RightEdge", "TopEdge", "BottomEdge", "Background"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getColours();
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getColoursPropertySource();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_TOP_LEFT_CORNER);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_TOP_RIGHT_CORNER);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_BOTTOM_LEFT_CORNER);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_BOTTOM_RIGHT_CORNER);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_LEFT_EDGE);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[7]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_RIGHT_EDGE);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[8]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_TOP_EDGE);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[9]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_BOTTOM_EDGE);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        } else if (attributeName == attributeList[10]) {
            attributeValue = falagardElement->getImage(CEGUI::FrameImageComponent::FIC_BACKGROUND);
#if 0
            if (not attributeValue) {
                return None, CEGUI::Image;
            }
#endif
        }

        // Elements that can be children of a StateImagery:
    } else if (auto falagardElement = falagardElement_.toLayerSpecification()) {
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getLayerPriority();
        }

        // Elements that can be children of a LayerSpecification:
    } else if (auto falagardElement = falagardElement_.toSectionSpecification()) {
        // "section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty"
        if (attributeName == attributeList[0]) {
            attributeValue = falagardElement->getSectionName();
        } else if (attributeName == attributeList[1]) {
            attributeValue = falagardElement->getOwnerWidgetLookFeel();
        } else if (attributeName == attributeList[2]) {
            attributeValue = falagardElement->getRenderControlPropertySource();
        } else if (attributeName == attributeList[3]) {
            attributeValue = falagardElement->getRenderControlValue();
        } else if (attributeName == attributeList[4]) {
            attributeValue = falagardElement->getRenderControlWidget();
        } else if (attributeName == attributeList[5]) {
            attributeValue = falagardElement->getOverrideColours();
        } else if (attributeName == attributeList[6]) {
            attributeValue = falagardElement->getOverrideColoursPropertySource();
        }

        // A ComponentArea element
    } else if (auto falagardElement = falagardElement_.toComponentArea()) {
        if (attributeName == attributeList[0]) {
            if (falagardElement->isAreaFetchedFromProperty()) {
                attributeValue = falagardElement->getAreaPropertySource();
            } else {
                attributeValue = CEGUI::String();
            }
        }
        if (attributeName == attributeList[1]) {
            if (falagardElement->isAreaFetchedFromNamedArea()) {
                attributeValue = falagardElement->getAreaPropertySource();
            } else {
                attributeValue = CEGUI::String();
            }
        }
        if (attributeName == attributeList[2]) {
            if (falagardElement->isAreaFetchedFromNamedArea()) {
                attributeValue = falagardElement->getNamedAreaSourceLook();
            } else {
                attributeValue = CEGUI::String();
            }
        }
    }

    // Please do not falsely "simplify" this line. The attributeValue may be false or "" but not None
    if (!attributeValue.isValid()) {
        throw Exception("Unknown Falagard element and/or attribute used in FalagardElementInterface::getAttributeValue");
    }

    return { attributeValue, attributeValue.type() };
}

void FalagardElementInterface::setAttributeValue(FalagardElement falagardElement_, const QString &attributeName, const CEED::Variant &attributeValue)
{
    auto attributeList = FalagardElementInterface::getListOfAttributes(falagardElement_);

    // Elements that can be children of a WidgetLookFeel:

    if (auto falagardElement = falagardElement_.toPropertyDefinitionBase()) {
        // "name", "type", "initialValue", "layoutOnWrite", "redrawOnWrite", "fireEvent", "help"
        if (attributeName == attributeList[0]) {
            //TODO Ident:
            throw Exception("TODO RENAME");
        } else if (attributeName == attributeList[1]) {
            //TODO Ident
            throw Exception("TODO TYPECHANGE");
        } else if (attributeName == attributeList[2]) {
            FalagardElementInterface::setPropertyDefinitionBaseValue(falagardElement, attributeValue);
        } else if (attributeName == attributeList[3]) {
            falagardElement->setLayoutOnWrite(attributeValue.toBool());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setRedrawOnWrite(attributeValue.toBool());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setEventFiredOnWrite(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setEventFiredOnWrite(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[7]) {
            falagardElement->setHelpString(attributeValue.toCEGUI_String());
        }

    } else if (auto falagardElement = falagardElement_.toPropertyInitialiser()) {
        // "name", "value"
        if (attributeName == attributeList[0]) {
            //TODO: What to do with the initialiser value once the type changes?
            //falagardElement->setTargetPropertyName(attributeValue)
            throw Exception("TODO TYPECHANGE");
        } else if (attributeName == attributeList[1]) {
            FalagardElementInterface::setPropertyInitialiserValue(falagardElement, attributeValue);
        }

    } else if (auto falagardElement = falagardElement_.toNamedArea()) {
        // "name"
        if (attributeName == attributeList[0]) {
            falagardElement->setName(attributeValue.toCEGUI_String());
        }

    } else if (auto falagardElement = falagardElement_.toImagerySection()) {
        // "name", "Colour", "ColourProperty"
        if (attributeName == attributeList[0]) {
            falagardElement->setName(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setMasterColours(attributeValue.toCEGUI_ColourRect());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setMasterColoursPropertySource(attributeValue.toCEGUI_String());
        }

    } else if (auto falagardElement = falagardElement_.toStateImagery()) {
        // "name", "clipped"
        if (attributeName == attributeList[0]) {
            falagardElement->setName(attributeValue.toCEGUI_String());
        }
        if (attributeName == attributeList[1]) {
            falagardElement->setClippedToDisplay(attributeValue.toBool());
        }

    } else if (auto falagardElement = falagardElement_.toWidgetComponent()) {
        // "nameSuffix", "type", "renderer", "look", "autoWindow", "VertAlignment", "HorzAlignment"
        if (attributeName == attributeList[0]) {
            falagardElement->setWidgetName(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setBaseWidgetType(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setWindowRendererType(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[3]) {
            falagardElement->setWidgetLookName(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setAutoWindow(attributeValue.toBool());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setVerticalWidgetAlignment(attributeValue.toCEGUI_VerticalAlignment());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setHorizontalWidgetAlignment(attributeValue.toCEGUI_HorizontalAlignment());
        }

    // Elements that can be children of an ImagerySection:

    } else if (auto falagardElement = falagardElement_.toImageryComponent()) {
        // "Image", "ImageProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
        if (attributeName == attributeList[0]) {
            falagardElement->setImage(attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setImagePropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setColours(attributeValue.toCEGUI_ColourRect());
        } else if (attributeName == attributeList[3]) {
            falagardElement->setColoursPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setVerticalFormatting(attributeValue.toCEGUI_VerticalFormatting());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setVerticalFormattingPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setHorizontalFormatting(attributeValue.toCEGUI_HorizontalFormatting());
        } else if (attributeName == attributeList[7]) {
            falagardElement->setHorizontalFormattingPropertySource(attributeValue.toCEGUI_String());
        }

    } else if (auto falagardElement = falagardElement_.toTextComponent()) {
        // "Text", "TextProperty", "Font", "FontProperty", "Colour", "ColourProperty", "VertFormat", "VertFormatProperty", "HorzFormat", "HorzFormatProperty"
        if (attributeName == attributeList[0]) {
            falagardElement->setText(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setTextPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setFont(attributeValue.toCEGUI_String()/*toCEGUI_Font()->getName()*/);
        } else if (attributeName == attributeList[3]) {
            falagardElement->setFontPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setColours(attributeValue.toCEGUI_ColourRect());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setColoursPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setVerticalFormatting(attributeValue.toCEGUI_VerticalTextFormatting());
        } else if (attributeName == attributeList[7]) {
            falagardElement->setVerticalFormattingPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[8]) {
            falagardElement->setHorizontalFormatting(attributeValue.toCEGUI_HorizontalTextFormatting());
        } else if (attributeName == attributeList[9]) {
            falagardElement->setHorizontalFormattingPropertySource(attributeValue.toCEGUI_String());
        }

    } else if (auto falagardElement = falagardElement_.toFrameComponent()) {
        // "Colour", "ColourProperty",
        if (attributeName == attributeList[0]) {
            falagardElement->setColours(attributeValue.toCEGUI_ColourRect());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setColoursPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_TOP_LEFT_CORNER, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[3]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_TOP_RIGHT_CORNER, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_BOTTOM_LEFT_CORNER, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_BOTTOM_RIGHT_CORNER, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_LEFT_EDGE, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[7]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_RIGHT_EDGE, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[8]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_TOP_EDGE, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[9]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_BOTTOM_EDGE, attributeValue.toCEGUI_Image());
        } else if (attributeName == attributeList[10]) {
            falagardElement->setImage(CEGUI::FrameImageComponent::FIC_BACKGROUND, attributeValue.toCEGUI_Image());
        }

        // Elements that can be children of a StateImagery:

    } else if (auto falagardElement = falagardElement_.toLayerSpecification()) {
        if (attributeName == attributeList[0]) {
            falagardElement->setLayerPriority(attributeValue.toUInt());
        }

    // General elements:

    // A SectionSpecification element
    } else if (auto falagardElement = falagardElement_.toSectionSpecification()) {
        // "section", "look", "controlProperty", "controlValue", "controlWidget", "Colour", "ColourProperty"
        if (attributeName == attributeList[0]) {
            falagardElement->setSectionName(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[1]) {
            falagardElement->setOwnerWidgetLookFeel(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[2]) {
            falagardElement->setRenderControlPropertySource(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[3]) {
            falagardElement->setRenderControlValue(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[4]) {
            falagardElement->setRenderControlWidget(attributeValue.toCEGUI_String());
        } else if (attributeName == attributeList[5]) {
            falagardElement->setOverrideColours(attributeValue.toCEGUI_ColourRect());
        } else if (attributeName == attributeList[6]) {
            falagardElement->setOverrideColoursPropertySource(attributeValue.toCEGUI_String());
        }

        // A ComponentArea element
    } else if (auto falagardElement = falagardElement_.toComponentArea()) {
        // FIXME? ComponentBase::getComponentArea() returns a const reference
        if (attributeName == attributeList[0]) {
            const_cast<CEGUI::ComponentArea*>(falagardElement)->setAreaPropertySource(attributeValue.toCEGUI_String());
        }
        if (attributeName == attributeList[1]) {
            const_cast<CEGUI::ComponentArea*>(falagardElement)->setAreaPropertySource(attributeValue.toCEGUI_String());
        }
        if (attributeName == attributeList[2]) {
            const_cast<CEGUI::ComponentArea*>(falagardElement)->setNamedAreaSouce(attributeValue.toCEGUI_String(), falagardElement->getAreaPropertySource());
        }

    } else {
        throw Exception("Unknown Falagard element and/or attribute used in FalagardElementInterface::setAttributeValue");
    }
}

QPair<CEED::Variant, CEED::VariantType> FalagardElementInterface::getPropertyInitialiserValueAsCeguiType(
        CEGUI::PropertyInitialiser *propertyInitialiser, tabbed_editor::LookNFeelTabbedEditor *tabbedEditor)
{
    CEGUI::String propertyName = propertyInitialiser->getTargetPropertyName();
    CEGUI::String initialValue = propertyInitialiser->getInitialiserValue();

    // We create a dummy window to be able to retrieve the correct dataType
    CEGUI::Window* dummyWindow = CEGUI::WindowManager::getSingleton().createWindow(FROM_QSTR(tabbedEditor->m_targetWidgetLook));
    CEGUI::Property* propertyInstance = dummyWindow->getPropertyInstance(propertyName);
    CEGUI::String dataType = propertyInstance->getDataType();
    CEGUI::WindowManager::getSingleton().destroyWindow(dummyWindow);

    return FalagardElementInterface::convertToCeguiValueAndCeguiType(initialValue, dataType);
}

QPair<CEED::Variant, CEED::VariantType> FalagardElementInterface::getPropertyDefinitionBaseValueAsCeguiType(CEGUI::PropertyDefinitionBase *propertyDefBase)
{
    CEGUI::String dataType; // = propertyDefBase->getDataType();
    if (auto property = dynamic_cast<CEGUI::Property*>(propertyDefBase))
        dataType = property->getDataType();
    else
        Q_ASSERT(false);

    CEGUI::String initialValue = propertyDefBase->getInitialValue();

    return FalagardElementInterface::convertToCeguiValueAndCeguiType(initialValue, dataType);
}

QPair<CEED::Variant, VariantType> FalagardElementInterface::convertToCeguiValueAndCeguiType(const CEGUI::String &valueAsString, const CEGUI::String &dataTypeAsString)
{
    CEED::VariantType pythonDataType = propertysetinspector::CEGUIPropertyManager::getPythonTypeFromStringifiedCeguiType(TO_QSTR(dataTypeAsString));

    CEED::Variant value = FalagardElementInterface::getCeguiTypeValueFromString(pythonDataType, TO_QSTR(valueAsString));
    CEED::VariantType valueType = falagard_element_inspector::FalagardElementAttributesManager::getCeguiTypeTypeFromPythonType(pythonDataType);

    return { value, valueType };
}

CEED::Variant FalagardElementInterface::getCeguiTypeValueFromString(CEED::VariantType pythonDataType, const QString &valueAsString)
{
    CEED::Variant value;

#if 1
    value = propertysetinspector::getCeguiValueFromString(pythonDataType, valueAsString);
#elif 0
    switch (pythonDataType) {
    case VariantType::Bool : value = propertytree::utility::boolFromString(valueAsString); break;
    case VariantType::Float : value = propertytree::utility::floatFromString(valueAsString); break;
    case VariantType::Int : value = propertytree::utility::intFromString(valueAsString); break;
    case VariantType::UInt : value = propertytree::utility::uintFromString(valueAsString); break;
    case VariantType::QString : value = CEGUI::String(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_UDim: value = cegui::ceguitypes::UDim::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_USize: value = cegui::ceguitypes::USize::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_UVector2: value = cegui::ceguitypes::UVector2::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_URect: value = cegui::ceguitypes::URect::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_Colour: value = cegui::ceguitypes::Colour::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_ColourRect: value = cegui::ceguitypes::ColourRect::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_FontRef: value = cegui::ceguitypes::FontRef::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    case VariantType::CEED_ImageRef: value = cegui::ceguitypes::ImageRef::tryToCeguiType(FROM_QSTR(valueAsString)); break;
    default:
        Q_ASSERT(false);
        break;
    }
#else
    if (issubclass(pythonDataType, ceguiTypes::Base)) {
        // if the type is a subtype of the python cegui type, then use the conversion function
        value = pythonDataType.tryToCeguiType(valueAsString);
    } else {
        if (valueAsString == nullptr) {
            value = None;
        } else if (pythonDataType == CEED::VariantType::Bool) {
            // The built-in bool parses "false" as true
            // so we replace the default value creator.
            value = propertytree::utility::boolFromString(valueAsString);
        } else if (pythonDataType == CEED::VariantType::QString) {
            value = valueAsString;
        }
    }
#endif

    return value;
}

void FalagardElementInterface::setPropertyInitialiserValue(CEGUI::PropertyInitialiser *propertyInitialiser, const CEED::Variant &value)
{
    CEED::VariantType valueType = value.type();

    CEED::VariantType pythonType = falagard_element_inspector::FalagardElementAttributesManager::getPythonTypeFromCeguiType(valueType);

#if 1
    CEED::Variant converted = value;
    converted.toType(pythonType);
    QString valueAsString = converted.toString();
#else
    //        from ceed.cegui import ceguitypes
    if (issubclass(pythonType, ceguitypes.Base)) {
        // if the value's type is a subclass of a python ceguitype, convert the value to a string
        valueAsString = pythonType.toString(value);
    } else {
        valueAsString = unicode(value);
    }
#endif

    propertyInitialiser->setInitialiserValue(FROM_QSTR(valueAsString));
}

void FalagardElementInterface::setPropertyDefinitionBaseValue(CEGUI::PropertyDefinitionBase *propertyDefBase, const CEED::Variant &value)
{
    CEED::VariantType valueType = value.type();

    CEED::VariantType pythonType = falagard_element_inspector::FalagardElementAttributesManager::getPythonTypeFromCeguiType(valueType);

#if 1
    CEED::Variant converted = value;
    converted.toType(pythonType);
    QString valueAsString = converted.toString();
#else
    if (issubclass(pythonType, ceguitypes.Base)) {
        // if the value's type is a subclass of a python ceguitype, convert the value to a string
        valueAsString = pythonType.toString(value);
    } else if (!value.isValid()) {
        valueAsString = "";
    } else {
        valueAsString = unicode(value);
    }
#endif

    propertyDefBase->setInitialValue(FROM_QSTR(valueAsString));
}


} // namespace falagard_element_interface
} // namespace looknfeel
} // namespace editors
} // namespace CEED
