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

#include "propertytree_properties.h"

#include "propertytree_utility.h"

namespace CEED {
namespace propertytree {
namespace properties {

QMap<QString, PropertyCategory *> PropertyCategory::categorisePropertyList(const QList<Property *> &propertyList, const QString &unknownCategoryName)
{
    QMap<QString, PropertyCategory*> categories;
    for (Property* prop : propertyList) {
        QString catName = prop->m_category.isEmpty() ? unknownCategoryName : prop->m_category;
        if (!categories.contains(catName))
            categories[catName] = new PropertyCategory(catName);
        auto category = categories[catName];
        category->m_properties[prop->m_name] = prop;
    }

    return categories;
}

QString PropertyCategory::getSortKey(const QString &originalKey)
{
    QString name = originalKey;

    if (name[0].isLower())
        return "0" + name;
    else if (name[0].isUpper())
        return "1" + name;
    else
        return "2" + name;
}

void PropertyCategory::sortProperties(bool reverse)
{
#if 1 // FIXME
#else
    QList<Property*> properties = m_properties.values();

    std::sort(properties.begin(), properties.end(), [this](Property* a, Property* b) {
        return getSortKey(a->m_name) < getSortKey(b->m_name);
    });
    m_properties = OrderedMap(sorted(m_properties.items(), key=getSortKey, reverse=reverse));
#endif
}

/////



/////

Property::Property(const QString &name, const Variant &value, const Variant &defaultValue, const QString &category,
                   const QString &helpText, bool readOnly, const EditorOptions &editorOptions, bool createComponents)
    : m_valueChanged(0, Property::DebugMode)
    , m_componentsUpdate(0, Property::DebugMode)
{
#if 0 // TODO
    // prevent values that are Property instances themselves;
    // these have to be added as components if needed.
    if (value.canConvert<Property>()) {
        QString msg = QString("The 'value' argument of the '%1' Property can't be a Property.").arg(name);
        throw std::invalid_argument(msg.toStdString());
    }
#endif

    m_name = name;
    m_value = value;
    m_defaultValue = defaultValue;
    m_category = category;
    m_helpText = helpText;
    m_readOnly = readOnly;
    m_editorOptions = editorOptions;

    // Events
    //        m_valueChanged = PropertyEvent(0, Property.DebugMode)
    //        m_componentsUpdate = PropertyEvent(0, Property.DebugMode)

    // Create components
//       m_components = None
#if 0// can't call virtual method from constructor
    if (createComponents)
        this->createComponents();
#endif
}

void Property::finalise()
{
    /*Perform any cleanup necessary.*/
    finaliseComponents();
    m_valueChanged.clear();
}

void Property::createComponents()
{
    OrderedMap<QString, Property*> components = getComponents();
    if (!components.isEmpty()) {
        for (Property* comp : components.values()) {
            comp->m_valueChanged.subscribe<Property, &Property::componentValueChanged>(this, {ChangeValueReason::ParentValueChanged});
        }
        m_componentsUpdate.trigger(this, ComponentsUpdateType::AfterCreate);
    }
}

void Property::finaliseComponents()
{
    auto components = getComponents();
    if (!components.isEmpty()) {
        m_componentsUpdate.trigger(this, ComponentsUpdateType::BeforeDestroy);
        for (Property* comp : components.values()) {
            comp->m_valueChanged.unsubscribe<Property, &Property::componentValueChanged>(this);
            comp->finalise();
        }
        components.clear();
    }
}

CEED::VariantType Property::valueType()
{
    return m_value.type();
}

bool Property::hasDefaultValue()
{
    return m_value == m_defaultValue;
}

QString Property::valueToString()
{
    if (m_value.isValid()) {
#if 0 // TODO: not allowed by Property constructor ?
        if (auto prop = dynamic_cast<Property*>(m_value.value<Property*>()))
            return prop->valueToString();
#endif
        return m_value.toString();
    }

    return "";
}

bool Property::setValue(const CEED::Variant &value, ChangeValueReason reason)
{
    //if Property.DebugMode:
    //    print("{} ({}).setValue: Value={}, Changed={}, Reason={}".format(m_name, __class__.__name__, str(value), not value == m_value, reason))

    // Really complicated stuff :/
    //
    // This used to check if the new value is different than the current and
    // only run if it is.
    //    if m_value != value:
    // It has been removed because there are cases where a component property
    // modifies the parent's value directly (i.e. they use the same instance of
    // a class) and the check would return false because the parent's value
    // has already been changed. The problem would be that the parent wouldn't
    // update it's inner values and would not trigger the valueChanged event
    // even though it has changed.
    //
    // The check, however, was a nice way to prevent infinite recursion bugs
    // because it would stabilise the model sooner or later. We've worked around
    // that by:
    //    a) fixing the bugs :D
    //    b) the events are set to detect and disallow recursion
    //
    if (true) { //m_value != value:

        // Do not update inner properties if our own value was changed
        // in response to an inner property's value having changed.
        if (reason != ChangeValueReason::InnerValueChanged) {
            if (!tryUpdateInner(value, ChangeValueReason::WrapperValueChanged))
                return false;
        }

        m_value = value;

        // Do not update components if our own value was changed
        // in response to a component's value having changed.
        if (reason != ChangeValueReason::ComponentValueChanged)
            updateComponents(ChangeValueReason::ParentValueChanged);

        // This must be raised after updating the components because handlers
        // of the event will probably want to use the components.
        m_valueChanged.trigger(this, reason);

        return true;
    }

    return false;
}

EditorOption Property::getEditorOption(const QString &path, const EditorOption &defaultValue)
{
    return propertytree::utility::getDictionaryTreePath(m_editorOptions, path, defaultValue);
}

/////

StringWrapperProperty::StringWrapperProperty(Property *innerProperty, bool instantApply)
    : Property(innerProperty->m_name + "__wrapper__",
               innerProperty->valueToString(),
               /*defaultValue*/QStringLiteral(""),
               /*category*/"",
               /*helpText*/"",
               /*readOnly*/false,
                {{ "instantApply" , instantApply }})
{
    m_innerProperty = innerProperty;
    m_innerProperty->m_valueChanged.subscribe<StringWrapperProperty, &StringWrapperProperty::cb_innerValueChanged>(this, {ChangeValueReason::WrapperValueChanged});
}

void StringWrapperProperty::finalise()
{
    m_innerProperty->m_valueChanged.unsubscribe<StringWrapperProperty, &StringWrapperProperty::cb_innerValueChanged>(this);
    Property::finalise();
}

void StringWrapperProperty::cb_innerValueChanged(Property *innerProperty, ChangeValueReason reason)
{
    Q_UNUSED(innerProperty)
    Q_UNUSED(reason)
    Q_ASSERT(innerProperty == m_innerProperty);
    setValue(m_innerProperty->valueToString(), ChangeValueReason::InnerValueChanged);
}

bool StringWrapperProperty::tryUpdateInner(const CEED::Variant &newValue, ChangeValueReason reason)
{
    CEED::Variant value = m_innerProperty->tryParse(newValue.toString());
    if (value.isValid()) {
        // set the value to the inner property and return true
        m_innerProperty->setValue(value, reason);
        return true;
    }

    return false;
}

/////

void MultiPropertyWrapper::gatherValueData(const QList<Property *> &properties, QList<CEED::Variant> &valuesOut, QList<CEED::Variant> &defaultValuesOut)
{
#if 1
    struct CountedValue {
        int count;
        CEED::Variant value;
    };

    QList<CountedValue> values;
    QList<CountedValue> defaultValues;

    for (Property* prop : properties) {
        CEED::Variant value = prop->m_value;
        bool exists = false;
        for (auto& cv : values) {
            if (cv.value == value) {
                cv.count++;
                exists = true;
                break;
            }
        }
        if (!exists)
            values += { 1, value };

        CEED::Variant defaultValue = prop->m_defaultValue;
        exists = false;
        for (auto& cv : defaultValues) {
            if (cv.value == defaultValue) {
                cv.count++;
                exists = true;
                break;
            }
        }
        if (!exists)
            defaultValues += { 1, defaultValue };
    }

    std::sort(values.begin(), values.end(), [](const CountedValue& a, const CountedValue& b) {
        return a.count > b.count;
    });
    for (auto &kv : values)
        valuesOut += kv.value;

    std::sort(defaultValues.begin(), defaultValues.end(), [](const CountedValue& a, const CountedValue& b) {
        return a.count > b.count;
    });
    for (auto &kv : defaultValues)
        defaultValuesOut += kv.value;
#else
    // QMap requries qHash() for CEED::Variant
    QMap<CEED::Variant,int> values;
    QList<CEED::Variant int> defaultValues;

    for (Property* prop : properties) {
        CEED::Variant value = prop->m_value;
        if (values.contains(value))
            values[value] += 1;
        else
            values[value] = 1;

        CEED::Variant defaultValue = prop->m_defaultValue;
        if (defaultValues.contains(defaultValue))
            defaultValues[defaultValue] += 1;
        else
            defaultValues[defaultValue] = 1;
    }

    for (auto &kv : values)
        valuesOut += kv.first;
    std::sort(valuesOut.begin(), valuesOut.end(), [&](const CEED::Variant& a, const CEED::Variant& b) {
        return values[a] > values[b];
    });

    for (auto &kv : defaultValues)
        defaultValuesOut += kv.first;
    std::sort(defaultValuesOut.begin(), defaultValuesOut.end(), [&](const CEED::Variant& a, const CEED::Variant& b) {
        return defaultValues[a] > defaultValues[b];
    });
#endif
}

MultiPropertyWrapper *MultiPropertyWrapper::create(Property *templateProperty, const QList<Property *> &innerProperties, bool takeOwnership,
                                                   MPWCreatorFunc func)
{
    if (innerProperties.isEmpty())
        throw std::invalid_argument("The 'innerProperties' argument has no elements; at least one is required.");

    // ensure all properties have the same valueType
    CEED::VariantType valueType = templateProperty->valueType();
    for (Property* prop : innerProperties) {
        if (prop->valueType() != valueType)
            throw std::invalid_argument("The valueType() of all the inner properties must be the same.");
    }

    QList<CEED::Variant> allValues;
    QList<CEED::Variant> allDefaultValues;
    gatherValueData(innerProperties, allValues, allDefaultValues);

    templateProperty->setValue(allValues[0]);
    templateProperty->m_defaultValue = allDefaultValues[0];

    // initialise with the most used value and default value
    return func(templateProperty, innerProperties, allValues, allDefaultValues, takeOwnership);
}

MultiPropertyWrapper::MultiPropertyWrapper(Property *templateProperty, const QList<Property *> &innerProperties,
                                           const QList<Variant> &allValues, const QList<Variant> &allDefaultValues, bool takeOwnership)
    : Property(templateProperty->m_name, templateProperty->m_value, templateProperty->m_defaultValue, templateProperty->m_category,
               templateProperty->m_helpText, templateProperty->m_readOnly, templateProperty->m_editorOptions)
    , m_templateProperty(templateProperty)
    , m_innerProperties(innerProperties)
    , m_allValues(allValues)
    , m_allDefaultValues(allDefaultValues)
    , m_ownsInnerProperties(takeOwnership)
{
    // subscribe to the valueChanged event of the inner properties
    // to update our value if the value of one of them changes.
    for (Property* prop : m_innerProperties) {
        prop->m_valueChanged.subscribe<MultiPropertyWrapper, &MultiPropertyWrapper::cb_innerValueChanged>(this, {ChangeValueReason::WrapperValueChanged});
    }

    // subscribe to the valueChanged event of the template property.
    m_templateProperty->m_valueChanged.subscribe<MultiPropertyWrapper, &MultiPropertyWrapper::cb_templateValueChanged>(
                this, {ChangeValueReason::WrapperValueChanged, ChangeValueReason::ParentValueChanged});

}

void MultiPropertyWrapper::finalise()
{
    for (Property* prop : m_innerProperties) {
        prop->m_valueChanged.unsubscribe<MultiPropertyWrapper, &MultiPropertyWrapper::cb_innerValueChanged>(this);
        // if we own it, finalise it
        if (m_ownsInnerProperties)
            prop->finalise();
    }
    m_innerProperties.clear();

    m_templateProperty->m_valueChanged.unsubscribe<MultiPropertyWrapper, &MultiPropertyWrapper::cb_templateValueChanged>(this);
    m_templateProperty->finalise();
    m_templateProperty = nullptr;

    Property::finalise();
}

void MultiPropertyWrapper::createComponents()
{
    // We don't call super because we don't need to subscribe
    // to the templateProperty's components!
    m_componentsUpdate.trigger(this, ComponentsUpdateType::AfterCreate);
}

OrderedMap<QString, Property *> MultiPropertyWrapper::getComponents()
{
    return m_templateProperty->getComponents();
}

void MultiPropertyWrapper::finaliseComponents()
{
    // We don't call super because we don't actually own any
    // components, they're owned by the templateProperty
    m_componentsUpdate.trigger(this, ComponentsUpdateType::BeforeDestroy);
}

CEED::VariantType MultiPropertyWrapper::valueType()
{
    return m_templateProperty->valueType();
}

bool MultiPropertyWrapper::hasDefaultValue()
{
    return m_templateProperty->hasDefaultValue();
}

bool MultiPropertyWrapper::isStringRepresentationEditable()
{
    return m_templateProperty->isStringRepresentationEditable();
}

CEED::Variant MultiPropertyWrapper::tryParse(const QString &strValue)
{
    return m_templateProperty->tryParse(strValue);
}

void MultiPropertyWrapper::updateComponents(ChangeValueReason reason)
{
    m_templateProperty->setValue(m_value, reason);
}

QString MultiPropertyWrapper::valueToString()
{
    if (m_allValues.length() == 1)
        return Property::valueToString();
    return "<multiple values>";
}

void MultiPropertyWrapper::cb_templateValueChanged(Property *sender, ChangeValueReason reason)
{
    Q_UNUSED(sender)
    // The template's value is changed when it's components change
    // or when we change it's value directly to match our own.
    // Unnecessary? if reason == Property.ChangeValueReason.ComponentValueChanged:
    setValue(m_templateProperty->m_value, reason);
}

bool MultiPropertyWrapper::tryUpdateInner(const CEED::Variant &newValue, ChangeValueReason reason)
{
    for (Property* prop : m_innerProperties)
        prop->setValue(newValue, reason);

    return true;
}

void MultiPropertyWrapper::cb_innerValueChanged(Property *innerProperty, ChangeValueReason reason)
{
    Q_UNUSED(reason)
    Q_UNUSED(innerProperty)
    Q_ASSERT(m_innerProperties.contains(innerProperty));

    m_allValues.clear();
    m_allDefaultValues.clear();
    gatherValueData(m_innerProperties, m_allValues, m_allDefaultValues);

    setValue(m_allValues[0], ChangeValueReason::InnerValueChanged);
}

/////

SinglePropertyWrapper::SinglePropertyWrapper(Property *wrappedProperty)
    : Property(wrappedProperty->m_name,
               wrappedProperty->m_value,
               wrappedProperty->m_defaultValue,
               wrappedProperty->m_category,
               wrappedProperty->m_helpText,
               wrappedProperty->m_readOnly,
               wrappedProperty->m_editorOptions)
    , m_wrappedProperty(wrappedProperty)
{
    m_wrappedProperty->m_valueChanged.subscribe<SinglePropertyWrapper, &SinglePropertyWrapper::cb_templateValueChanged>(this,
        {ChangeValueReason::WrapperValueChanged, ChangeValueReason::ParentValueChanged});
}

void SinglePropertyWrapper::finalise()
{
    m_wrappedProperty->m_valueChanged.unsubscribe<SinglePropertyWrapper, &SinglePropertyWrapper::cb_templateValueChanged>(this);
    m_wrappedProperty->finalise();
    m_wrappedProperty = nullptr;

    Property::finalise();
}

void SinglePropertyWrapper::createComponents()
{
    // We don't call super because we don't need to subscribe
    // to the templateProperty's components!
    m_componentsUpdate.trigger(this, ComponentsUpdateType::AfterCreate);
}

OrderedMap<QString, Property *> SinglePropertyWrapper::getComponents()
{
    return m_wrappedProperty->getComponents();
}

void SinglePropertyWrapper::finaliseComponents()
{
    // We don't call super because we don't actually own any
    // components, they're owned by the templateProperty
    m_componentsUpdate.trigger(this, ComponentsUpdateType::BeforeDestroy);
}

CEED::VariantType SinglePropertyWrapper::valueType()
{
    return m_wrappedProperty->valueType();
}

bool SinglePropertyWrapper::hasDefaultValue()
{
    return m_wrappedProperty->hasDefaultValue();
}

bool SinglePropertyWrapper::isStringRepresentationEditable()
{
    return m_wrappedProperty->isStringRepresentationEditable();
}

CEED::Variant SinglePropertyWrapper::tryParse(const QString &strValue)
{
    return m_wrappedProperty->tryParse(strValue);
}

void SinglePropertyWrapper::updateComponents(ChangeValueReason reason)
{
    m_wrappedProperty->setValue(m_value, reason);
}

void SinglePropertyWrapper::cb_templateValueChanged(Property *sender, ChangeValueReason reason)
{
    // The template's value is changed when its components change
    // or when we change it's value directly to match our own.
    // Unnecessary? if reason == Property.ChangeValueReason.ComponentValueChanged:
    setValue(m_wrappedProperty->m_value, reason);
}

} // namespace properties
} // namespace propertytree
} // namespace CEED
