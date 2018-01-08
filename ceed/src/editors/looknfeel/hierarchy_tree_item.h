/*
   created:    25th June 2014
   author:     Lukas E Meindl
*/

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

#ifndef CEED_editors_looknfeel_hierarchy_tree_item_
#define CEED_editors_looknfeel_hierarchy_tree_item_

#include "CEEDBase.h"

#include <QStandardItem>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_item {

class LookNFeelHierarchyItem : public QStandardItem
{
public:
    FalagardElement m_falagardElement;
    QString m_prefix;

    /**
    Creates a hierarchy item based on an element of the WidgetLookFeel object. The element can be
    WidgetLookFeel itself or any child node (such as StateImagery or NamedArea, etc.) that is contained
    in the WidgetLookFeel or any of its children. The children of this element will created as well. The
    resulting hierarchy will be equal, or at least very similar, to the node hierarchy seen in the XML file.
    :param falagardElement:
    :return:
    */
    LookNFeelHierarchyItem(FalagardElement falagardElement, const QString& prefix = "");

    /**
    Creates a name and tooltip for any element that can be part of a WidgetLookFeel and returns it
    :param falagardElement:
    :return: str, str
    */
    static QPair<QString, QString> getNameAndToolTip(FalagardElement falagardElement, const QString& prefix);
#if 0
    static QPair<QString, QString> getNameAndToolTip(CEGUI::WidgetLookFeel* falagardElement, const QString& prefix);
    // Elements that can be children of a WidgetLookFeel:
    static QPair<QString, QString> getNameAndToolTip(CEGUI::PropertyDefinitionBase* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::PropertyInitialiser* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::NamedArea* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::ImagerySection* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::StateImagery* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::WidgetComponent* falagardElement, const QString& prefix);
    // Elements that can be children of a ImagerySection:
    static QPair<QString, QString> getNameAndToolTip(CEGUI::ImageryComponent* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::TextComponent* falagardElement, const QString& prefix);
    static QPair<QString, QString> getNameAndToolTip(CEGUI::FrameComponent* falagardElement, const QString& prefix);
    // Elements that can be children of a StateImagery:
    static QPair<QString, QString> getNameAndToolTip(CEGUI::LayerSpecification* falagardElement, const QString& prefix);
    // Elements that can be children of a LayerSpecification:
    static QPair<QString, QString> getNameAndToolTip(CEGUI::SectionSpecification* falagardElement, const QString& prefix);
    // The ComponentArea element
    static QPair<QString, QString> getNameAndToolTip(CEGUI::ComponentArea* falagardElement, const QString& prefix);
    // The Image element
    static QPair<QString, QString> getNameAndToolTip(CEGUI::Image* falagardElement, const QString& prefix);
#endif

    /**
    Creates the children items for for the this item's WidgetLookFeel element
    :param:
    :return:
    */
    void createChildren();

    /**
    Creates an item based on the supplied object and adds it to this object. A prefix string can be added to the displayed name.
    :param falagardElement:
    :param prefix: str
    :return:
    */
    void createAndAddItem(FalagardElement falagardElement, const QString& prefix = "");

    /**
    Creates and appends an item for the Area of the NamedArea
    :return:
    */
    void createNamedAreaChildren(CEGUI::NamedArea *falagardElement);

    /**
    Creates and appends children items based on an ImagerySection.
    :return:
    */
    void createImagerySectionChildren(CEGUI::ImagerySection *falagardElement);

    /**
    Creates and appends children items based on an ImagerySection.
    :return:
    */
    void createStateImageryChildren(CEGUI::StateImagery *falagardElement);

    /**
    Creates and appends children items based on a WidgetComponent (child widget).
    :return:
    */
    void createWidgetComponentChildren(CEGUI::WidgetComponent* falagardElement);

    /**
    Creates and appends children items based on an ImageryComponent.
    :return:
    */
    void createImageryComponentChildren(CEGUI::ImageryComponent* falagardElement);

    /**
    Creates and appends children items based on a TextComponent.
    :return:
    */
    void createTextComponentChildren(CEGUI::TextComponent *falagardElement);

    /**
    Creates and appends children items based on a FrameComponent.
    :return:
    */
    void createFrameComponentChildren(CEGUI::FrameComponent *falagardElement);

    /**
    Creates and appends children items based on a LayerSpecification.
    :return:
    */
    void createLayerSpecificationChildren(CEGUI::LayerSpecification* falagardElement);
};

} // namespace hierarchy_tree_item
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
