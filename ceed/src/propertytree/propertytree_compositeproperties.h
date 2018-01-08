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

#ifndef CEED_propertytree_compositeproperties_
#define CEED_propertytree_compositeproperties_

#include "CEEDBase.h"

/**The standard composite properties.

DictionaryProperty -- Generic property based on a dictionary.
*/

#include "propertytree/propertytree_properties.h"
#include "propertytree/propertytree_parsers.h"

#include "qtorderedmap.h"

//using namespace CEED::propertytree::properties;

namespace CEED {
namespace propertytree {
namespace compositeproperties {

//using AstHelper = CEED::propertytree::parsers::AstHelper;

/*!
\brief DictionaryProperty

A generic composite property based on a dict (or OrderedDict).

    The key-value pairs are used as components. A value can be a Property
    itself, allowing nested properties and the creation of multi-level,
    hierarchical properties.

    Example::
        colourProp = DictionaryProperty(
                                        name = "Colour",
                                        value = OrderedDict([
                                                             ("Red", 160),
                                                             ("Green", 255),
                                                             ("Blue", 160)
                                                             ]),
                                        editorOptions = {"instantApply":false, "numeric": {"min":0, "max":255, "step": 8}}
                                        )
        DictionaryProperty("dictionary", OrderedDict([
                                                      ("X", 0),
                                                      ("Y", 0),
                                                      ("Width", 50),
                                                      ("Height", 50),
                                                      ("Colour", colourProp)
                                                      ]),
                           readOnly=false)

*/
class DictionaryProperty : public CEED::propertytree::properties::Property
{
public:
    enum class StringRepresentationMode
    {
        ReadOnly = 0,
        EditValuesRestrictTypes = 1,
        EditValuesFreeTypes = 2,
        EditKeysAndValues = 3,
    };

    StringRepresentationMode m_strReprMode;
    QMap<QString, QVariant> m_strValueReplacements;

    DictionaryProperty(const QString& name, const CEED::Variant& value=CEED::Variant(), const QString& category=QString(), const QString& helpText=QString(),
                       bool readOnly=false, const CEED::propertytree::properties::EditorOptions& editorOptions = CEED::propertytree::properties::EditorOptions(),
                       StringRepresentationMode strReprMode = StringRepresentationMode::ReadOnly,
                       const QMap<QString, QVariant>& strValueReplacements = QMap<QString, QVariant>());

    void createComponents() override;

    bool hasDefaultValue() override;

    OrderedMap<QString, Property*> getComponents() override
    {
        return m_components;
    }

    QString valueToString() override
    {
        QStringList list;
        for (Property* prop : m_components.values()) {
            list += prop->m_name + ":" + prop->valueToString();
        }
        return "{" + list.join(", ") + "}";
    }

    bool isStringRepresentationEditable() override
    {
        return m_strReprMode != StringRepresentationMode::ReadOnly;
    }

    CEED::Variant tryParse(const QString& strValue) override;

    void componentValueChanged(CEED::propertytree::properties::Property* component, CEED::propertytree::properties::ChangeValueReason reason) override;

    void updateComponents(CEED::propertytree::properties::ChangeValueReason reason = CEED::propertytree::properties::ChangeValueReason::Unknown) override;
};

} // namespace compositeproperties
} // namesapce propertytree
} // namespace CEED

#endif
