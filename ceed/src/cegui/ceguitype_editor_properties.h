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

#ifndef CEED_cegui_ceguitype_editor_properties_
#define CEED_cegui_ceguitype_editor_properties_

#include "CEEDBase.h"

/**Lightweight CEGUI property value types that can parse and write text.*/

#include "cegui/ceguitypes.h"

#include "propertytree/propertytree_properties.h"

namespace  CEED {
namespace cegui {
namespace ceguitype_editor_properties {

using namespace propertytree::properties;

/*!
\brief BaseProperty

Base class for all Property types.

    Note that, by default, it expects the components to map
    directly to an attribute of it's value; with the first letter in lower case.

    For example the UDimProperty has two components, 'Scale' and 'Offset' and
    it also uses the UDim type that has the 'scale' and 'offset' attribute values.

*/
class BaseProperty : public Property
{
public:
    BaseProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : Property(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override
    {
        for (auto *prop : m_components) {
            prop->createComponents();
        }

        Property::createComponents();
    }

    OrderedMap<QString, Property*> getComponents() override
    {
        return m_components;
    }

#if 1
    virtual CEED::Variant getComponentValue(const QString& componentName) = 0;
    virtual void setComponentValue(const QString& componentName, const CEED::Variant& newValue) = 0;
#else
    /**Get the attribute name from the component name.*/
    static
    QString getAttrName(const QString& componentName)
    {
        return componentName[0].toLower() + componentName.mid(1);
    }
#endif

    void updateComponents(ChangeValueReason reason = ChangeValueReason::Unknown);

    void componentValueChanged(Property* component, ChangeValueReason reason);
};


/*!
\brief UDimProperty

Property for UDim values.
*/
class UDimProperty : public BaseProperty
{
public:
    UDimProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};


/*!
\brief USizeProperty

Property for USize values.
*/
class USizeProperty : public BaseProperty
{
public:
    USizeProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};


/*!
\brief UVector2Property

Property for UVector2 values.
*/
class UVector2Property : public BaseProperty
{
public:
    UVector2Property(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};


/*!
\brief URectProperty

Property for URect values.
*/
class URectProperty : public BaseProperty
{
public:
    URectProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};


/*!
\brief UBoxProperty

Property for UBox values.
*/
class UBoxProperty : public BaseProperty
{
public:
    UBoxProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
                 const QString& category = "", const QString& helpText = "", bool readOnly = false,
                 const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};

/*!
\brief QuaternionProperty

Property for Quaternion values.
*/
class QuaternionProperty : public BaseProperty
{
public:
    QuaternionProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {
        Q_ASSERT(!value.isValid() || value.isType(VariantType::CEED_Quaternion));
        Q_ASSERT(!defaultValue.isValid() || defaultValue.isType(VariantType::CEED_Quaternion));
    }

    void createComponents() override;

    void updateComponents(ChangeValueReason reason = ChangeValueReason::Unknown);

    void componentValueChanged(Property* component, ChangeValueReason reason);

    bool isStringRepresentationEditable() override
    {
      return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};


/*!
\brief XYZRotationProperty

Property for XYZRotation values.
*/
class XYZRotationProperty : public BaseProperty
{
public:
    XYZRotationProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {
        Q_ASSERT(!value.isValid() || value.isType(VariantType::CEED_XYZRotation));
        Q_ASSERT(!defaultValue.isValid() || defaultValue.isType(VariantType::CEED_XYZRotation));
    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

     CEED::Variant tryParse(const QString &strValue) override;

     CEED::Variant getComponentValue(const QString& componentName) override;
     void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};

/*!
\brief ColourProperty

Property for Colour values.
*/
class ColourProperty : public BaseProperty
{
public:
    ColourProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {

    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};

/*!
\brief ColourRectProperty

Property for ColourRect values.
*/
class ColourRectProperty : public BaseProperty
{
public:
    ColourRectProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const EditorOptions& editorOptions = EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {
        Q_ASSERT(!value.isValid() || value.isType(VariantType::CEED_ColourRect));
        Q_ASSERT(!defaultValue.isValid() || defaultValue.isType(VariantType::CEED_ColourRect));
    }

    void createComponents() override;

    bool isStringRepresentationEditable() override
    {
        return true;
    }

    CEED::Variant tryParse(const QString &strValue) override;

    CEED::Variant getComponentValue(const QString& componentName) override;
    void setComponentValue(const QString& componentName, const CEED::Variant& newValue) override;
};

} // namespace ceguitype_editor_properties
} // namespace cegui
} // namespace  CEED

#endif
