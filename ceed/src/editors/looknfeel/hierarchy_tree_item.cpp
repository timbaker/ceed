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

#include "editors/looknfeel/hierarchy_tree_item.h"

#include "editors/looknfeel/falagard_element_interface.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_item {

LookNFeelHierarchyItem::LookNFeelHierarchyItem(FalagardElement falagardElement, const QString &prefix)
    : QStandardItem()
{
    m_falagardElement = falagardElement;
    m_prefix = prefix;

    auto nameAndToolTip = getNameAndToolTip(falagardElement, m_prefix);

    setText(nameAndToolTip.first);
    setToolTip(nameAndToolTip.second);

    setFlags(Qt::ItemIsEnabled |
             Qt::ItemIsSelectable);

    setData(QVariant(), Qt::CheckStateRole);

    createChildren();
}

#if 0
QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(FalagardElement falagardElement, const QString& prefix)
{
    switch (falagardElement.type()) {
    case FalagardElement::Type::PropertyDefinitionBase:
        return getNameAndToolTip(falagardElement.toPropertyDefinitionBase(), prefix);
    case FalagardElement::Type::PropertyInitialiser:
        return getNameAndToolTip(falagardElement.toPropertyInitialiser(), prefix);
    case FalagardElement::Type::NamedArea:
        return getNameAndToolTip(falagardElement.toNamedArea(), prefix);
    case FalagardElement::Type::ImagerySection:
        return getNameAndToolTip(falagardElement.toImagerySection(), prefix);
    case FalagardElement::Type::StateImagery:
        return getNameAndToolTip(falagardElement.toStateImagery(), prefix);
    case FalagardElement::Type::WidgetComponent:
        return getNameAndToolTip(falagardElement.toWidgetComponent(), prefix);
    case FalagardElement::Type::ImageryComponent:
        return getNameAndToolTip(falagardElement.toImageryComponent(), prefix);
    case FalagardElement::Type::TextComponent:
        return getNameAndToolTip(falagardElement.toTextComponent(), prefix);
    case FalagardElement::Type::FrameComponent:
        return getNameAndToolTip(falagardElement.toFrameComponent(), prefix);
    case FalagardElement::Type::LayerSpecification:
        return getNameAndToolTip(falagardElement.toLayerSpecification(), prefix);
    case FalagardElement::Type::SectionSpecification:
        return getNameAndToolTip(falagardElement.toSectionSpecification(), prefix);
    case FalagardElement::Type::ComponentArea:
        return getNameAndToolTip(falagardElement.toComponentArea(), prefix);
    case FalagardElement::Type::Image:
        return getNameAndToolTip(falagardElement.toImage(), prefix);
    default:
        Q_ASSERT(false);
    }
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::WidgetLookFeel *falagardElement_, const QString &prefix)
{
    CEGUI::String name = prefix;
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::PropertyDefinitionBase *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getPropertyName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::PropertyInitialiser *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getTargetPropertyName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::NamedArea *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::ImagerySection *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::StateImagery *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::WidgetComponent *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "\"" + falagardElement->getWidgetName() + "\"" + " (\"" + falagardElement->getBaseWidgetType() + "\")";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::ImageryComponent *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "ImageryComponent";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::TextComponent *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "TextComponent";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::FrameComponent *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "FrameComponent";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::LayerSpecification *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "Layer";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::SectionSpecification *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "Section: \"" + falagardElement->getSectionName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::ComponentArea *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += "Area";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(CEGUI::Image *falagardElement, const QString &prefix)
{
    CEGUI::String name = prefix;
    name += " \"" + falagardElement->getName() + "\"";
    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);
    return name, toolTip;
}

#else

QPair<QString, QString> LookNFeelHierarchyItem::getNameAndToolTip(FalagardElement falagardElement_, const QString& prefix)
{
    QString name = prefix;

    QString toolTip = "type: " + falagard_element_interface::FalagardElementInterface::getFalagardElementTypeAsString(falagardElement_);

    if (auto falagardElement = falagardElement_.toPropertyDefinitionBase()) {
        name += "\"" + TO_QSTR(falagardElement->getPropertyName()) + "\"";

    } else if (auto falagardElement = falagardElement_.toPropertyInitialiser()) {
        name += "\"" + TO_QSTR(falagardElement->getTargetPropertyName()) + "\"";

    } else if (auto falagardElement = falagardElement_.toNamedArea()) {
        name += "\"" + TO_QSTR(falagardElement->getName()) + "\"";
    } else if (auto falagardElement = falagardElement_.toImagerySection()) {
        name += "\"" + TO_QSTR(falagardElement->getName()) + "\"";
    } else if (auto falagardElement = falagardElement_.toStateImagery()) {
        name += "\"" + TO_QSTR(falagardElement->getName()) + "\"";
    } else if (auto falagardElement = falagardElement_.toWidgetComponent()) {
        name += "\"" + TO_QSTR(falagardElement->getWidgetName()) + "\"" + " (\"" + TO_QSTR(falagardElement->getBaseWidgetType()) + "\")";

    // Elements that can be children of a ImagerySection:
    } else if (auto falagardElement = falagardElement_.toImageryComponent()) {
        name += "ImageryComponent";
    } else if (auto falagardElement = falagardElement_.toTextComponent()) {
        name += "TextComponent";
    } else if (auto falagardElement = falagardElement_.toFrameComponent()) {
        name += "FrameComponent";

    // Elements that can be children of a StateImagery:
    } else if (auto falagardElement = falagardElement_.toLayerSpecification()) {
        name += "Layer";

    // Elements that can be children of a LayerSpecification:
    } else if (auto falagardElement = falagardElement_.toSectionSpecification()) {
        name += "Section: \"" + TO_QSTR(falagardElement->getSectionName()) + "\"";

    // The ComponentArea element
    } else if (auto falagardElement = falagardElement_.toComponentArea()) {
        name += "Area";

    // The Image element
    } else if (auto falagardElement = falagardElement_.toImage()) {
        name += " \"" + TO_QSTR(falagardElement->getName()) + "\"";
    }

    return { name, toolTip };
}
#endif

void LookNFeelHierarchyItem::createChildren()
{
    if (auto falagardElement = m_falagardElement.toNamedArea()) {
        createNamedAreaChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toImagerySection()) {
        createImagerySectionChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toStateImagery()) {
        createStateImageryChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toWidgetComponent()) {
        createWidgetComponentChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toImageryComponent()) {
        createImageryComponentChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toTextComponent()) {
        createTextComponentChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toFrameComponent()) {
        createFrameComponentChildren(falagardElement);
    } else if (auto falagardElement = m_falagardElement.toLayerSpecification()) {
        createLayerSpecificationChildren(falagardElement);
    }
}

void LookNFeelHierarchyItem::createAndAddItem(FalagardElement falagardElement, const QString &prefix)
{
    auto newItem = new LookNFeelHierarchyItem(falagardElement, prefix);
    appendRow(newItem);
}

void LookNFeelHierarchyItem::createNamedAreaChildren(CEGUI::NamedArea* falagardElement)
{
    auto& area = falagardElement->getArea();
    createAndAddItem(&area);
}

void LookNFeelHierarchyItem::createImagerySectionChildren(CEGUI::ImagerySection* falagardElement)
{
    auto frameComponentList = falagardElement->getFrameComponentPointers();
    for (auto frameComponent : frameComponentList)
        createAndAddItem(frameComponent);

    auto textComponentList = falagardElement->getTextComponentPointers();
    for (auto textComponent : textComponentList)
        createAndAddItem(textComponent);

    auto imageryComponentList = falagardElement->getImageryComponentPointers();
    for (auto imageryComponent : imageryComponentList)
        createAndAddItem(imageryComponent);
}

void LookNFeelHierarchyItem::createStateImageryChildren(CEGUI::StateImagery* falagardElement)
{
    auto layerSpecList = falagardElement->getLayerSpecificationPointers();
    for (auto layerSpec : layerSpecList)
        createAndAddItem(layerSpec);
}

void LookNFeelHierarchyItem::createWidgetComponentChildren(CEGUI::WidgetComponent* falagardElement)
{
    auto& area = falagardElement->getComponentArea();
    createAndAddItem(&area);
}

void LookNFeelHierarchyItem::createImageryComponentChildren(CEGUI::ImageryComponent* falagardElement)
{
    auto& area = falagardElement->getComponentArea();
    createAndAddItem(&area);
}

void LookNFeelHierarchyItem::createTextComponentChildren(CEGUI::TextComponent* falagardElement)
{
    auto& area = falagardElement->getComponentArea();
    createAndAddItem(&area);
}

void LookNFeelHierarchyItem::createFrameComponentChildren(CEGUI::FrameComponent *falagardElement)
{
    auto& area = falagardElement->getComponentArea();
    createAndAddItem(&area);
}

void LookNFeelHierarchyItem::createLayerSpecificationChildren(CEGUI::LayerSpecification *falagardElement)
{
    auto sectionSpecList = falagardElement->getSectionSpecificationPointers();
    for (auto sectionSpec : sectionSpecList)
        createAndAddItem(sectionSpec);
}


} // namespace hierarchy_tree_item
} // namespace looknfeel
} // namespace editors
} // namespace CEED
