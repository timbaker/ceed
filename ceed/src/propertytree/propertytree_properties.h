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

#ifndef CEED_propertytree_properties_
#define CEED_propertytree_properties_

#include "CEEDBase.h"

/**The basic properties.

Property -- The base class for all properties, has name, value, etc.
PropertyCategory -- A category, groups properties together.
PropertyEvent -- Custom event implementation for the Property system.
PropertyEventSubscription -- A subscription to a PropertyEvent.
StringWrapperProperty -- Special purpose property to edit the string representation of another property.
MultiPropertyWrapper -- Special purpose property used to group many properties of the same type in one.
EnumValue -- Interface for properties that have a predetermined list of possible values, like enums.
*/

#include "cegui/ceguitypes.h"

#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>

#include "delegate.h"

#include <functional>

namespace CEED {
namespace propertytree {
namespace properties {

class Property;

enum class ChangeValueReason
{
    Unknown,
    ComponentValueChanged,
    ParentValueChanged,
    Editor,
    InnerValueChanged,
    WrapperValueChanged,
};

enum class ComponentsUpdateType
{
    AfterCreate = 0,
    BeforeDestroy = 1,
};

class BasePropertyCategory
{
public:
    QString m_name;
    OrderedMap<QString, Property*> m_properties;
};

/*!
\brief PropertyCategory

A category for properties.
    Categories have a name and hold a list of properties.

*/
class PropertyCategory : public BasePropertyCategory
{
public:
//    QString m_name;
//    OrderedMap<QString, Property*> m_properties;

    /**Initialise the instance with the specified name.*/
    PropertyCategory(const QString& name)
    {
        m_name = name;
    }

    /**Given a list of properties, create categories and add the
    properties to them based on their 'category' field.

    The unknownCategoryName is used for a category that holds all
    properties that have no 'category' specified.
    */
    static QMap<QString, PropertyCategory*> categorisePropertyList(const QList<Property*>& propertyList, const QString& unknownCategoryName = "Unknown");

    QString getSortKey(const QString& originalKey);

    // sort properties by name adding non-capitalised ones before capitalised ones
    void sortProperties(bool reverse = false);
};

/*!
\brief PropertyEventSubscription

A subscription to a PropertyEvent.
*/

/* The guts of this are based on the Lumix::Delegate class */
template<typename T> class PropertyEventSubscription;

template <typename R>
class PropertyEventSubscription
{
private:
    typedef void* InstancePtr;
    typedef R (*InternalFunction)(InstancePtr);
    struct Stub
    {
        InstancePtr first;
        InternalFunction second;
    };
    Stub m_stub;

    template <R (*Function)()> static LUMIX_FORCE_INLINE R FunctionStub(InstancePtr) { return (Function)(); }

    template <class C, R (C::*Function)()> static LUMIX_FORCE_INLINE R ClassMethodStub(InstancePtr instance)
    {
        return (static_cast<C*>(instance)->*Function)();
    }

    template <class C, R (C::*Function)() const> static LUMIX_FORCE_INLINE R ClassMethodStub(InstancePtr instance)
    {
        return (static_cast<C*>(instance)->*Function)();
    }


public:
    QList<ChangeValueReason> m_excludedReasons;
    QList<ChangeValueReason> m_includedReasons;

    /**Initialise the subscription to call the specified callback.

    callback -- The callable to call, will take the arguments specified
                during the call to PropertyEvent.trigger().
    excludedReasons -- If the 'reason' argument of PropertyEvent.trigger()
                is in this set, the callback will not be called that time.
                Has higher priority than 'includedReasons'.
    includedReasons -- Like 'excludedReasons' but specifies the values that
                will cause the callback to be called. If None, all values
                are valid.
                Has lower priority than 'excludedReasons'
    */
    PropertyEventSubscription(const QList<ChangeValueReason>& excludedReasons, const QList<ChangeValueReason>&includedReasons)
    {
        m_stub.first = nullptr;
        m_stub.second = nullptr;
        m_excludedReasons = excludedReasons;
        m_includedReasons = includedReasons;
    }

    /**Return true if the callback should be called for the specified reason.*/
    bool isValidForReason(ChangeValueReason reason)
    {
        if (m_includedReasons.isEmpty() || m_includedReasons.contains(reason))
            if (m_excludedReasons.isEmpty() || !m_excludedReasons.contains(reason))
                return true;
        return false;
    }

    template <R (*Function)()> void bind(void)
    {
        m_stub.first = nullptr;
        m_stub.second = &FunctionStub<Function>;
    }

    template <class C, R (C::*Function)()> void bind(C* instance)
    {
        m_stub.first = instance;
        m_stub.second = &ClassMethodStub<C, Function>;
    }

    template <class C, R(C::*Function)() const> void bind(C* instance)
    {
        m_stub.first = instance;
        m_stub.second = &ClassMethodStub<C, Function>;
    }

    R invoke() const
    {
        ASSERT(m_stub.second != nullptr);
        return m_stub.second(m_stub.first);
    }

    bool operator==(const PropertyEventSubscription<R()>& rhs)
    {
        return m_stub.first == rhs.m_stub.first && m_stub.second == rhs.m_stub.second;
    }
};

template <typename R, typename... Args>
class PropertyEventSubscription<R(Args...)>
{
private:
    typedef void* InstancePtr;
    typedef R (*InternalFunction)(InstancePtr, Args...);
    struct Stub
    {
        InstancePtr first;
        InternalFunction second;
    };
    Stub m_stub;

    template <R (*Function)(Args...)> static LUMIX_FORCE_INLINE R FunctionStub(InstancePtr, Args... args)
    {
        return (Function)(args...);
    }

    template <class C, R (C::*Function)(Args...)>
    static LUMIX_FORCE_INLINE R ClassMethodStub(InstancePtr instance, Args... args)
    {
        return (static_cast<C*>(instance)->*Function)(args...);
    }


public:
    QList<ChangeValueReason> m_excludedReasons;
    QList<ChangeValueReason> m_includedReasons;

    /**Initialise the subscription to call the specified callback.

    callback -- The callable to call, will take the arguments specified
                during the call to PropertyEvent.trigger().
    excludedReasons -- If the 'reason' argument of PropertyEvent.trigger()
                is in this set, the callback will not be called that time.
                Has higher priority than 'includedReasons'.
    includedReasons -- Like 'excludedReasons' but specifies the values that
                will cause the callback to be called. If None, all values
                are valid.
                Has lower priority than 'excludedReasons'
    */
    PropertyEventSubscription(const QList<ChangeValueReason>& excludedReasons, const QList<ChangeValueReason>&includedReasons)
    {
        m_stub.first = nullptr;
        m_stub.second = nullptr;
        m_excludedReasons = excludedReasons;
        m_includedReasons = includedReasons;
    }

    PropertyEventSubscription()
    {
        m_stub.first = nullptr;
        m_stub.second = nullptr;
    }

    /**Return true if the callback should be called for the specified reason.*/
    bool isValidForReason(ChangeValueReason reason)
    {
        if (m_includedReasons.isEmpty() || m_includedReasons.contains(reason))
            if (m_excludedReasons.isEmpty() || !m_excludedReasons.contains(reason))
                return true;
        return false;
    }

    template <R (*Function)(Args...)> void bind(void)
    {
        m_stub.first = nullptr;
        m_stub.second = &FunctionStub<Function>;
    }

    template <class C, R (C::*Function)(Args...)> void bind(C* instance)
    {
        m_stub.first = instance;
        m_stub.second = &ClassMethodStub<C, Function>;
    }

    R invoke(Args... args) const
    {
        Q_ASSERT(m_stub.second != nullptr);
        return m_stub.second(m_stub.first, args...);
    }

    bool operator==(const PropertyEventSubscription<R(Args...)>& rhs)
    {
        return m_stub.first == rhs.m_stub.first && m_stub.second == rhs.m_stub.second;
    }

#if 0 // FIXME: callback determines equality
    void __hash__()
        /**Return the hash of the callback.

        The __hash__, __eq__ and __ne__ methods have been re-implemented so that it's
        possible to manage this subscription via it's callback, without holding a
        reference to the subscription instance.
        */
        return hash(m_callback)

    void __eq__(self, other):
        /**Return true if this subscription's callback is equal to the
        'other' argument (subscription or callback).

        See the notes on '__hash__'
        */
        if isinstance(other, PropertyEventSubscription):
            return m_callback == other.callback
        if callable(other) and m_callback == other:
            return true
        return false

    void __ne__(self, other):
        /**Inverted '__eq__'.

        See the notes on '__hash__' and '__eq__'
        */
        return not __eq__(other)
#endif
};

template <typename T> class PropertyEvent;

template <typename R, typename... Args>
class PropertyEvent<R(Args...)>
{
public:
    QList<PropertyEventSubscription<R(Args...)>> m_subscriptions;
    int m_maxRecursionDepth;
    bool m_assertOnDepthExceeded;
    int m_recursionDepth;

    /**Custom event.

    An event can have subscribers and can guard against recursion.

    maxRecursionDepth -- 0 (zero) means no recursion at all, -1 means unlimited.
    assertOnDepthExcessed -- Will call 'assert false' if the recursion depth exceeds the maximum.
    */
    PropertyEvent(int maxRecursionDepth=-1, bool assertOnDepthExceeded=false)
    {
        m_maxRecursionDepth = maxRecursionDepth;
        m_assertOnDepthExceeded = assertOnDepthExceeded;
        m_recursionDepth = -1;
    }

    void clear()
    {
        /**Remove all subscriptions.*/
        m_subscriptions.clear();
    }

    /**Shortcut to create and add a subscription.

    Multiple subscriptions with the same callback are not supported.
    */
    template <typename C, R (C::*Function)(Args...)>
    void subscribe(C* instance,
                   const QList<ChangeValueReason>& excludedReasons = QList<ChangeValueReason>(),
                   const QList<ChangeValueReason>& includedReasons = QList<ChangeValueReason>()
            )
    {
        PropertyEventSubscription<R(Args...)> cb(excludedReasons, includedReasons);
        cb.template bind<C, Function>(instance);
        m_subscriptions.append(cb);
    }

    /**Remove a subscription.

    eventSubOrCallback -- An event subscription instance or the callback
                        of a subscription to remove.
    safe -- If true, check that the subscription exists to avoid
            KeyErrors.
    */
    template <typename C, R (C::*Function)(Args...)>
    void unsubscribe(C* instance)
    {
        PropertyEventSubscription<R(Args...)> cb;
        cb.template bind<C, Function>(instance);
        for (int i = 0; i < m_subscriptions.size(); ++i) {
            if (m_subscriptions[i] == cb) {
                m_subscriptions.removeAt(i);
                break;
            }
        }
    }

    /**Raise the event by calling all subscriptions that are valid
    for the specified reason.
    */
    void trigger(Property* property, ChangeValueReason reason)
    {
        struct RAII {
            int& m_target;
            RAII(int& target)
                : m_target(target)
            {
                m_target++;
            }
            ~RAII()
            {
                m_target--;
            }
        };

        // increase the counter first, so the first call sets it to 0.
        RAII rd(m_recursionDepth);

        // if there's a max set and we're over it
        if ((m_maxRecursionDepth != -1) && (m_recursionDepth > m_maxRecursionDepth)) {
            // if we want assertions
            if (m_assertOnDepthExceeded) {
                throw RuntimeError(QString("Excessive recursion detected ({} when the max is {}).").arg(m_recursionDepth).arg(m_maxRecursionDepth));
            }
            // bail out
            return;
        }

        for (auto& subscription : m_subscriptions) {
            if (subscription.isValidForReason(reason))
                subscription.invoke(property, reason);
        }
    }

    void trigger(Property* property, ComponentsUpdateType reason)
    {
        try {
            // increase the counter first, so the first call sets it to 0.
            m_recursionDepth += 1;
            // if there's a max set and we're over it
            if ((m_maxRecursionDepth != -1) && (m_recursionDepth > m_maxRecursionDepth)) {
                // if we want assertions
                if (m_assertOnDepthExceeded) {
                    throw RuntimeError(QString("Excessive recursion detected ({} when the max is {}).").arg(m_recursionDepth).arg(m_maxRecursionDepth));
                }
                // bail out
                return;
            }

            for (auto& subscription : m_subscriptions) {
                subscription.invoke(property, reason);
            }
        }
        catch (...) {
            m_recursionDepth -= 1;
            throw;
        }
    }
};

typedef QVariant EditorOption;
typedef QVariantMap EditorOptions;

/*!
\brief Property

A property which is the base for all properties.

    The most important fields of a property are 'name' and 'value'.
    A property instance should be able to return the type of its value
    and has a simple mechanism to notify others when its value changes.

*/
class Property
{
public:
    // TODO: Is it necessary to set this to false for releases or is
    // the release made with optimisations?
    static const bool DebugMode = true;

    QString m_name;
    CEED::Variant m_value;
    CEED::Variant m_defaultValue;
    QString m_category;
    QString m_helpText;
    bool m_readOnly;

    EditorOptions m_editorOptions;

    PropertyEvent<void(Property*, ChangeValueReason)> m_valueChanged;
    PropertyEvent<void(Property*, ComponentsUpdateType)> m_componentsUpdate;
    OrderedMap<QString, Property*> m_components;

    /**Initialise an instance using the specified parameters.

    The 'category' field is usually a string that is used by
    'PropertyCategory.categorisePropertyList()' to place the
    property in a category.

    In the default implementation, the 'editorOptions' argument
    should be a dictionary of options that will be passed to the
    editor of the property's value. See getEditorOption().
    */
    Property(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true);

    virtual void finalise();

    /**Create an OrderedDict with the component-properties that make up
    this property.

    The default implementation is incomplete,
    it simply subscribes to the components' valueChanged event
    to be able to update this property's own value when
    their value changes. It also calls raiseComponentsUpdate().

    Implementors should create the components, make sure they
    are accessible from getComponents() and then call this
    as super().
    */
    virtual void createComponents();

    /**Return the OrderedDict of the components, or None.*/
    virtual OrderedMap<QString, Property*> getComponents()
    {
        return m_components;
    }

    /**Clean up components.

    The default implementation is usually enough, it will
    unsubscribe from the components' events, finalise them
    and clear the components field. It will also call
    raiseComponentsUpdate().
    */
    virtual void finaliseComponents();

    /**Return the type of this property's value.
    The default implementation simply returns the Python type()
    of the current value.
    */
    virtual CEED::VariantType valueType();

    virtual bool hasDefaultValue();

    /**Return a string representation of the current value.
    */
    virtual QString valueToString();

    /**Return true if the property supports editing its string representation,
    in addition to editing its components.*/
    //pylint: disable-msg=R0201
    // "Method could be a function"
    // No, it couldn't, it's meant to be overriden but provides
    // the default implementation.
    virtual bool isStringRepresentationEditable()
    {
        return false;
    }

    /**Parse the specified string value and return
    a tuple with the parsed value and a boolean
    specifying success or failure.
    */
    virtual CEED::Variant tryParse(const QString& strValue)
    {
        Q_UNUSED(strValue)
        return CEED::Variant();
    }

    /**Change the current value to the one specified
    and notify all subscribers of the change. Return true
    if the value was changed, otherwise false.

    If the property has components, this method is responsible
    for updating their values, if necessary. The default
    implementation does this by calling 'updateComponents()'.

    If the property is a wrapper property (a proxy to another
    property), this method tries to set the value of the
    inner property first and bails out if it can't. The default
    implementation does this by calling 'tryUpdateInner()'.
    */
    virtual bool setValue(const Variant &value, ChangeValueReason reason = ChangeValueReason::Unknown);

    /**Update this property's components (if any) to match this property's value.*/
    virtual void updateComponents(ChangeValueReason reason=ChangeValueReason::Unknown)
    {
        Q_UNUSED(reason)
    }

    /**Try to update the inner properties (if any) to the new value.

    Return true on success, false on failure.
    */
    virtual bool tryUpdateInner(const CEED::Variant& newValue, ChangeValueReason reason=ChangeValueReason::Unknown)
    {
        Q_UNUSED(newValue)
        Q_UNUSED(reason)
        return true;
    }

    /**Callback called when a component's value changes.

    This will generally call setValue() to update this instance's
    value in response to the component's value change. If this
    happens, the call to setValue() should use ChangeValueReason.ComponentValueChanged.

    See the DictionaryProperty for a different implementation.
    */
    virtual void componentValueChanged(Property* component, ChangeValueReason reason)
    {
        Q_UNUSED(component)
        Q_UNUSED(reason)
    }

    /**Get the value of the editor option at the specified path string.

    Return 'defaultValue' if the option/path can't be found.
    */
    EditorOption getEditorOption(const QString& path, const EditorOption& defaultValue = EditorOption());
};

/*!
\brief StringWrapperProperty

Special purpose property used to wrap the string value
    of another property so it can be edited.

*/
class StringWrapperProperty : public Property
{
public:
    Property* m_innerProperty;

    StringWrapperProperty(Property* innerProperty, bool instantApply=false);

    void finalise();

    void cb_innerValueChanged(Property* innerProperty, ChangeValueReason reason);

    CEED::Variant tryParse(const QString& strValue) override { return strValue; }

    bool tryUpdateInner(const CEED::Variant& newValue, ChangeValueReason reason=ChangeValueReason::Unknown) override;
};

typedef std::function<MultiPropertyWrapper*(Property*, const QList<Property*>&, const QList<CEED::Variant>&, const QList<CEED::Variant>&, bool)> MPWCreatorFunc;

/*!
\brief MultiPropertyWrapper

Special purpose property used to group many properties of the same type in one.
*/
class MultiPropertyWrapper : public Property
{
public:
    Property* m_templateProperty;
    QList<Property*> m_innerProperties;
    bool m_ownsInnerProperties;
    QList<CEED::Variant> m_allValues;
    QList<CEED::Variant> m_allDefaultValues;

    static Property* makeTemplateProperty(Property* templateProperty, const QList<Property*>& innerProperties);

    static
    /**Go through the properties and return the unique values and default values found,
    ordered by use count.
    */
    void gatherValueData(const QList<Property*>& properties, QList<CEED::Variant> &valuesOut, QList<CEED::Variant> &defaultValuesOut);

    /**Initialise the instance with the specified properties.

    templateProperty -- A newly created instance of a property of the same type
                        as the properties that are to be wrapped. This should be
                        already initialised with the proper name, category, settings,
                        etc. It will be used internally and it will be owned by
                        this wrapper.
    innerProperties -- The list of properties to wrap, must have at least one
                        property and all properties must have the same type
                        or an error is raised.
    takeOwnership -- Boolean flag indicating whether this wrapper should take
                        ownership of the inner properties, destroying them when
                        it is destroyed.
    */
    static MultiPropertyWrapper *create(Property* templateProperty, const QList<Property*>& innerProperties, bool takeOwnership,
                                        MPWCreatorFunc func);

    MultiPropertyWrapper(Property* templateProperty, const QList<Property *> &innerProperties, const QList<CEED::Variant>& allValues,
                         const QList<CEED::Variant>& allDefaultValues, bool takeOwnership);

    void finalise() override;

    void createComponents() override;

    OrderedMap<QString, Property*> getComponents() override;

    void finaliseComponents() override;

    CEED::VariantType valueType() override;

    bool hasDefaultValue() override;

    bool isStringRepresentationEditable() override;

    CEED::Variant tryParse(const QString& strValue) override;

    void updateComponents(ChangeValueReason reason = ChangeValueReason::Unknown) override;

    QString valueToString() override;

    void cb_templateValueChanged(Property* sender, ChangeValueReason reason);

    bool tryUpdateInner(const CEED::Variant &newValue, ChangeValueReason reason = ChangeValueReason::Unknown);

    void cb_innerValueChanged(Property* innerProperty, ChangeValueReason reason);
};

/*!
\brief SinglePropertyWrapper

A property wrapper that can be used for inheritation and in that way, for overriding functions.
    Owns a property of a specific type.
*/
class SinglePropertyWrapper : public Property
{
public:
    Property *m_wrappedProperty;

    /**Initialise the instance with the specified properties.

    templateProperty -- A newly created instance of a property of the same type
                        as the properties that are to be wrapped. This should be
                        already initialised with the proper name, category, settings,
                        etc. It will be used internally and it will be owned by
                        this wrapper.
    */
    SinglePropertyWrapper(Property* wrappedProperty);

    void finalise() override;

    void createComponents() override;

    OrderedMap<QString, Property*> getComponents() override;

    void finaliseComponents() override;

    CEED::VariantType valueType() override;

    bool hasDefaultValue() override;

    bool isStringRepresentationEditable() override;

    CEED::Variant tryParse(const QString& strValue) override;

    void updateComponents(ChangeValueReason reason=ChangeValueReason::Unknown) override;

    void cb_templateValueChanged(Property* sender, ChangeValueReason reason);
};

/*!
\brief EnumValue

Interface for properties that have a predetermined list
    of possible values, like enums.

    Used by the EnumValuePropertyEditor (combo box).

*/
class EnumValue
{
public:
    /**Return a dictionary of all possible values and their display names.*/
    // tnb: I changed this from value:name to name:value
    virtual OrderedMap<QString, CEED::Variant> getEnumValues() = 0;
};

} // namespace properties
} // namespace propertytree
} // namespace CEED

#endif
