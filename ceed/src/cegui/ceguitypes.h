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

#ifndef CEED_cegui_ceguitypes_
#define CEED_cegui_ceguitypes_

#include "CEEDBase.h"

/**Lightweight CEGUI property value types that can parse and write text.*/

#include "CEGUI/falagard/XMLEnumHelper.h"

#include <QColor>
#include <QtMath>

namespace CEED {
class Variant;
}

namespace CEED {
namespace cegui {
namespace ceguitypes {


/*!
\brief Base

Abstract base class for all value types.
*/
class Base
{
public:
    static const QString floatPattern;
#if 1 // TODO
#define BASE_fromString(cls) \
    static cls fromString(const QString& strValue) { \
        auto value = tryParse(strValue); \
        if (!value) \
            throw ValueError(QString("Could not convert string to %1: '%1'.").arg(#cls).arg(strValue)); \
        return *value; \
    }
#else
    static
    QVariant tryParse(const QString& strValue, optional<>& target)
        /**Parse the specified string value and return
        a tuple with the parsed value and a boolean
        specifying success or failure.

        If 'target' is not None, update its attributes
        with the parse value.
        */
        raise NotImplementedError("'tryParse()' not implemented for class '%s'" % cls.__name__)

    static
    def tryToString(cls, ceguiObject):
        /**
        Translates a given object into its string representation
        :param ceguiObject:
        :return: str
        */
        raise NotImplementedError("'toString()' not implemented for class '%s'" % cls.__name__)

    static
    def tryToCeguiType(cls, stringValue):
        /**
        Translates the given object into its original CEGUI type
        :param stringValue: unicode
        :return:
        */
        raise NotImplementedError("'toCeguiType()' not implemented for class '%s'" % cls.__name__)

    static
    def fromString(cls, strValue):
        /**Parse the specified string value and return
        a new instance of this type.
        Raise a ValueError if the string can't be parsed.
        */
        value, valid = cls.tryParse(strValue)
        if not valid:
            raise ValueError("Could not convert string to %s: '%s'." % (cls.__name__, strValue))
        return value

    static
    def toString(cls, ceguiObject):
        /**
        Translates a given object into its string representation
        :param ceguiObject:
        :return: str
        */

        if type(ceguiObject) == unicode:
            return ceguiObject

        try:
            return cls.tryToString(ceguiObject)
        except:
            raise ValueError("Could not convert CEGUI object to string: '%s'." % (cls.__name__, ceguiObject))

    static
    def toCeguiType(cls, ceguiObject):
        /**
        Translates a given object into its string representation
        :param ceguiObject:
        :return:
        */

        try:
            return cls.tryToCeguiType(ceguiObject)
        except:
            raise ValueError("Could not convert CEGUI object to string: '%s'." % (cls.__name__, ceguiObject))

    static
    def getPropertyType(cls):
        raise NotImplementedError("'getPropertyType()' not implemented for class '%s'" % cls.__name__)

    def parse(self, strValue):
        return self.__class__.tryParse(strValue, self)[1]

    @abc.abstractmethod
    def __hash__()
        pass

    @abc.abstractmethod
    def __eq__(self, other):
        pass

    @abc.abstractmethod
    def __repr__()
        pass

    def __ne__(self, other):
        /**Default implementation of __ne__, negates __eq__!*/
        return not (self.__eq__(other))
  #endif
};



/*!
\brief UDim

UDim
*/
class UDim : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<UDim> tryParse(const QString& strValue, UDim* target = nullptr)
    {
        rex.indexIn(strValue);
        if (rex.captureCount() != 2)
            return nonstd::nullopt;

        float scale = rex.cap(1).toFloat();
        float offset = rex.cap(2).toFloat();

        if (target == nullptr)
            return UDim(scale, offset);
        else {
            target->scale = scale;
            target->offset = offset;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::UDim& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::UDim>::toString(ceguiObject);
    }

    static
    CEGUI::UDim tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::UDim>::fromString(stringValue);
    }

    BASE_fromString(UDim)

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

    float scale;
    float offset;

    UDim(float scale=0.0, float offset=0.0)
      : Base()
      , scale(scale)
      , offset(offset)
    {
    }

    bool operator==(const UDim& rhs) const { return scale == rhs.scale && offset == rhs.offset; }

    // no scientific notation, 16 digits precision, remove trailing zeroes
    QString toString() const { return QString("{%1, %2}").arg(scale, 0, 'g', 16).arg(offset, 0, 'g', 16); }

#if 0
    def __hash__()
        return hash((self.scale, self.offset))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.scale == other.scale and self.offset == other.offset
        return false

    def __repr__()
        def fmt(value):
            // no scientific notation, 16 digits precision, remove trailing zeroes
            return "{:.16f}".format(value).rstrip("0").rstrip(".")

        return "{{{}, {}}}".format(fmt(self.scale), fmt(self.offset))
#endif
};


/*!
\brief USize

USize (uses UDim)
*/
class USize : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<USize> tryParse(const QString& strValue, USize* target = nullptr)
    {
        rex.indexIn(strValue);
        if (rex.captureCount() == 0)
            return nonstd::nullopt;

        float widthScale = rex.cap(1).toFloat();
        float widthOffset = rex.cap(2).toFloat();
        float heightScale = rex.cap(3).toFloat();
        float heightOffset = rex.cap(4).toFloat();

        if (target == nullptr)
            return USize(UDim(widthScale, widthOffset), UDim(heightScale, heightOffset));
        else {
            target->width.scale = widthScale;
            target->width.offset = widthOffset;
            target->height.scale = heightScale;
            target->height.offset = heightOffset;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::USize& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::USize>::toString(ceguiObject);
    }

    static
    CEGUI::USize tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::USize>::fromString(stringValue);
    }

    BASE_fromString(USize)

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

    UDim width;
    UDim height;

    USize(const UDim& width=UDim(), const UDim& height=UDim())
        : Base()
        , width(width)
        , height(height)
    {

    }

    bool operator==(const USize& rhs) const { return width == rhs.width && height == rhs.height; }

    QString toString() const { return QString("{%1, %2}").arg(width.toString()).arg(height.toString()); }
#if 0
    def __hash__()
        return hash((hash(self.width), hash(self.height)))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.width == other.width and self.height == other.height
        return false

    def __repr__()
        return "{{{}, {}}}".format(self.width, self.height)
#endif
};

/*!
\brief UVector2

UVector2 (uses UDim)

    Very similar to USize.

*/
class UVector2 : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<UVector2> tryParse(const QString& strValue, UVector2* target = nullptr)
    {
        rex.indexIn(strValue);
        if (rex.captureCount() == 0)
            return nonstd::nullopt;

        float xScale = rex.cap(1).toFloat();
        float xOffset = rex.cap(2).toFloat();
        float yScale = rex.cap(3).toFloat();
        float yOffset = rex.cap(4).toFloat();

        if (target == nullptr)
            return UVector2(UDim(xScale, xOffset), UDim(yScale, yOffset));
        else {
            target->x.scale = xScale;
            target->x.offset = xOffset;
            target->y.scale = yScale;
            target->y.offset = yOffset;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::UVector2& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::UVector2>::toString(ceguiObject);
    }

    static
    CEGUI::UVector2 tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::UVector2>::fromString(stringValue);
    }

    BASE_fromString(UVector2)

    UDim x;
    UDim y;

    UVector2(const UDim& x=UDim(), const UDim& y=UDim())
        : Base()
        , x(x)
        , y(y)
    {

    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

    bool operator==(const UVector2& rhs) const { return x == rhs.x && y == rhs.y; }

    QString toString() const { return QString("{%1, %2}").arg(x.toString(), y.toString()); }

#if 0
    def __hash__()
        return hash((hash(self.x), hash(self.y)))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.x == other.x and self.y == other.y
        return false

    def __repr__()
        return "{{{}, {}}}".format(self.x, self.y)

    static
    def getPropertyType(cls):
        from ceguitype_editor_properties import UVector2Property
        return UVector2Property
#endif
};

/*!
\brief URect

URect (uses UDim)
*/
class URect : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<URect> tryParse(const QString& strValue, URect* target = nullptr)
    {
        rex.indexIn(strValue);
        if (rex.captureCount() == 0)
            return nonstd::nullopt;

        float leftScale = rex.cap(1).toFloat();
        float leftOffset = rex.cap(2).toFloat();
        float topScale = rex.cap(3).toFloat();
        float topOffset = rex.cap(4).toFloat();
        float rightScale = rex.cap(5).toFloat();
        float rightOffset = rex.cap(6).toFloat();
        float bottomScale = rex.cap(7).toFloat();
        float bottomOffset = rex.cap(8).toFloat();

        if (target == nullptr)
            return URect(UDim(leftScale, leftOffset), UDim(topScale, topOffset), UDim(rightScale, rightOffset), UDim(bottomScale, bottomOffset));
        else {
            target->left.scale = leftScale;
            target->left.offset = leftOffset;
            target->top.scale = topScale;
            target->top.offset = topOffset;
            target->right.scale = rightScale;
            target->right.offset = rightOffset;
            target->bottom.scale = bottomScale;
            target->bottom.offset = bottomOffset;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::URect& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::URect>::toString(ceguiObject);
    }

    static
    CEGUI::URect tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::URect>::fromString(stringValue);
    }

    BASE_fromString(URect)

    UDim left;
    UDim top;
    UDim right;
    UDim bottom;

    URect(const UDim& left=UDim(), const UDim& top=UDim(), const UDim& right=UDim(), const UDim& bottom=UDim())
        :Base()
        , left(left)
        , top(top)
        , right(right)
        , bottom(bottom)
    {

    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

    bool operator==(const URect& rhs) const { return left == rhs.left && top == rhs.top && right == rhs.right && bottom == rhs.bottom; }

    QString toString() const { return QString("{%1, %2, %3, %4}").arg(left.toString()).arg(top.toString()).arg(right.toString()).arg(bottom.toString()); }

#if 0
    def __hash__()
        return hash((hash(self.left), hash(self.top), hash(self.right), hash(self.bottom)))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.left == other.left and self.top == other.top and self.right == other.right and self.bottom == other.bottom
        return false

    def __repr__()
        return "{{{}, {}, {}, {}}}".format(self.left, self.top, self.right, self.bottom)

    static
    def getPropertyType(cls):
        from ceguitype_editor_properties import URectProperty
        return URectProperty
#endif
};

// Added in C++ version
class UBox : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<UBox> tryParse(const QString& strValue, UBox* target = nullptr)
    {
        rex.indexIn(strValue);
        if (rex.captureCount() != 8)
            return nonstd::nullopt;

        float topScale = rex.cap(1).toFloat();
        float topOffset = rex.cap(2).toFloat();
        float leftScale = rex.cap(3).toFloat();
        float leftOffset = rex.cap(4).toFloat();
        float bottomScale = rex.cap(5).toFloat();
        float bottomOffset = rex.cap(6).toFloat();
        float rightScale = rex.cap(7).toFloat();
        float rightOffset = rex.cap(8).toFloat();

        if (target == nullptr)
            return UBox(UDim(topScale, topOffset),
                        UDim(leftScale, leftOffset),
                        UDim(bottomScale, bottomOffset),
                        UDim(rightScale, rightOffset));
        else {
            target->top.scale = topScale;
            target->top.offset = topOffset;
            target->left.scale = leftScale;
            target->left.offset = leftOffset;
            target->bottom.scale = bottomScale;
            target->bottom.offset = bottomOffset;
            target->right.scale = rightScale;
            target->right.offset = rightOffset;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::UBox& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::UBox>::toString(ceguiObject);
    }

    static
    CEGUI::UBox tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::UBox>::fromString(stringValue);
    }

    BASE_fromString(UBox)

    UDim top;
    UDim left;
    UDim bottom;
    UDim right;

    UBox(const UDim& top=UDim(), const UDim& left=UDim(), const UDim& bottom=UDim(), const UDim& right=UDim())
        :Base()
        , top(top)
        , left(left)
        , bottom(bottom)
        , right(right)
    {

    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

    bool operator==(const UBox& rhs) const { return top == rhs.top && left == rhs.left && bottom == rhs.bottom && right == rhs.right; }

    QString toString() const { return QString("{top:%1,left:%2,bottom:%3,right:%4}").arg(top.toString()).arg(left.toString()).arg(bottom.toString()).arg(right.toString()); }
};

/* Interface used by EnumValuePropertyEditor */
class EnumValue
{
public:
    virtual OrderedMap<QString, CEED::Variant> getEnumValues() const = 0;
};

/**Base class for types that have a predetermined list of possible values.*/
template <typename T>
class EnumBase : public Base, public EnumValue
{
public:

    // key-value pairs of allowed values
    // the key should be the value and the value should be the display name.
    // reversed key-value order for C++ version
    static OrderedMap<QString, CEED::Variant> enumValues;

    static
    optional<T> tryParse(const QString& strValue, T* target = nullptr)
    {
        bool ok = false;
        for (auto& value : enumValues.values()) {
            if (value.toString() == strValue) {
                ok = true;
                break;
            }
        }

        if (!ok)
            return nonstd::nullopt;

        QString value = strValue;

        if (target == nullptr)
            return T(value);
        else {
            target->value = value;
            return *target;
        }
    }

    BASE_fromString(T)

    QString value;

    EnumBase(const QString& value="")
        : Base()
        , value(value)
    {

    }

#if 0
    def __hash__()
        return hash(self.value)

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.value == other.value
        return false

    def __repr__()
        return str(self.value)

    static
    def getPropertyType(cls):
        from ceguitype_editor_properties import BaseProperty
        return BaseProperty
#endif

    bool operator==(const EnumBase& rhs) const
    {
        return value == rhs.value;
    }

    QString toString() const
    {
        return value;
    }

    OrderedMap<QString, CEED::Variant> getEnumValues() const override
    {
        return enumValues;
    }
};


/*!
\brief AspectMode

AspectMode
*/
class AspectMode : public EnumBase<AspectMode>
{
public:
    AspectMode(const QString& value = "Ignore")
        : EnumBase(value)
    {
    }

    static
    CEGUI::String tryToString(CEGUI::AspectMode ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::AspectMode>::toString(ceguiObject);
    }

    static
    CEGUI::AspectMode tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::AspectMode>::fromString(stringValue);
    }
};


/*!
\brief HorizontalAlignment

HorizontalAlignment
*/
class HorizontalAlignment : public EnumBase<HorizontalAlignment>
{
public:

    HorizontalAlignment(const QString& value = "Left")
        : EnumBase(value)
    {
    }

    static
    CEGUI::String tryToString(CEGUI::HorizontalAlignment ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::HorizontalAlignment>::toString(ceguiObject);
    }

    static
    CEGUI::HorizontalAlignment tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::HorizontalAlignment>::fromString(stringValue);
    }
};


/*!
\brief VerticalAlignment

VerticalAlignment
*/
class VerticalAlignment : public EnumBase<VerticalAlignment>
{
public:
    VerticalAlignment(const QString& value="Top")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::VerticalAlignment ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::VerticalAlignment>::toString(ceguiObject);
    }

    static
    CEGUI::VerticalAlignment tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::VerticalAlignment>::fromString(stringValue);
    }
};


/*!
\brief HorizontalFormatting

HorizontalFormatting
*/
class HorizontalFormatting : public EnumBase<HorizontalFormatting>
{
public:
    HorizontalFormatting(const QString& value="LeftAligned")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::HorizontalFormatting ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::HorizontalFormatting>::toString(ceguiObject);
    }

    static
    CEGUI::HorizontalFormatting tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::HorizontalFormatting>::fromString(stringValue);
    }
};


/*!
\brief VerticalFormatting

VerticalFormatting
*/
class VerticalFormatting : public EnumBase<VerticalFormatting>
{
public:
    VerticalFormatting(const QString& value="TopAligned")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::VerticalFormatting ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::VerticalFormatting>::toString(ceguiObject);
    }

    static
    CEGUI::VerticalFormatting tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::VerticalFormatting>::fromString(stringValue);
    }
};


/*!
\brief HorizontalTextFormatting

HorizontalTextFormatting
*/
class HorizontalTextFormatting : public EnumBase<HorizontalTextFormatting>
{
public:
    HorizontalTextFormatting(const QString& value="LeftAligned")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::HorizontalTextFormatting ceguiObject)
    {
        return CEGUI::FalagardXMLHelper<CEGUI::HorizontalTextFormatting>::toString(ceguiObject);
    }

    static
    CEGUI::HorizontalTextFormatting tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::FalagardXMLHelper<CEGUI::HorizontalTextFormatting>::fromString(stringValue);
    }
};


/*!
\brief VerticalTextFormatting

VerticalTextFormatting
*/
class VerticalTextFormatting : public EnumBase<VerticalTextFormatting>
{
public:
    VerticalTextFormatting(const QString& value="TopAligned")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::VerticalTextFormatting ceguiObject)
    {
        return CEGUI::FalagardXMLHelper<CEGUI::VerticalTextFormatting>::toString(ceguiObject);
    }

    static
    CEGUI::VerticalTextFormatting tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::FalagardXMLHelper<CEGUI::VerticalTextFormatting>::fromString(stringValue);
    }
};


/*!
\brief WindowUpdateMode

WindowUpdateMode
*/
class WindowUpdateMode : public EnumBase<WindowUpdateMode>
{
public:
    WindowUpdateMode(const QString& value="Always")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::WindowUpdateMode ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::WindowUpdateMode>::toString(ceguiObject);
    }

    static
    CEGUI::WindowUpdateMode tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::WindowUpdateMode>::fromString(stringValue);
    }
};


/*!
\brief SortMode

ItemListBase::SortMode
*/
class SortMode : public EnumBase<SortMode>
{
public:
    SortMode(const QString& value="Ascending")
        : EnumBase(value)
    {

    }

    static
    CEGUI::String tryToString(CEGUI::ItemListBase::SortMode ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::ItemListBase::SortMode>::toString(ceguiObject);
    }

    static
    CEGUI::ItemListBase::SortMode tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::ItemListBase::SortMode>::fromString(stringValue);
    }
};


/*!
\brief Quaternion

Quaternion
*/
class Quaternion : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<Quaternion> tryParse(const QString& strValue, Quaternion* target = nullptr)
    {
        rex.indexIn(strValue);

        if (rex.captureCount() < 3)
            return nonstd::nullopt;

        // TODO: Python version allowed x,y,z (without w)

        float w = rex.cap(1).toFloat();
        float x = rex.cap(2).toFloat();
        float y = rex.cap(3).toFloat();
        float z = rex.cap(4).toFloat();

        if (target == nullptr) {
            return Quaternion(w, x, y, z);
        } else {
            target->w = w;
            target->x = x;
            target->y = y;
            target->z = z;
            return *target;
        }

#if 0
        match = re.match(cls.rex, strValue)
        if match is None:
            return None, false

        w = None
        if match.group(1) is not None:
            // we have the 'w' component
            w = float(match.group(1))
        x = float(match.group(2))
        y = float(match.group(3))
        z = float(match.group(4))

        if target is None:
            target = cls(x, y, z, w)
        else:
            if w is not None:
                target->x = x
                target->y = y
                target->z = z
                target->w = w
            else:
                (target->w, target->x, target->y, target->z) = Quaternion.convertEulerDegreesToQuaternion(x, y, z)

        return target, true
#endif
    }

    static
    CEGUI::Quaternion tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::Quaternion>::fromString(stringValue);
    }

    /**Convert the x, y, z angles (in radians) to w, x, y, z (quaternion).

    The order of rotation: 1) around Z 2) around Y 3) around X

    Copied from CEGUI, Quaternion.cpp
    */
    static
    Quaternion convertEulerRadiansToQuaternion(qreal x, qreal y, qreal z)
    {
        qreal sin_z_2 = qSin(0.5 * z);
        qreal sin_y_2 = qSin(0.5 * y);
        qreal sin_x_2 = qSin(0.5 * x);

        qreal cos_z_2 = qCos(0.5 * z);
        qreal cos_y_2 = qCos(0.5 * y);
        qreal cos_x_2 = qCos(0.5 * x);

        return Quaternion(cos_z_2 * cos_y_2 * cos_x_2 + sin_z_2 * sin_y_2 * sin_x_2,
                          cos_z_2 * cos_y_2 * sin_x_2 - sin_z_2 * sin_y_2 * cos_x_2,
                          cos_z_2 * sin_y_2 * cos_x_2 + sin_z_2 * cos_y_2 * sin_x_2,
                          sin_z_2 * cos_y_2 * cos_x_2 - cos_z_2 * sin_y_2 * sin_x_2);
    }

    static
    Quaternion convertEulerDegreesToQuaternion(qreal x, qreal y, qreal z)
    {
        qreal d2r = M_PI / 180.0;

        return Quaternion::convertEulerRadiansToQuaternion(x * d2r, y * d2r, z * d2r);
    }

    /**Return a tuple of yaw, pitch, roll.

      Ported from http://stackoverflow.com/a/1031235

      Note: This is probably wrong but it's a start.
      */
    static
    void convertQuaternionToYPR(const Quaternion& q, double& yaw, double& pitch, double& roll)
    {
        const double w2 = q.w*q.w;
        const double x2 = q.x*q.x;
        const double y2 = q.y*q.y;
        const double z2 = q.z*q.z;
        const double unitLength = w2 + x2 + y2 + z2;    // Normalised == 1, otherwise correction divisor.
        const double abcd = q.w*q.x + q.y*q.z;
        const double eps = 1e-7;    // TODO: pick from your math lib instead of hardcoding.
        const double pi = M_PI;
        if (abcd > (0.5-eps)*unitLength)
        {
            yaw = 2 * atan2(q.y, q.w);
            pitch = pi;
            roll = 0;
        }
        else if (abcd < (-0.5+eps)*unitLength)
        {
            yaw = -2 * ::atan2(q.y, q.w);
            pitch = -pi;
            roll = 0;
        }
        else
        {
            const double adbc = q.w*q.z - q.x*q.y;
            const double acbd = q.w*q.y - q.x*q.z;
            yaw = ::atan2(2*adbc, 1 - 2*(z2+x2));
            pitch = ::asin(2*abcd/unitLength);
            roll = ::atan2(2*acbd, 1 - 2*(y2+x2));
        }
    }
#if 0
    static
    def convertQuaternionToYPR(w, x, y, z):
        // FIXME: Please fix me! (See above)
        w2 = w * w
        x2 = x * x
        y2 = y * y
        z2 = z * z
        unitLength = w2 + x2 + y2 + z2  // Normalised == 1, otherwise correction divisor.
        abcd = w * x + y * z
        eps = Quaternion.machineEpsilon()
        pi = math.pi

        yaw = 0
        pitch = 0
        roll = 0

        if abcd > (0.5 - eps) * unitLength:
            yaw = 2 * math.atan2(y, w)
            pitch = pi
            roll = 0
        elif abcd < (-0.5 + eps) * unitLength:
            yaw = -2 * math.atan2(y, w)
            pitch = -pi
            roll = 0
        else:
            adbc = w * z - x * y
            acbd = w * y - x * z
            yaw = math.atan2(2 * adbc, 1 - 2 * (z2 + x2))
            pitch = math.asin(2 * abcd / unitLength)
            roll = math.atan2(2 * acbd, 1 - 2 * (y2 + x2))

        return (yaw, pitch, roll)
#endif
    static
    void convertQuaternionToDegrees(const Quaternion& q, double& pitch, double& roll, double& yaw)
    {
        qreal r2d = 180.0 / M_PI;

        Quaternion::convertQuaternionToYPR(q, yaw, pitch, roll);
        pitch *= r2d;
        roll *= r2d;
        yaw *= r2d;
    }

    float w, x, y, z;

    Quaternion(float w=0.0, float x=0.0, float y=0.0, float z=1.0)
        : Base()
        , w(w)
        , x(x)
        , y(y)
        , z(z)
    {
    }

    BASE_fromString(Quaternion)

    bool operator==(const Quaternion& other) const
    {
        return w == other.w && x == other.x && y == other.y && z == other.z;
    }

    QString toString() const
    {
        // no scientific notation, 16 digits precision, remove trailing zeroes
        return QString("w:%1 x:%2 y:%3 z:%4")
                .arg(w, 0, 'g', 16)
                .arg(x, 0, 'g', 16)
                .arg(y, 0, 'g', 16)
                .arg(z, 0, 'g', 16);
    }

#if 0
    def __hash__()
        return hash((self.x, self.y, self.z, self.w))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.x == other.x and self.y == other.y and self.z == other.z and self.w == other.w
        return false

    def __repr__()
        def fmt(value):
            // no scientific notation, 16 digits precision, remove trailing zeroes
            return "{:.16f}".format(value).rstrip("0").rstrip(".")

        return "w:{} x:{} y:{} z:{}".format(fmt(self.w), fmt(self.x), fmt(self.y), fmt(self.z))
#endif

    void toDegrees(double& x, double& y, double& z) const
    {
        convertQuaternionToDegrees(*this, x, y, z);
    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();
};


class XYZRotation : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<XYZRotation> tryParse(const QString& strValue, XYZRotation* target)
    {
        rex.indexIn(strValue);

        if (rex.captureCount() != 3)
            return nonstd::nullopt;

        float x = rex.cap(1).toFloat();
        float y = rex.cap(2).toFloat();
        float z = rex.cap(3).toFloat();

        if (target == nullptr) {
            return XYZRotation(x, y, z);
        } else {
            target->x = x;
            target->y = y;
            target->z = z;
            return *target;
        }
    }

    static
    XYZRotation fromQuaternion(const Quaternion& quat)
    {
        double x, y, z;
        quat.toDegrees(x, y, z);
        return XYZRotation(x, y, z);
    }

    float x, y, z;

    XYZRotation(float x=0.0, float y=0.0, float z=0.0)
        : Base()
        , x(x)
        , y(y)
        , z(z)
    {

    }

    bool operator==(const XYZRotation& other) const
    {
        return x == other.x && y == other.y && z == other.z;
    }

    QString toString() const
    {
        // no scientific notation, 16 digits precision, remove trailing zeroes
        return QString("x:%1 y:%2 z:%3")
                .arg(x, 0, 'g', 16)
                .arg(y, 0, 'g', 16)
                .arg(z, 0, 'g', 16);
    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();

#if 0
    def __hash__()
        return hash((self.x, self.y, self.z))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.x == other.x and self.y == other.y and self.z == other.z
        return false

    def __repr__()
        def fmt(value):
            // no scientific notation, 16 digits precision, remove trailing zeroes
            return "{:.16f}".format(value).rstrip("0").rstrip(".")

        return "x:{} y:{} z:{}".format(fmt(self.x), fmt(self.y), fmt(self.z))

    static
    def getPropertyType(cls):
        from ceguitype_editor_properties import XYZRotationProperty
        return XYZRotationProperty
#endif
};

/*!
\brief Colour

Colour

    Can parse hex strings like:
        [#]RGB
        [#]RRGGBB
        [#]AARRGGBB
    and named colors like 'green', 'skyblue', 'whitesmoke' using Color.

*/
class Colour : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<Colour> tryParse(const QString& strValue, Colour* target = nullptr)
    {
        int alpha = 0xFF;
        int red = 0;
        int green = 0;
        int blue = 0;

        rex.indexIn(strValue);
        if (rex.captureCount() != 1) {
            if (QColor::isValidColor(strValue)) {
                QColor qtColor(strValue);
                alpha = qtColor.alpha();
                red = qtColor.red();
                green = qtColor.green();
                blue = qtColor.blue();
            } else {
                return nonstd::nullopt;
            }
        } else {
            QString value = rex.cap(1);
#if 1
            if (value.length() == 3) {
                // short CSS style RGB
                red = value.mid(0, 1).toInt(nullptr, 16) * 2;
                green = value.mid(1, 1).toInt(nullptr, 16) * 2;
                blue = value.mid(2, 1).toInt(nullptr, 16) * 2;
            } else if (value.length() == 6) {
                // CSS RGB
                red = value.mid(0, 2).toInt(nullptr, 16);
                green = value.mid(2, 2).toInt(nullptr, 16);
                blue = value.mid(4, 2).toInt(nullptr, 16);
            } else if (value.length() == 8) {
                // ARGB
                alpha = value.mid(0, 2).toInt(nullptr, 16);
                red = value.mid(2, 2).toInt(nullptr, 16);
                green = value.mid(4, 2).toInt(nullptr, 16);
                blue = value.mid(6, 2).toInt(nullptr, 16);
            } else {
                return nonstd::nullopt;
            }
#else
            if (value.length() == 3) {
                // short CSS style RGB
                red = int(value[0] * 2, 16);
                green = int(value[1] * 2, 16);
                blue = int(value[2] * 2, 16);
            } else if (value.length() == 6) {
                // CSS RGB
                red = int(value[0:2], 16);
                green = int(value[2:4], 16);
                blue = int(value[4:6], 16);
            } else if (value.length()) == 8) {
                // ARGB
                alpha = int(value[0:2], 16);
                red = int(value[2:4], 16);
                green = int(value[4:6], 16);
                blue = int(value[6:8], 16);
            } else {
                return nonstd::nullopt;
            }
#endif
        }

        if (target == nullptr)
            return Colour(red, green, blue, alpha);
        else {
            target->red = red;
            target->green = green;
            target->blue = blue;
            target->alpha = alpha;
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::Colour& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::Colour>::toString(ceguiObject);
    }

    static
    CEGUI::Colour tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::Colour>::fromString(stringValue);
    }

    static
    Colour fromQColor(const QColor& qtColor)
    {
        return Colour(qtColor.red(), qtColor.green(), qtColor.blue(), qtColor.alpha());
    }

    static
    Colour fromColour(const Colour& other)
    {
        return Colour(other.red, other.green, other.blue, other.alpha);
    }
  
    QColor toQColor()
    {
        return QColor(red, green, blue, alpha);
    }

    BASE_fromString(Colour)

    int red, green, blue, alpha;

    Colour(int red=0, int green=0, int blue=0, int alpha=255)
        : Base()
    {
        this->red = int(red);
        this->green = int(green);
        this->blue = int(blue);
        this->alpha = int(alpha);
    }

    uint getARGB() const
    {
        return blue | (green << 8) | (red << 16) | (alpha << 24);
    }

//    def __hash__()
//        return hash(self.getARGB())

    bool operator==(const Colour& other) const
    {
        return getARGB() == other.getARGB();
    }

    QString toString() const
    {
        return QString("%1").arg((uint)getARGB(), 8, 16, QChar('0')); // %08X
    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();
};


/*!
\brief ColourRect

ColourRect

    Can parse strings like:
        colour
        or
        tl:colour tr:colour bl:colour br:colour

        where colour is:
            [#]RGB
            [#]RRGGBB
            [#]AARRGGBB
            or a named color like 'green', 'skyblue', 'whitesmoke'

*/
class ColourRect : public Base
{
public:
    static const QString pattern;
    static const QRegExp rex;

    static
    optional<ColourRect> tryParse(const QString& strValue, ColourRect* target = nullptr)
    {
        QMap<QString, Colour> values;
        const QString fields[] = { "tl", "tr", "bl", "br" };

        if (strValue.contains("tl:")) {
            // try to parse as full ColourRect
            rex.indexIn(strValue);
            if (rex.captureCount() != 4) {
                return nonstd::nullopt;
            }
            for (int i = 0; i < 4; i++) {
                auto colour = Colour::tryParse(rex.cap(i+1));
                if (!colour)
                    return nonstd::nullopt;
                values[fields[i]] = *colour;
            }
        } else {
            // try to parse as one Colour
            auto colour = Colour::tryParse(strValue);
            if (!colour)
                return nonstd::nullopt;
            // assign to all values of ColourRect
            // make copies or else all Colour components will be
            // the same and changing one will change all.
            for (int i = 0; i < 4; i++) {
                values[fields[i]] = *colour;
            }
        }

        if (target == nullptr) {
            return ColourRect(values["tl"], values["tr"], values["bl"], values["br"]);
        } else {
            target->topLeft = values["tl"];
            target->topRight = values["tr"];
            target->bottomLeft = values["bl"];
            target->bottomRight = values["br"];
            return *target;
        }
    }

    static
    CEGUI::String tryToString(const CEGUI::ColourRect& ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::ColourRect>::toString(ceguiObject);
    }

    static
    CEGUI::ColourRect tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::ColourRect>::fromString(stringValue);
    }

    BASE_fromString(ColourRect)

    Colour topLeft;
    Colour topRight;
    Colour bottomLeft;
    Colour bottomRight;

    ColourRect(const Colour& tl=Colour(), const Colour& tr=Colour(), const Colour& bl=Colour(), const Colour& br=Colour())
        : Base()
    {
        topLeft = tl;
        topRight = tr;
        bottomLeft = bl;
        bottomRight = br;
    }

    bool operator==(const ColourRect& other) const
    {
        return topLeft == other.topLeft &&
                topRight == other.topRight &&
                bottomLeft == other.bottomLeft &&
                bottomRight == other.bottomRight;
    }

    QString toString() const
    {
        return QString("tl:%1 tr:%2 bl:%3 br:%4")
                .arg(topLeft.toString())
                .arg(topRight.toString())
                .arg(bottomLeft.toString())
                .arg(bottomRight.toString());
    }

    static
    propertysetinspector::PropertyFactoryBase* getPropertyType();
#if 0
    def __hash__(self)
    {
        return hash((hash(self.topLeft), hash(self.topRight), hash(self.bottomLeft), hash(self.bottomRight)))
    }

    def __eq__(self, other)
    {
        if isinstance(other, self.__class__):
            return self.topLeft == other.topLeft and self.topRight == other.topRight and self.bottomLeft == other.bottomLeft and self.bottomRight == other.bottomRight
        return false
    }

    def __repr__(self)
    {
        return "tl:{} tr:{} bl:{} br:{}".format(self.topLeft, self.topRight, self.bottomLeft, self.bottomRight)
        }

    static
    def getPropertyType(cls):
        from ceguitype_editor_properties import ColourRectProperty
        return ColourRectProperty
#endif
};


/*!
\brief StringWrapper

Simple string that does no parsing but allows us to map editors to it
*/
class StringWrapper : public Base
{
public:
    QString value;

    StringWrapper(const QString& value)
        : Base()
        , value(value)
    {
    }

    QString toString() const
    {
        return value;
    }

    bool operator==(const StringWrapper& other) const
    {
        return value == other.value;
    }

#if 0
    def __hash__()
        return hash(self.value)

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return self.value == other.value
        return false

    def __repr__()
        return self.value
#endif
      static
      propertysetinspector::PropertyFactoryBase* getPropertyType();
};


class FontRef : public StringWrapper
{
public:
    FontRef(const QString& value = "")
        : StringWrapper(value)
    {
    }

    static
    optional<FontRef> tryParse(const QString& strValue, FontRef* target = nullptr)
    {
        if (target != nullptr)
            target->value = strValue;
        return FontRef(strValue);
    }

    static
    CEGUI::String tryToString(const CEGUI::Font* ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::Font*>::toString(ceguiObject);
    }

    static
    const CEGUI::Font* tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::Font*>::fromString(stringValue);
    }

    BASE_fromString(FontRef)
};


class ImageRef : public StringWrapper
{
public:
    ImageRef(const QString& value = "")
        : StringWrapper(value)
    {
    }

    static
    optional<ImageRef> tryParse(const QString& strValue, ImageRef* target = nullptr)
    {
        if (target != nullptr)
            target->value = strValue;
        return ImageRef(strValue);
    }

    static
    CEGUI::String tryToString(const CEGUI::Image* ceguiObject)
    {
        return CEGUI::PropertyHelper<CEGUI::Image*>::toString(ceguiObject);
    }

    static
    const CEGUI::Image* tryToCeguiType(const CEGUI::String& stringValue)
    {
        return CEGUI::PropertyHelper<CEGUI::Image*>::fromString(stringValue);
    }

    BASE_fromString(ImageRef)
};

} // namespace ceguitypes
} // namespace cegui

class Variant
{
public:
    typedef OrderedMap<QString, Variant> MapType;

    VariantType type() const { return m_type; }
    bool isType(VariantType type) const { return m_type == type; }
    bool isValid() const { return m_type != VariantType::NONE; }

    Variant() : m_type(VariantType::NONE) {}
    Variant(const CEED::Variant& other);
    Variant(bool v) : m_type(VariantType::Bool), m_Bool(v) {}
    Variant(float v) : m_type(VariantType::Float), m_Float(v) {}
    Variant(int v) : m_type(VariantType::Int), m_Int(v) {}
    Variant(uint v) : m_type(VariantType::UInt), m_UInt(v) {}
    Variant(const MapType& v) : m_type(VariantType::OrderedMap), m_OrderedMap(v) {}
    Variant(const QString& v) : m_type(VariantType::QString), m_QString(v) {}

    // CEED wrappers
    Variant(const cegui::ceguitypes::UDim& v) : m_type(VariantType::CEED_UDim), m_CEED_UDim(v) {}
    Variant(const cegui::ceguitypes::USize& v) : m_type(VariantType::CEED_USize), m_CEED_USize(v) {}
    Variant(const cegui::ceguitypes::UVector2& v) : m_type(VariantType::CEED_UVector2), m_CEED_UVector2(v) {}
    Variant(const cegui::ceguitypes::URect& v) : m_type(VariantType::CEED_URect), m_CEED_URect(v) {}
    Variant(const cegui::ceguitypes::UBox& v) : m_type(VariantType::CEED_UBox), m_CEED_UBox(v) {}
    Variant(const cegui::ceguitypes::Colour& v) : m_type(VariantType::CEED_Colour), m_CEED_Colour(v) {}
    Variant(const cegui::ceguitypes::ColourRect& v) : m_type(VariantType::CEED_ColourRect), m_CEED_ColourRect(v) {}
    Variant(const cegui::ceguitypes::AspectMode& v) : m_type(VariantType::CEED_AspectMode), m_CEED_AspectMode(v) {}
    Variant(const cegui::ceguitypes::HorizontalAlignment& v) : m_type(VariantType::CEED_HorizontalAlignment), m_CEED_HorizontalAlignment(v) {}
    Variant(const cegui::ceguitypes::VerticalAlignment& v) : m_type(VariantType::CEED_VerticalAlignment), m_CEED_VerticalAlignment(v) {}
    Variant(const cegui::ceguitypes::WindowUpdateMode& v) : m_type(VariantType::CEED_WindowUpdateMode), m_CEED_WindowUpdateMode(v) {}
    Variant(const cegui::ceguitypes::Quaternion& v) : m_type(VariantType::CEED_Quaternion), m_CEED_Quaternion(v) {}
    Variant(const cegui::ceguitypes::XYZRotation& v) : m_type(VariantType::CEED_XYZRotation), m_CEED_XYZRotation(v) {}
    Variant(const cegui::ceguitypes::HorizontalFormatting& v) : m_type(VariantType::CEED_HorizontalFormatting), m_CEED_HorizontalFormatting(v) {}
    Variant(const cegui::ceguitypes::VerticalFormatting& v) : m_type(VariantType::CEED_VerticalFormatting), m_CEED_VerticalFormatting(v) {}
    Variant(const cegui::ceguitypes::HorizontalTextFormatting& v) : m_type(VariantType::CEED_HorizontalTextFormatting), m_CEED_HorizontalTextFormatting(v) {}
    Variant(const cegui::ceguitypes::VerticalTextFormatting& v) : m_type(VariantType::CEED_VerticalTextFormatting), m_CEED_VerticalTextFormatting(v) {}
    Variant(const cegui::ceguitypes::SortMode& v) : m_type(VariantType::CEED_SortMode), m_CEED_SortMode(v) {}
    Variant(const cegui::ceguitypes::FontRef& v) : m_type(VariantType::CEED_FontRef), m_CEED_FontRef(v) {}
    Variant(const cegui::ceguitypes::ImageRef& v) : m_type(VariantType::CEED_ImageRef), m_CEED_ImageRef(v) {}
    Variant(propertytree::properties::Property* v) : m_type(VariantType::CEED_Property), m_CEED_Property(v) {}

    // CEGUI types
    Variant(const CEGUI::Colour& v) : m_type(VariantType::CEGUI_Colour), m_CEGUI_Colour(v) {}
    Variant(const CEGUI::ColourRect& v) : m_type(VariantType::CEGUI_ColourRect), m_CEGUI_ColourRect(v) {}
    Variant(const CEGUI::Font* v) : m_type(VariantType::CEGUI_Font), m_CEGUI_Font(const_cast<CEGUI::Font*>(v)) {}
    Variant(const CEGUI::Image* v) : m_type(VariantType::CEGUI_Image), m_CEGUI_Image(const_cast<CEGUI::Image*>(v)) {}
    Variant(const CEGUI::Quaternion& v) : m_type(VariantType::CEGUI_Quaternion), m_CEGUI_Quaternion(v) {}
    Variant(const CEGUI::String& v) : m_type(VariantType::CEGUI_String), m_CEGUI_String(v) {}
    Variant(const CEGUI::UDim& v) : m_type(VariantType::CEGUI_UDim), m_CEGUI_UDim(v) {}
    Variant(const CEGUI::USize& v) : m_type(VariantType::CEGUI_USize), m_CEGUI_USize(v) {}
    Variant(const CEGUI::UVector2& v) : m_type(VariantType::CEGUI_UVector2), m_CEGUI_UVector2(v) {}
    Variant(const CEGUI::URect& v) : m_type(VariantType::CEGUI_URect), m_CEGUI_URect(v) {}
    Variant(const CEGUI::UBox& v) : m_type(VariantType::CEGUI_UBox), m_CEGUI_UBox(v) {}
    Variant(CEGUI::AspectMode v) : m_type(VariantType::CEGUI_AspectMode), m_CEGUI_AspectMode(v) {}
    Variant(CEGUI::HorizontalAlignment v) : m_type(VariantType::CEGUI_HorizontalAlignment), m_CEGUI_HorizontalAlignment(v) {}
    Variant(CEGUI::VerticalAlignment v) : m_type(VariantType::CEGUI_VerticalAlignment), m_CEGUI_VerticalAlignment(v) {}
    Variant(CEGUI::HorizontalFormatting v) : m_type(VariantType::CEGUI_HorizontalFormatting), m_CEGUI_HorizontalFormatting(v) {}
    Variant(CEGUI::VerticalFormatting v) : m_type(VariantType::CEGUI_VerticalFormatting), m_CEGUI_VerticalFormatting(v) {}
    Variant(CEGUI::HorizontalTextFormatting v) : m_type(VariantType::CEGUI_HorizontalTextFormatting), m_CEGUI_HorizontalTextFormatting(v) {}
    Variant(CEGUI::VerticalTextFormatting v) : m_type(VariantType::CEGUI_VerticalTextFormatting), m_CEGUI_VerticalTextFormatting(v) {}
    Variant(CEGUI::ItemListBase::SortMode v) : m_type(VariantType::CEGUI_SortMode), m_CEGUI_SortMode(v) {}
    Variant(CEGUI::WindowUpdateMode v) : m_type(VariantType::CEGUI_WindowUpdateMode), m_CEGUI_WindowUpdateMode(v) {}

    // QVariant
    Variant(QVariant& qvariant);

    bool toBool() const { return isType(VariantType::Bool) ? m_Bool : false; }
    float toFloat() const { return isType(VariantType::Float) ? m_Float : 0.f; }
    int toInt() const { return isType(VariantType::Int) ? m_Int : 0; }
    uint toUInt() const { return isType(VariantType::UInt) ? m_UInt : 0; }
    OrderedMap<QString, Variant> toEnumValues() const;
    MapType toOrderedMap() const { return isType(VariantType::OrderedMap) ? m_OrderedMap : MapType(); }

    // CEED wrappers
    cegui::ceguitypes::UDim toCEED_UDim() const { return isType(VariantType::CEED_UDim) ? m_CEED_UDim : cegui::ceguitypes::UDim(); }
    cegui::ceguitypes::USize toCEED_USize() const { return isType(VariantType::CEED_USize) ? m_CEED_USize : cegui::ceguitypes::USize(); }
    cegui::ceguitypes::UVector2 toCEED_UVector2() const { return isType(VariantType::CEED_UVector2) ? m_CEED_UVector2 : cegui::ceguitypes::UVector2(); }
    cegui::ceguitypes::URect toCEED_URect() const { return isType(VariantType::CEED_URect) ? m_CEED_URect : cegui::ceguitypes::URect(); }
    cegui::ceguitypes::UBox toCEED_UBox() const { return isType(VariantType::CEED_UBox) ? m_CEED_UBox : cegui::ceguitypes::UBox(); }
    cegui::ceguitypes::Colour toCEED_Colour() const { return isType(VariantType::CEED_Colour) ? m_CEED_Colour : cegui::ceguitypes::Colour(); }
    cegui::ceguitypes::ColourRect toCEED_ColourRect() const { return isType(VariantType::CEED_ColourRect) ? m_CEED_ColourRect : cegui::ceguitypes::ColourRect(); }
    cegui::ceguitypes::AspectMode toCEED_AspectMode() const { return isType(VariantType::CEED_AspectMode) ? m_CEED_AspectMode : cegui::ceguitypes::AspectMode(); }
    cegui::ceguitypes::HorizontalAlignment toCEED_HorizontalAlignment() const { return isType(VariantType::CEED_HorizontalAlignment) ? m_CEED_HorizontalAlignment : cegui::ceguitypes::HorizontalAlignment(); }
    cegui::ceguitypes::VerticalAlignment toCEED_VerticalAlignment() const { return isType(VariantType::CEED_VerticalAlignment) ? m_CEED_VerticalAlignment : cegui::ceguitypes::VerticalAlignment(); }
    cegui::ceguitypes::WindowUpdateMode toCEED_WindowUpdateMode() const { return isType(VariantType::CEED_WindowUpdateMode) ? m_CEED_WindowUpdateMode : cegui::ceguitypes::WindowUpdateMode(); }
    cegui::ceguitypes::Quaternion toCEED_Quaternion() const { return isType(VariantType::CEED_Quaternion) ? m_CEED_Quaternion : cegui::ceguitypes::Quaternion(); }
    cegui::ceguitypes::XYZRotation toCEED_XYZRotation() const { return isType(VariantType::CEED_XYZRotation) ? m_CEED_XYZRotation : cegui::ceguitypes::XYZRotation(); }
    cegui::ceguitypes::HorizontalFormatting toCEED_HorizontalFormatting() const { return isType(VariantType::CEED_HorizontalFormatting) ? m_CEED_HorizontalFormatting : cegui::ceguitypes::HorizontalFormatting(); }
    cegui::ceguitypes::VerticalFormatting toCEED_VerticalFormatting() const { return isType(VariantType::CEED_VerticalFormatting) ? m_CEED_VerticalFormatting : cegui::ceguitypes::VerticalFormatting(); }
    cegui::ceguitypes::HorizontalTextFormatting toCEED_HorizontalTextFormatting() const { return isType(VariantType::CEED_HorizontalTextFormatting) ? m_CEED_HorizontalTextFormatting : cegui::ceguitypes::HorizontalTextFormatting(); }
    cegui::ceguitypes::VerticalTextFormatting toCEED_VerticalTextFormatting() const { return isType(VariantType::CEED_VerticalTextFormatting) ? m_CEED_VerticalTextFormatting : cegui::ceguitypes::VerticalTextFormatting(); }
    cegui::ceguitypes::SortMode toCEED_SortMode() const { return isType(VariantType::CEED_SortMode) ? m_CEED_SortMode : cegui::ceguitypes::SortMode(); }

    propertytree::properties::Property* toCEED_Property() const { return isType(VariantType::CEED_Property) ? m_CEED_Property : nullptr; }

    // CEGUI types
    CEGUI::Colour toCEGUI_Colour() const { return isType(VariantType::CEGUI_Colour) ? m_CEGUI_Colour : CEGUI::Colour(); }
    CEGUI::ColourRect toCEGUI_ColourRect() const { return isType(VariantType::CEGUI_ColourRect) ? m_CEGUI_ColourRect : CEGUI::ColourRect(); }
    CEGUI::Font* toCEGUI_Font() const { return isType(VariantType::CEGUI_Font) ? m_CEGUI_Font : nullptr; }
    CEGUI::Image* toCEGUI_Image() const { return isType(VariantType::CEGUI_Image) ? m_CEGUI_Image : nullptr; }
    CEGUI::Quaternion toCEGUI_Quaternion() const { return isType(VariantType::CEGUI_Quaternion) ? m_CEGUI_Quaternion : CEGUI::Quaternion(); }
    CEGUI::String toCEGUI_String() const { return isType(VariantType::CEGUI_String) ? m_CEGUI_String : CEGUI::String(); }
    CEGUI::HorizontalAlignment toCEGUI_HorizontalAlignment() const { return isType(VariantType::CEGUI_HorizontalAlignment) ? m_CEGUI_HorizontalAlignment : CEGUI::HorizontalAlignment::HA_LEFT; }
    CEGUI::VerticalAlignment toCEGUI_VerticalAlignment() const { return isType(VariantType::CEGUI_VerticalAlignment) ? m_CEGUI_VerticalAlignment : CEGUI::VerticalAlignment::VA_TOP; }
    CEGUI::HorizontalFormatting toCEGUI_HorizontalFormatting() const { return isType(VariantType::CEGUI_HorizontalFormatting) ? m_CEGUI_HorizontalFormatting : CEGUI::HorizontalFormatting::HF_LEFT_ALIGNED; }
    CEGUI::VerticalFormatting toCEGUI_VerticalFormatting() const { return isType(VariantType::CEGUI_VerticalFormatting) ? m_CEGUI_VerticalFormatting : CEGUI::VerticalFormatting::VF_TOP_ALIGNED; }
    CEGUI::HorizontalTextFormatting toCEGUI_HorizontalTextFormatting() const { return isType(VariantType::CEGUI_HorizontalTextFormatting) ? m_CEGUI_HorizontalTextFormatting : CEGUI::HorizontalTextFormatting::HTF_LEFT_ALIGNED; }
    CEGUI::VerticalTextFormatting toCEGUI_VerticalTextFormatting() const { return isType(VariantType::CEGUI_VerticalTextFormatting) ? m_CEGUI_VerticalTextFormatting : CEGUI::VerticalTextFormatting::VTF_TOP_ALIGNED; }

    bool operator==(const Variant& rhs) const;

    bool operator!=(const Variant& rhs) const
    {
        return !operator ==(rhs);
    }

    Variant &toType(VariantType newType);
    QString toString() const;

protected:
    VariantType m_type;

    // Can't put these in a union, could malloc them I suppose
    // TODO: replace all this with std::variant
    QString m_QString;
    MapType m_OrderedMap; // for DictionaryProperty

    // CEED wrappers
    cegui::ceguitypes::UDim m_CEED_UDim;
    cegui::ceguitypes::USize m_CEED_USize;
    cegui::ceguitypes::UVector2 m_CEED_UVector2;
    cegui::ceguitypes::URect m_CEED_URect;
    cegui::ceguitypes::UBox m_CEED_UBox;
    cegui::ceguitypes::Colour m_CEED_Colour;
    cegui::ceguitypes::ColourRect m_CEED_ColourRect;
    cegui::ceguitypes::AspectMode m_CEED_AspectMode;
    cegui::ceguitypes::HorizontalAlignment m_CEED_HorizontalAlignment;
    cegui::ceguitypes::VerticalAlignment m_CEED_VerticalAlignment;
    cegui::ceguitypes::WindowUpdateMode m_CEED_WindowUpdateMode;
    cegui::ceguitypes::Quaternion m_CEED_Quaternion;
    cegui::ceguitypes::XYZRotation m_CEED_XYZRotation;
    cegui::ceguitypes::HorizontalFormatting m_CEED_HorizontalFormatting;
    cegui::ceguitypes::VerticalFormatting m_CEED_VerticalFormatting;
    cegui::ceguitypes::HorizontalTextFormatting m_CEED_HorizontalTextFormatting;
    cegui::ceguitypes::VerticalTextFormatting m_CEED_VerticalTextFormatting;
    cegui::ceguitypes::SortMode m_CEED_SortMode;
    cegui::ceguitypes::FontRef m_CEED_FontRef;
    cegui::ceguitypes::ImageRef m_CEED_ImageRef;

    // CEGUI types
    CEGUI::Colour m_CEGUI_Colour;
    CEGUI::ColourRect m_CEGUI_ColourRect;
    CEGUI::Quaternion m_CEGUI_Quaternion;
    CEGUI::String m_CEGUI_String;
    CEGUI::UDim m_CEGUI_UDim;
    CEGUI::USize m_CEGUI_USize;
    CEGUI::UVector2 m_CEGUI_UVector2;
    CEGUI::URect m_CEGUI_URect;
    CEGUI::UBox m_CEGUI_UBox;

    union
    {
        int m_Int;
        unsigned int m_UInt;
        float m_Float;
        bool m_Bool;
#if 0 // TODO
        AspectMode m_aspectMode;
#endif // TODO
        CEGUI::AspectMode m_CEGUI_AspectMode;
        CEGUI::HorizontalAlignment m_CEGUI_HorizontalAlignment;
        CEGUI::VerticalAlignment m_CEGUI_VerticalAlignment;
        CEGUI::HorizontalFormatting m_CEGUI_HorizontalFormatting;
        CEGUI::VerticalFormatting m_CEGUI_VerticalFormatting;
        CEGUI::HorizontalTextFormatting m_CEGUI_HorizontalTextFormatting;
        CEGUI::VerticalTextFormatting m_CEGUI_VerticalTextFormatting;
        CEGUI::ItemListBase::SortMode m_CEGUI_SortMode;
        CEGUI::WindowUpdateMode m_CEGUI_WindowUpdateMode;
        CEGUI::Font* m_CEGUI_Font;
        CEGUI::Image* m_CEGUI_Image;
        propertytree::properties::Property* m_CEED_Property;
    };
};

} // CEED

#include <QMetaType>
Q_DECLARE_METATYPE(CEED::Variant)

#endif
