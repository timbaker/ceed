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

#include "propertytree_compositeproperties.h"

namespace CEED {
namespace propertytree {
namespace compositeproperties {

DictionaryProperty::DictionaryProperty(const QString &name, const CEED::Variant &value, const QString &category,
                                       const QString &helpText, bool readOnly,
                                       const CEED::propertytree::properties::EditorOptions &editorOptions,
                                       StringRepresentationMode strReprMode,
                                       const QMap<QString, QVariant> &strValueReplacements)
    : Property(name,
               value,
               CEED::Variant()/*defaultValue*/,
               category,
               helpText,
               readOnly,
               editorOptions)

{
    m_strReprMode = strReprMode;
    m_strValueReplacements = strValueReplacements;
}

void DictionaryProperty::createComponents()
{
    CEED::Variant::MapType map = m_value.toOrderedMap();
    for (auto it = map.begin(); it != map.end(); it++) {
        QString name = it.key();
        CEED::Variant value = it.value();
        // if the value of the item is a Property, make sure
        // it's in a compatible state (i.e. readOnly) and add it
        // as a component directly.
        if (Property* property = value.toCEED_Property()) {
            // make it read only if we're read only
            if (m_readOnly)
                property->m_readOnly = true;
            // ensure it's name is the our key name
            property->m_name = name;
            // add it
            m_components[name] = property;
            // if it's any other value, create a Property for it.
        } else {
            m_components[name] = new Property(name, value, value, "", "", m_readOnly, m_editorOptions);
        }
    }

    // call super to have it subscribe to our components;
    // it will call 'getComponents()' to get them.
    Property::createComponents();
}

bool DictionaryProperty::hasDefaultValue()
{
    // it doesn't really make sense to maintain a default value for this property.
    // we check whether our components have their default values instead.
    for (Property* comp : m_components.values()) {
        if (!comp->hasDefaultValue())
            return false;
    }
    return true;
}

Variant DictionaryProperty::tryParse(const QString &strValue)
{
#if 0 // TODO
    try {
        OrderedMap<QString, QVariant> value = AstHelper::parseOrderedDict(strValue, m_strValueReplacements);
        if (!value.isEmpty()) {
            // we parsed it successfully and it's a dictionary.
            // now make sure it agrees with our edit mode.
            *valid = false;

            // all changes allowed
            if (m_strReprMode == StringRepresentationMode::EditKeysAndValues)
                *valid = true;
            // keys must remain the same, values can change type or value
            else if (m_strReprMode == StringRepresentationMode::EditValuesFreeTypes)
                *valid = m_value.toMap().keys().toSet() == value.keys().toSet();
            // only values can be changed but not their type
            else if (m_strReprMode == StringRepresentationMode::EditValuesRestrictTypes) {
                // ensure keys are the same
                *valid = m_value.toMap().keys().toSet() == value.keys().toSet();
                if (*valid) {
                    // check value types
                    QVariantMap selfValue = m_value.toMap();
                    for (auto it = selfValue.begin(); it != selfValue.end(); it++) {
                        if (it.value().type() != value[it.key()].type()) { // possibly canConvert() instead?
                            valid = false;
                            break;
                        }
                    }
                }
            }
            return value;
        }
    } catch (ValueError e) {

    } catch (SyntaxError e) {

    }
#endif
    return CEED::Variant();
}

void DictionaryProperty::componentValueChanged(properties::Property *component, properties::ChangeValueReason reason)
{
    CEED::Variant::MapType map = m_value.toOrderedMap();
    map[component->m_name] = component->m_value;
    m_value = map;
    m_valueChanged.trigger(this, CEED::propertytree::properties::ChangeValueReason::ComponentValueChanged);
}

void DictionaryProperty::updateComponents(properties::ChangeValueReason reason)
{
    // check if our value and our components match
    // and if not, recreate our components.
    // we do this on this Property because our value is a dictionary
    // and its items are not fixed.

    // if our keys are the same as our components' keys, simply update the values
    CEED::Variant::MapType value = m_value.toOrderedMap();
    if (value.keys().toSet() == m_components.keys().toSet()) {
        for (QString name : value.keys()) {
            m_components[name]->setValue(value[name], reason);
        }
    } else {
        // recreate our components
        finaliseComponents();
        createComponents();
    }
}


} // namespace compositeproperties
} // namesapce propertytree
} // namespace CEED
