#include "CEEDBase.h"


CEED::FalagardElement::FalagardElement(const CEED::FalagardElement &other)
{
    *this = other;
}

CEED::FalagardElement &CEED::FalagardElement::operator=(const CEED::FalagardElement &other)
{
    m_type = other.m_type;

    switch (m_type) {
    case Type::NONE: break;
    case Type::ColourRect: colourRect = other.colourRect; break;
    case Type::ComponentArea: componentArea = other.componentArea; break;
    case Type::FrameComponent: frameComponent = other.frameComponent; break;
    case Type::Image: image = other.image; break;
    case Type::ImageryComponent: imageryComponent = other.imageryComponent; break;
    case Type::ImagerySection: imagerySection = other.imagerySection; break;
    case Type::LayerSpecification: layerSpecification = other.layerSpecification; break;
    case Type::NamedArea: namedArea = other.namedArea; break;
    case Type::PropertyDefinitionBase: propertyDefinitionBase = other.propertyDefinitionBase; break;
    case Type::PropertyInitialiser: propertyInitialiser = other.propertyInitialiser; break;
    case Type::SectionSpecification: sectionSpecification = other.sectionSpecification; break;
    case Type::StateImagery: stateImagery = other.stateImagery; break;
    case Type::TextComponent: textComponent = other.textComponent; break;
    case Type::WidgetComponent: widgetComponent = other.widgetComponent; break;
    case Type::WidgetLookFeel: widgetLookFeel = other.widgetLookFeel; break;
    default:
        Q_ASSERT(false);
        break;
    }

    return *this;
}

bool CEED::FalagardElement::operator==(const CEED::FalagardElement &other) const
{
    if (m_type != other.m_type)
        return false;

    // Is any of this valid?
    switch (m_type) {
    case Type::NONE: return true;
    case Type::ColourRect:
        return colourRect.d_top_left == other.colourRect.d_top_left &&
                colourRect.d_top_right == other.colourRect.d_top_right &&
                colourRect.d_bottom_left == other.colourRect.d_bottom_left &&
                colourRect.d_bottom_right == other.colourRect.d_bottom_right;
    case Type::ComponentArea: return componentArea == other.componentArea;
    case Type::FrameComponent: return frameComponent == other.frameComponent;
    case Type::Image: return image == other.image;
    case Type::ImageryComponent: return imageryComponent == other.imageryComponent;
    case Type::ImagerySection: return imagerySection == other.imagerySection;
    case Type::LayerSpecification: return layerSpecification == other.layerSpecification;
    case Type::NamedArea: return namedArea == other.namedArea;
    case Type::PropertyDefinitionBase: return propertyDefinitionBase == other.propertyDefinitionBase;
    case Type::PropertyInitialiser: return propertyInitialiser == other.propertyInitialiser;
    case Type::SectionSpecification: return sectionSpecification == other.sectionSpecification;
    case Type::StateImagery: return stateImagery == other.stateImagery;
    case Type::TextComponent: return textComponent == other.textComponent;
    case Type::WidgetComponent: return widgetComponent == other.widgetComponent;
    case Type::WidgetLookFeel: return widgetLookFeel == other.widgetLookFeel;
    default:
        Q_ASSERT(false);
        return false;
    }
}
