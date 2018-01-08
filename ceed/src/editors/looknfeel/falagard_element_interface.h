/*
   created:    5th July 2014
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

#ifndef CEED_editors_looknfeel_falagard_element_interface_
#define CEED_editors_looknfeel_falagard_element_interface_

#include "CEEDBase.h"

#include "cegui/ceguitypes.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_interface {

/*!
\brief FalagardElementInterface

 Implements static functions that can be used to get and set the
    attributes of Falagard elements. Also contains a list of all attributes
    for each type of Falagard element.

*/
class FalagardElementInterface
{
public:
    /**
    Returns the CEGUI class name that is corresponding to the Falagard element's CEGUI-type
    :param falagardElement:
    :return:
    */
    static
    QString getFalagardElementTypeAsString(FalagardElement falagardElement);

    /**
    Returns a list of names of attributes for a given Falagard element. Children elements, which can only exist a maximum of one time, are also added to the list. The list
    can be used in connection with getAttributeValue and setAttributeValue.
    :param falagardElement:
    :return:
    */
    static
    QStringList getListOfAttributes(FalagardElement falagardElement);

    /**
    Returns an attribute value of of a Falagard element using the Falagard element's
    getter function as implemented in the CEGUI code. The attribute is identified
    by a string, which is used to determine the right function call.
    :param falagardElement:
    :param attributeName: str
    :return:
    */
    static
    QPair<CEED::Variant, CEED::VariantType> getAttributeValue(FalagardElement falagardElement, const QString& attributeName, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    /**
    Sets an attribute value of of a Falagard element using the Falagard element's
    getter function as implemented in the CEGUI code. The attribute is identified
    by a string, which is used to determine the right function call.
    :param falagardElement:
    :param attributeName: str
    :param attributeValue:
    :return:
    */
    static
    void setAttributeValue(FalagardElement falagardElement, const QString& attributeName, const CEED::Variant &attributeValue);

    /**
    Returns a CEGUI value and CEGUI type based on the PropertyInitialiser value
    :param propertyInitialiser:
    :return:
    */
    static
    QPair<CEED::Variant, CEED::VariantType> getPropertyInitialiserValueAsCeguiType(CEGUI::PropertyInitialiser* propertyInitialiser, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    /**
    Returns a CEGUI value and CEGUI type based on the PropertyDefinitionBase value
    :param propertyDefBase:
    :return:
    */
    static
    QPair<CEED::Variant, CEED::VariantType> getPropertyDefinitionBaseValueAsCeguiType(CEGUI::PropertyDefinitionBase *propertyDefBase);

    /**
    Converts a string based CEGUI value and CEGUI type to the native CEGUI value and CEGUI type
    :param valueAsString: str
    :param dataTypeAsString: str
    :return:
    */
    static
    QPair<CEED::Variant, CEED::VariantType> convertToCeguiValueAndCeguiType(const CEGUI::String& valueAsString, const CEGUI::String& dataTypeAsString);

    /**
    Returns a CEGUI-typed object based on a given string and Python type
    :param pythonDataType:
    :param valueAsString: str
    :return:
    */
    static
    CEED::Variant getCeguiTypeValueFromString(CEED::VariantType pythonDataType, const QString& valueAsString);

    static
    void setPropertyInitialiserValue(CEGUI::PropertyInitialiser* propertyInitialiser, const CEED::Variant &value);

    static
    void setPropertyDefinitionBaseValue(CEGUI::PropertyDefinitionBase* propertyDefBase, const Variant &value);
};


} // namespace falagard_element_interface
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
