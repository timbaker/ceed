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

#include "ceguitype_editor_properties.h"

namespace  CEED {
namespace cegui {
namespace ceguitype_editor_properties {

//#define getattr(a,b) CEED::Variant()
//#define setattr(a,b,c)

void BaseProperty::updateComponents(ChangeValueReason reason)
{
    auto components = getComponents();
    if (!components.isEmpty()) {
        for (auto it = components.begin(); it != components.end(); it++) {
            QString compName = it.key();
            Property* compValue = it.value();
            // set component value from attribute value
            CEED::Variant value = getComponentValue(compName);
            compValue->setValue(value, reason);
        }
    }
}

void BaseProperty::componentValueChanged(Property *component, ChangeValueReason reason)
{
    Q_UNUSED(component)
    Q_UNUSED(reason)

    // set attribute value from component value
    setComponentValue(component->m_name, component->m_value);
    // trigger our value changed event directly because
    // we didn't call 'setValue()' to do it for us.
    m_valueChanged.trigger(this, ChangeValueReason::ComponentValueChanged);
}

/////

void UDimProperty::createComponents()
{
    auto value = m_value.toCEED_UDim();
    auto defaultValue = m_defaultValue.toCEED_UDim();

    m_components["Scale"] = new Property("Scale", value.scale, defaultValue.scale, "", "", m_readOnly, m_editorOptions);
    m_components["Offset"] = new Property("Offset", value.offset, defaultValue.offset, "", "", m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

CEED::Variant UDimProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::UDim::tryParse(strValue, nullptr);
    if (v) {
        return *v;
    }
    return CEED::Variant();
}

CEED::Variant UDimProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_UDim();

    if (componentName == "Scale") return value.scale;
    if (componentName == "Offset") return value.offset;

    Q_ASSERT(false);
    return CEED::Variant();
}

void UDimProperty::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_UDim();

    if (componentName == "Scale") { m_value = ceguitypes::UDim(newValue.toFloat(), oldValue.offset); return; }
    if (componentName == "Offset") { m_value = ceguitypes::UDim(oldValue.scale, newValue.toFloat()); return; }

    Q_ASSERT(false);
}

/////

void USizeProperty::createComponents()
{
    auto value = m_value.toCEED_USize();
    auto defaultValue = m_defaultValue.toCEED_USize();

    m_components["Width"] = new UDimProperty("Width",
                                             value.width,
                                             defaultValue.width,
                                             "", "", m_readOnly, m_editorOptions);
    m_components["Height"] = new UDimProperty("Height",
                                              value.height,
                                              defaultValue.height,
                                              "", "", m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant USizeProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::USize::tryParse(strValue, nullptr);
    if (v) {
        return *v;
    }
    return CEED::Variant();
}

Variant USizeProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_USize();

    if (componentName == "Width") return value.width;
    if (componentName == "Height") return value.height;

    Q_ASSERT(false);
    return CEED::Variant();
}

void USizeProperty::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_USize();

    if (componentName == "Width") { m_value = ceguitypes::USize(newValue.toCEED_UDim(), oldValue.height); return; }
    if (componentName == "Height") { m_value = ceguitypes::USize(oldValue.width, newValue.toCEED_UDim()); return; }

    Q_ASSERT(false);
}

/////

void UVector2Property::createComponents()
{
    auto value = m_value.toCEED_UVector2();
    auto defaultValue = m_defaultValue.toCEED_UVector2();

    m_components["X"] = new UDimProperty("X",
                                         value.x,
                                         defaultValue.x,
                                         "", "", m_readOnly, m_editorOptions);
    m_components["Y"] = new UDimProperty("Y",
                                         value.y,
                                         defaultValue.y,
                                         "", "", m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

CEED::Variant UVector2Property::tryParse(const QString &strValue)
{
    auto v = ceguitypes::UVector2::tryParse(strValue, nullptr);
    if (v) {
        return *v;
    }
    return CEED::Variant();
}

Variant UVector2Property::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_UVector2();

    if (componentName == "X") return value.x;
    if (componentName == "Y") return value.y;

    Q_ASSERT(false);
    return CEED::Variant();
}

void UVector2Property::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_UVector2();

    if (componentName == "X") { m_value = ceguitypes::UVector2(newValue.toCEED_UDim(), oldValue.y); return; }
    if (componentName == "Y") { m_value = ceguitypes::UVector2(oldValue.x, newValue.toCEED_UDim()); return; }

    Q_ASSERT(false);
}

/////

void URectProperty::createComponents()
{
    ceguitypes::URect value = m_value.toCEED_URect();
    ceguitypes::URect defaultValue = m_defaultValue.toCEED_URect();

    m_components["Left"] = new UDimProperty("Left", value.left, defaultValue.left, "", "",
                                            m_readOnly, m_editorOptions);
    m_components["Top"] = new UDimProperty("Top", value.top, defaultValue.top, "", "",
                                           m_readOnly, m_editorOptions);
    m_components["Right"] = new UDimProperty("Right", value.right, defaultValue.right, "", "",
                                             m_readOnly, m_editorOptions);
    m_components["Bottom"] = new UDimProperty("Bottom", value.bottom, defaultValue.bottom, "", "",
                                              m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant URectProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::URect::tryParse(strValue, nullptr);
    if (v) {
        return *v;
    }
    return CEED::Variant();
}

Variant URectProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_URect();

    if (componentName == "Left") return value.left;
    if (componentName == "Top") return value.top;
    if (componentName == "Right") return value.right;
    if (componentName == "Bottom") return value.bottom;

    Q_ASSERT(false);
    return CEED::Variant();
}

void URectProperty::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_URect();

    if (componentName == "Left") { m_value = ceguitypes::URect(newValue.toCEED_UDim(), oldValue.top, oldValue.right, oldValue.bottom); return; }
    if (componentName == "Top") { m_value = ceguitypes::URect(oldValue.left, newValue.toCEED_UDim(), oldValue.right, oldValue.bottom); return; }
    if (componentName == "Right") { m_value = ceguitypes::URect(oldValue.left, oldValue.top, newValue.toCEED_UDim(), oldValue.bottom); return; }
    if (componentName == "Bottom") { m_value = ceguitypes::URect(oldValue.left, oldValue.top, oldValue.right, newValue.toCEED_UDim()); return; }

    Q_ASSERT(false);
}

/////

void UBoxProperty::createComponents()
{
    ceguitypes::UBox value = m_value.toCEED_UBox();
    ceguitypes::UBox defaultValue = m_defaultValue.toCEED_UBox();

    m_components["Top"] = new UDimProperty("Top", value.top, defaultValue.top, "", "",
                                           m_readOnly, m_editorOptions);
    m_components["Left"] = new UDimProperty("Left", value.left, defaultValue.left, "", "",
                                            m_readOnly, m_editorOptions);
    m_components["Bottom"] = new UDimProperty("Bottom", value.bottom, defaultValue.bottom, "", "",
                                              m_readOnly, m_editorOptions);
    m_components["Right"] = new UDimProperty("Right", value.right, defaultValue.right, "", "",
                                             m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant UBoxProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::UBox::tryParse(strValue, nullptr);
    if (v) {
        return *v;
    }
    return CEED::Variant();
}

Variant UBoxProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_UBox();

    if (componentName == "Top") return value.top;
    if (componentName == "Left") return value.left;
    if (componentName == "Bottom") return value.bottom;
    if (componentName == "Right") return value.right;

    Q_ASSERT(false);
    return CEED::Variant();
}

void UBoxProperty::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_UBox();

    if (componentName == "Top") { m_value = ceguitypes::UBox(newValue.toCEED_UDim(), oldValue.left, oldValue.bottom, oldValue.right); return; }
    if (componentName == "Left") { m_value = ceguitypes::UBox(oldValue.top, newValue.toCEED_UDim(), oldValue.bottom, oldValue.right); return; }
    if (componentName == "Bottom") { m_value = ceguitypes::UBox(oldValue.top, oldValue.left, newValue.toCEED_UDim(), oldValue.right); return; }
    if (componentName == "Right") { m_value = ceguitypes::UBox(oldValue.top, oldValue.left, oldValue.bottom, newValue.toCEED_UDim()); return; }

    Q_ASSERT(false);
}

/////

void ColourProperty::createComponents()
{
    ceguitypes::Colour value = m_value.toCEED_Colour();
    ceguitypes::Colour defaultValue = m_defaultValue.toCEED_Colour();

    EditorOptions numeric = { { "min", 0 }, { "max",  255 } };
    m_editorOptions = { { "numeric",  numeric } };

    m_components["Alpha"] = new Property("Alpha", value.alpha, defaultValue.alpha, "", "",
                                         m_readOnly, m_editorOptions);
    m_components["Red"] = new Property("Red", value.red, defaultValue.red, "", "",
                                       m_readOnly, m_editorOptions);
    m_components["Green"] = new Property("Green", value.green, defaultValue.green, "", "",
                                         m_readOnly, m_editorOptions);
    m_components["Blue"] = new Property("Blue", value.blue, defaultValue.blue, "", "",
                                        m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant ColourProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::Colour::tryParse(strValue, nullptr);
    if (v)
        return *v;
    return CEED::Variant();
}

Variant ColourProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_Colour();

    if (componentName == "Alpha") return value.alpha;
    if (componentName == "Red") return value.red;
    if (componentName == "Green") return value.green;
    if (componentName == "Blue") return value.blue;

    Q_ASSERT(false);
    return CEED::Variant();
}

void ColourProperty::setComponentValue(const QString &componentName, const Variant &newValue)
{
    auto oldValue = m_value.toCEED_Colour();

    if (componentName == "Alpha") { m_value = ceguitypes::Colour(oldValue.red, oldValue.green, oldValue.blue, newValue.toInt()); return; }
    if (componentName == "Red") { m_value = ceguitypes::Colour(newValue.toInt(), oldValue.green, oldValue.blue, oldValue.alpha); return; }
    if (componentName == "Green") { m_value = ceguitypes::Colour(oldValue.red, newValue.toInt(), oldValue.blue, oldValue.alpha); return; }
    if (componentName == "Blue") { m_value = ceguitypes::Colour(oldValue.red, oldValue.green, newValue.toInt(), oldValue.alpha); return; }

    Q_ASSERT(false);
}

/////

void ColourRectProperty::createComponents()
{
    ceguitypes::ColourRect value = m_value.toCEED_ColourRect();
    ceguitypes::ColourRect defaultValue = m_defaultValue.toCEED_ColourRect();

    m_components["TopLeft"] = new ColourProperty("TopLeft", value.topLeft, defaultValue.topLeft, "", "",
                                                 m_readOnly, m_editorOptions);
    m_components["TopRight"] = new ColourProperty("TopRight", value.topRight, defaultValue.topRight, "", "",
                                                  m_readOnly, m_editorOptions);
    m_components["BottomLeft"] = new ColourProperty("BottomLeft", value.bottomLeft, defaultValue.bottomLeft, "", "",
                                                    m_readOnly, m_editorOptions);
    m_components["BottomRight"] = new ColourProperty("BottomRight", value.bottomRight, defaultValue.bottomRight, "", "",
                                                     m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant ColourRectProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::ColourRect::tryParse(strValue, nullptr);
    if (v)
        return *v;
    return CEED::Variant();
}

Variant ColourRectProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_ColourRect();

    if (componentName == "TopLeft") return value.topLeft;
    if (componentName == "TopRight") return value.topRight;
    if (componentName == "BottomLeft") return value.bottomLeft;
    if (componentName == "BottomRight") return value.bottomRight;

    Q_ASSERT(false);
    return CEED::Variant();
}

void ColourRectProperty::setComponentValue(const QString &componentName, const Variant &newValue_)
{
    auto oldValue = m_value.toCEED_ColourRect();
    auto newValue = newValue_.toCEED_Colour();

    if (componentName == "TopLeft") { m_value = ceguitypes::ColourRect(newValue, oldValue.topRight, oldValue.bottomLeft, oldValue.bottomRight); return; }
    if (componentName == "TopRight") { m_value = ceguitypes::ColourRect(oldValue.topLeft, newValue, oldValue.bottomLeft, oldValue.bottomRight); return; }
    if (componentName == "BottomLeft") { m_value = ceguitypes::ColourRect(oldValue.topLeft, oldValue.topRight, newValue, oldValue.bottomRight); return; }
    if (componentName == "BottomRight") { m_value = ceguitypes::ColourRect(oldValue.topLeft, oldValue.topRight, oldValue.bottomLeft, newValue); return; }

    Q_ASSERT(false);
}

/////

void QuaternionProperty::createComponents()
{
    ceguitypes::Quaternion value = m_value.toCEED_Quaternion();
    ceguitypes::Quaternion defaultValue = m_defaultValue.toCEED_Quaternion();

    // TODO: Set min/max/step for W, X, Y, Z. See how it's done on XYZRotationProperty.
    m_components["W"] = new Property("W", value.w, defaultValue.w, "", "",
                                     m_readOnly, m_editorOptions);
    m_components["X"] = new Property("X", value.x, defaultValue.x, "", "",
                                     m_readOnly, m_editorOptions);
    m_components["Y"] = new Property("Y", value.y, defaultValue.y, "", "",
                                     m_readOnly, m_editorOptions);
    m_components["Z"] = new Property("Z", value.z, defaultValue.z, "", "",
                                     m_readOnly, m_editorOptions);

    m_components["Degrees"] = new XYZRotationProperty("Degrees",
                                                      ceguitypes::XYZRotation::fromQuaternion(value),
                                                      ceguitypes::XYZRotation::fromQuaternion(defaultValue),
                                                      "", "",
                                                      m_readOnly,
                                                      m_editorOptions);

    BaseProperty::createComponents();
}

void QuaternionProperty::updateComponents(ChangeValueReason reason)
{
    ceguitypes::Quaternion value = m_value.toCEED_Quaternion();

    auto components = getComponents();
    if (!components.isEmpty()) {
        components["W"]->setValue(value.w, reason);
        components["X"]->setValue(value.x, reason);
        components["Y"]->setValue(value.y, reason);
        components["Z"]->setValue(value.z, reason);
        components["Degrees"]->setValue(ceguitypes::XYZRotation::fromQuaternion(value), reason);
    }
}

void QuaternionProperty::componentValueChanged(Property *component, ChangeValueReason reason)
{
    if (component->m_name == "Degrees") {
        ceguitypes::XYZRotation value = component->m_value.toCEED_XYZRotation();
        ceguitypes::Quaternion q = ceguitypes::Quaternion::convertEulerDegreesToQuaternion(value.x, value.y, value.z);
        m_components["W"]->setValue(q.w);
        m_components["X"]->setValue(q.x);
        m_components["Y"]->setValue(q.y);
        m_components["Z"]->setValue(q.z);
        m_valueChanged.trigger(this, ChangeValueReason::ComponentValueChanged);
    } else {
        BaseProperty::componentValueChanged(component, reason);
    }
}

Variant QuaternionProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::Quaternion::tryParse(strValue, nullptr);
    if (v)
        return *v;
    return CEED::Variant();
}

Variant QuaternionProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_Quaternion();

    if (componentName == "W") return value.w;
    if (componentName == "X") return value.x;
    if (componentName == "Y") return value.y;
    if (componentName == "Z") return value.z;
    if (componentName == "Degrees") {
        double x, y, z;
        value.toDegrees(x, y, z);
        return ceguitypes::XYZRotation(x, y, z);
    }

    Q_ASSERT(false);
    return CEED::Variant();
}

void QuaternionProperty::setComponentValue(const QString &componentName, const Variant &newValue_)
{
    auto oldValue = m_value.toCEED_Quaternion();

    if (componentName == "Degrees") {
        ceguitypes::XYZRotation newValue = newValue_.toCEED_XYZRotation();
        m_value = ceguitypes::Quaternion::convertEulerDegreesToQuaternion(newValue.x, newValue.y, newValue.z);
        return;
    }

    auto newValue = newValue_.toFloat();

    if (componentName == "W") { m_value = ceguitypes::Quaternion(newValue, oldValue.x, oldValue.y, oldValue.z); return; }
    if (componentName == "X") { m_value = ceguitypes::Quaternion(oldValue.w, newValue, oldValue.y, oldValue.z); return; }
    if (componentName == "Y") { m_value = ceguitypes::Quaternion(oldValue.w, oldValue.x, newValue, oldValue.z); return; }
    if (componentName == "Z") { m_value = ceguitypes::Quaternion(oldValue.w, oldValue.x, oldValue.y, newValue); return; }

    Q_ASSERT(false);

}

/////

void XYZRotationProperty::createComponents()
{
    auto value = m_value.toCEED_XYZRotation();
    auto defaultValue = m_defaultValue.toCEED_XYZRotation();

    EditorOptions numeric = { { "min", -360 }, { "max", 360 }, { "wrapping", true } };
    m_editorOptions = { { "numeric", numeric } };

    m_components["X"] = new Property("X", value.x, defaultValue.x, "", "",
                                     m_readOnly, m_editorOptions);
    m_components["Y"] = new Property("Y", value.y, defaultValue.y, "", "",
                                     m_readOnly, m_editorOptions);
    m_components["Z"] = new Property("Z", value.z, defaultValue.z, "", "",
                                     m_readOnly, m_editorOptions);

    BaseProperty::createComponents();
}

Variant XYZRotationProperty::tryParse(const QString &strValue)
{
    auto v = ceguitypes::XYZRotation::tryParse(strValue, nullptr);
    if (v)
        return *v;
    return CEED::Variant();
}

Variant XYZRotationProperty::getComponentValue(const QString &componentName)
{
    auto value = m_value.toCEED_XYZRotation();

    if (componentName == "X") return value.x;
    if (componentName == "Y") return value.y;
    if (componentName == "Z") return value.z;

    Q_ASSERT(false);
    return CEED::Variant();
}

void XYZRotationProperty::setComponentValue(const QString &componentName, const Variant &newValue_)
{
    auto oldValue = m_value.toCEED_XYZRotation();
    auto newValue = newValue_.toFloat();

    if (componentName == "X") { m_value = ceguitypes::XYZRotation(newValue, oldValue.y, oldValue.z); return; }
    if (componentName == "Y") { m_value = ceguitypes::XYZRotation(oldValue.x, newValue, oldValue.z); return; }
    if (componentName == "Z") { m_value = ceguitypes::XYZRotation(oldValue.x, oldValue.y, newValue); return; }

    Q_ASSERT(false);

}

}
}
}
