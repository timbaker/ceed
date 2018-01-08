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

#include "ceguitypes.h"

#include "cegui/ceguitype_editor_properties.h"

#include "propertysetinspector.h"
#include "propertytree/propertytree_parsers.h"
#include "propertytree/propertytree_properties.h"

namespace CEED {
namespace cegui {
namespace ceguitypes {

const QString Base::floatPattern = "\\s*(-?\\d+(?:\\.\\d+)?(?:e[-+]\\d+)?)\\s*";

const QString UDim::pattern = QString("\\s*\\{%1,%1\\}\\s*").arg(Base::floatPattern);
const QRegExp UDim::rex(pattern);

const QString USize::pattern = QString("\\s*\\{%1,%1\\}\\s*").arg(UDim::pattern);
const QRegExp USize::rex(pattern);

const QString UVector2::pattern = QString("\\s*\\{%1,%1\\}\\s*").arg(UDim::pattern);
const QRegExp UVector2::rex(pattern);

const QString URect::pattern = QString("\\s*\\{%1,%1,%1,%1\\}\\s*").arg(UDim::pattern);
const QRegExp URect::rex(pattern);

const QString UBox::pattern = QString("\\s*\\{top:%1,left:%1,bottom:%1,right:%1\\}\\s*").arg(UDim::pattern);
const QRegExp UBox::rex(pattern);

const QString Quaternion::pattern = QString("\\s*w\\s*:%1\\s+x\\s*:%1\\s+y\\s*:%1\\s+z\\s*:%1").arg(Base::floatPattern);
const QRegExp Quaternion::rex(pattern, Qt::CaseInsensitive);

const QString XYZRotation::pattern = "\\s*x\\s*:%1\\s+y\\s*:%1\\s+z\\s*:%1";
const QRegExp XYZRotation::rex(pattern, Qt::CaseInsensitive);

const QString Colour::pattern("\\s*#?([0-9a-fA-F]+)\\s*");
const QRegExp Colour::rex(pattern, Qt::CaseInsensitive);

const QString ColourRect::pattern = QString("\\s*tl:%1tr:%1bl:%1br:%1").arg(Colour::pattern);
const QRegExp ColourRect::rex(pattern, Qt::CaseInsensitive);

propertysetinspector::PropertyFactoryBase *UDim::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::UDimProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *USize::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::USizeProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *UVector2::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::UVector2Property> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *URect::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::URectProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *UBox::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::UBoxProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *Quaternion::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::QuaternionProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *XYZRotation::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::XYZRotationProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *Colour::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::ColourProperty> factory;
    return &factory;
}

propertysetinspector::PropertyFactoryBase *ColourRect::getPropertyType()
{
    static propertysetinspector::PropertyFactory<ceguitype_editor_properties::ColourRectProperty> factory;
    return &factory;
}

class _BaseProperty : public ceguitype_editor_properties::BaseProperty
{
public:
    _BaseProperty(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
             const QString& category = "", const QString& helpText = "", bool readOnly = false,
             const propertytree::properties::EditorOptions& editorOptions = propertytree::properties::EditorOptions(), bool createComponents = true)
        : BaseProperty(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents)
    {
    }

    CEED::Variant getComponentValue(const QString& componentName)
    {
        Q_ASSERT(false);
        return CEED::Variant();
    }

    void setComponentValue(const QString& componentName, const CEED::Variant& newValue)
    {
        Q_ASSERT(false);
    }
};

propertysetinspector::PropertyFactoryBase *StringWrapper::getPropertyType()
{
    static propertysetinspector::PropertyFactory<_BaseProperty> factory;
    return &factory;
}

OrderedMap<QString, CEED::Variant> AspectMode::enumValues;
OrderedMap<QString, CEED::Variant> HorizontalAlignment::enumValues;
OrderedMap<QString, CEED::Variant> VerticalAlignment::enumValues;
OrderedMap<QString, CEED::Variant> HorizontalFormatting::enumValues;
OrderedMap<QString, CEED::Variant> WindowUpdateMode::enumValues;
OrderedMap<QString, CEED::Variant> VerticalFormatting::enumValues;
OrderedMap<QString, CEED::Variant> HorizontalTextFormatting::enumValues;
OrderedMap<QString, CEED::Variant> VerticalTextFormatting::enumValues;
OrderedMap<QString, CEED::Variant> SortMode::enumValues;

struct InitEnumValues {
    InitEnumValues()
    {
//#define PHSTR(A,B) TO_QSTR(CEGUI::PropertyHelper<CEGUI::A>::toString(CEGUI::A::B))
#define PHSTR(A,B) A(#B)
        AspectMode::enumValues["Ignore"] = PHSTR(AspectMode, Ignore);
        AspectMode::enumValues["Shrink"] = PHSTR(AspectMode, Shrink);
        AspectMode::enumValues["Expand"] = PHSTR(AspectMode, Expand);

        HorizontalAlignment::enumValues["Left"] = PHSTR(HorizontalAlignment, Left);
        HorizontalAlignment::enumValues["Centre"] = PHSTR(HorizontalAlignment, Centre);
        HorizontalAlignment::enumValues["Right"] = PHSTR(HorizontalAlignment, Right);

        VerticalAlignment::enumValues["Top"] = PHSTR(VerticalAlignment, Top);
        VerticalAlignment::enumValues["Centre"] = PHSTR(VerticalAlignment, Centre);
        VerticalAlignment::enumValues["Bottom"] = PHSTR(VerticalAlignment, Bottom);

        HorizontalFormatting::enumValues["Left"] = PHSTR(HorizontalFormatting, LeftAligned);
        HorizontalFormatting::enumValues["Centre"] = PHSTR(HorizontalFormatting, CentreAligned);
        HorizontalFormatting::enumValues["Right"] = PHSTR(HorizontalFormatting, RightAligned);
        HorizontalFormatting::enumValues["Stretched"] = PHSTR(HorizontalFormatting, Stretched);
        HorizontalFormatting::enumValues["Tiled"] = PHSTR(HorizontalFormatting, Tiled);

        VerticalFormatting::enumValues["Top"] = PHSTR(VerticalFormatting, TopAligned);
        VerticalFormatting::enumValues["Centre"] = PHSTR(VerticalFormatting, CentreAligned);
        VerticalFormatting::enumValues["Bottom"] = PHSTR(VerticalFormatting, BottomAligned);
        VerticalFormatting::enumValues["Stretched"] = PHSTR(VerticalFormatting, Stretched);
        VerticalFormatting::enumValues["Tiled"] = PHSTR(VerticalFormatting, Tiled);

        HorizontalTextFormatting::enumValues["Left"] = PHSTR(HorizontalTextFormatting, LeftAligned);
        HorizontalTextFormatting::enumValues["Centre"] = PHSTR(HorizontalTextFormatting, CentreAligned);
        HorizontalTextFormatting::enumValues["Right"] = PHSTR(HorizontalTextFormatting, RightAligned);
        HorizontalTextFormatting::enumValues["Justified"] = PHSTR(HorizontalTextFormatting, Justified);
        HorizontalTextFormatting::enumValues["Word-Wrap Left"] = PHSTR(HorizontalTextFormatting, WordWrapLeftAligned);
        HorizontalTextFormatting::enumValues["Word-Wrap Centre"] = PHSTR(HorizontalTextFormatting, WordWrapCentreAligned);
        HorizontalTextFormatting::enumValues["Word-Wrap Right"] = PHSTR(HorizontalTextFormatting, WordWrapRightAligned);
        HorizontalTextFormatting::enumValues["Word-Wrap Justified"] = PHSTR(HorizontalTextFormatting, WordWrapJustified);

        VerticalTextFormatting::enumValues["Top"] = PHSTR(VerticalTextFormatting, TopAligned);
        VerticalTextFormatting::enumValues["Centre"] = PHSTR(VerticalTextFormatting, CentreAligned);
        VerticalTextFormatting::enumValues["Bottom"] = PHSTR(VerticalTextFormatting, BottomAligned);

        WindowUpdateMode::enumValues["Always"] = PHSTR(WindowUpdateMode, Always);
        WindowUpdateMode::enumValues["Visible"] = PHSTR(WindowUpdateMode, Visible);
        WindowUpdateMode::enumValues["Never"] = PHSTR(WindowUpdateMode, Never);

        SortMode::enumValues["Ascending"] = PHSTR(SortMode, Ascending);
        SortMode::enumValues["Descending"] = PHSTR(SortMode, Descending);
        SortMode::enumValues["UserSort"] = PHSTR(SortMode, UserSort);
#undef PHSTR
    }
};
static InitEnumValues initEnumValues;

} // namespace ceguitypes
} // namesapce cegui

Variant::Variant(const Variant &other)
{
    m_type = other.m_type;
    switch (m_type) {
    case VariantType::NONE : break;
    case VariantType::Int : m_Int = other.m_Int; break;
    case VariantType::UInt : m_UInt = other.m_UInt; break;
    case VariantType::Float : m_Float = other.m_Float; break;
    case VariantType::Bool : m_Bool = other.m_Bool; break;
    case VariantType::QString : m_QString = other.m_QString; break;
    case VariantType::OrderedMap : m_OrderedMap = other.m_OrderedMap; break;

    case VariantType::CEED_UDim : m_CEED_UDim = other.m_CEED_UDim; break;
    case VariantType::CEED_USize : m_CEED_USize = other.m_CEED_USize; break;
    case VariantType::CEED_UVector2 : m_CEED_UVector2 = other.m_CEED_UVector2; break;
    case VariantType::CEED_URect : m_CEED_URect = other.m_CEED_URect; break;
    case VariantType::CEED_UBox : m_CEED_UBox = other.m_CEED_UBox; break;
    case VariantType::CEED_AspectMode : m_CEED_AspectMode = other.m_CEED_AspectMode; break;
    case VariantType::CEED_HorizontalAlignment : m_CEED_HorizontalAlignment = other.m_CEED_HorizontalAlignment; break;
    case VariantType::CEED_VerticalAlignment : m_CEED_VerticalAlignment = other.m_CEED_VerticalAlignment; break;
    case VariantType::CEED_WindowUpdateMode : m_CEED_WindowUpdateMode = other.m_CEED_WindowUpdateMode; break;
    case VariantType::CEED_Quaternion : m_CEED_Quaternion = other.m_CEED_Quaternion; break;
    case VariantType::CEED_XYZRotation : m_CEED_XYZRotation = other.m_CEED_XYZRotation; break;
    case VariantType::CEED_HorizontalFormatting : m_CEED_HorizontalFormatting = other.m_CEED_HorizontalFormatting; break;
    case VariantType::CEED_VerticalFormatting : m_CEED_VerticalFormatting = other.m_CEED_VerticalFormatting; break;
    case VariantType::CEED_HorizontalTextFormatting : m_CEED_HorizontalTextFormatting = other.m_CEED_HorizontalTextFormatting; break;
    case VariantType::CEED_VerticalTextFormatting : m_CEED_VerticalTextFormatting = other.m_CEED_VerticalTextFormatting; break;
    case VariantType::CEED_SortMode : m_CEED_SortMode = other.m_CEED_SortMode; break;
    case VariantType::CEED_Colour : m_CEED_Colour = other.m_CEED_Colour; break;
    case VariantType::CEED_ColourRect : m_CEED_ColourRect = other.m_CEED_ColourRect; break;
    case VariantType::CEED_FontRef : m_CEED_FontRef = other.m_CEED_FontRef; break;
    case VariantType::CEED_ImageRef : m_CEED_ImageRef = other.m_CEED_ImageRef; break;

    case VariantType::CEGUI_String : m_CEGUI_String = other.m_CEGUI_String; break;
    case VariantType::CEGUI_UDim: m_CEGUI_UDim = other.m_CEGUI_UDim; break;
    case VariantType::CEGUI_USize: m_CEGUI_USize = other.m_CEGUI_USize; break;
    case VariantType::CEGUI_UVector2: m_CEGUI_UVector2 = other.m_CEGUI_UVector2; break;
    case VariantType::CEGUI_URect: m_CEGUI_URect = other.m_CEGUI_URect; break;
    case VariantType::CEGUI_UBox: m_CEGUI_UBox = other.m_CEGUI_UBox; break;
    case VariantType::CEGUI_HorizontalAlignment: m_CEGUI_HorizontalAlignment = other.m_CEGUI_HorizontalAlignment; break;
    case VariantType::CEGUI_VerticalAlignment: m_CEGUI_VerticalAlignment = other.m_CEGUI_VerticalAlignment; break;
    case VariantType::CEGUI_WindowUpdateMode: m_CEGUI_WindowUpdateMode = other.m_CEGUI_WindowUpdateMode; break;
    case VariantType::CEGUI_Quaternion: m_CEGUI_Quaternion = other.m_CEGUI_Quaternion; break;
    case VariantType::CEGUI_HorizontalFormatting : m_CEGUI_HorizontalFormatting = other.m_CEGUI_HorizontalFormatting; break;
    case VariantType::CEGUI_VerticalFormatting : m_CEGUI_VerticalFormatting = other.m_CEGUI_VerticalFormatting; break;
    case VariantType::CEGUI_HorizontalTextFormatting : m_CEGUI_HorizontalTextFormatting = other.m_CEGUI_HorizontalTextFormatting; break;
    case VariantType::CEGUI_VerticalTextFormatting : m_CEGUI_VerticalTextFormatting = other.m_CEGUI_VerticalTextFormatting; break;
    case VariantType::CEGUI_Colour : m_CEGUI_Colour = other.m_CEGUI_Colour; break;
    case VariantType::CEGUI_ColourRect : m_CEGUI_ColourRect = other.m_CEGUI_ColourRect; break;
    case VariantType::CEGUI_Font : m_CEGUI_Font = other.m_CEGUI_Font; break;
    case VariantType::CEGUI_Image : m_CEGUI_Image = other.m_CEGUI_Image; break;
/*    case VariantType:: : m_ = other.m_; break;
    case VariantType:: : m_ = other.m_; break;
    case VariantType:: : m_ = other.m_; break;
    case VariantType:: : m_ = other.m_; break;*/
    default: Q_ASSERT(false); break;
    }
}

OrderedMap<QString, Variant> Variant::toEnumValues() const
{
    switch (m_type) {
    case VariantType::CEED_AspectMode: return m_CEED_AspectMode.getEnumValues();
    case VariantType::CEED_HorizontalAlignment: return m_CEED_HorizontalAlignment.getEnumValues();
    case VariantType::CEED_VerticalAlignment: return m_CEED_VerticalAlignment.getEnumValues();
    case VariantType::CEED_WindowUpdateMode : return m_CEED_WindowUpdateMode.getEnumValues();
    case VariantType::CEED_HorizontalFormatting: return m_CEED_HorizontalFormatting.getEnumValues();
    case VariantType::CEED_VerticalFormatting: return m_CEED_VerticalFormatting.getEnumValues();
    case VariantType::CEED_HorizontalTextFormatting: return m_CEED_HorizontalTextFormatting.getEnumValues();
    case VariantType::CEED_VerticalTextFormatting: return m_CEED_VerticalTextFormatting.getEnumValues();
    case VariantType::CEED_SortMode : return m_CEED_SortMode.getEnumValues();
    }
    Q_ASSERT(false);
    return OrderedMap<QString, Variant>();
}

bool Variant::operator==(const Variant &rhs) const
{
    if (m_type != rhs.m_type)
        return false;
    switch (m_type) {
    case VariantType::NONE: return true;
    case VariantType::Int: return m_Int == rhs.m_Int;
    case VariantType::UInt: return m_UInt == rhs.m_UInt;
    case VariantType::Float: return m_Float == rhs.m_Float;
    case VariantType::Bool: return m_Bool == rhs.m_Bool;
    case VariantType::QString: return m_QString == rhs.m_QString;
    case VariantType::OrderedMap: return m_OrderedMap == rhs.m_OrderedMap;

    case VariantType::CEED_UDim: return m_CEED_UDim == rhs.m_CEED_UDim;
    case VariantType::CEED_USize: return m_CEED_USize == rhs.m_CEED_USize;
    case VariantType::CEED_UVector2: return m_CEED_UVector2 == rhs.m_CEED_UVector2;
    case VariantType::CEED_URect: return m_CEED_URect == rhs.m_CEED_URect;
    case VariantType::CEED_UBox: return m_CEED_UBox == rhs.m_CEED_UBox;
    case VariantType::CEED_AspectMode: return m_CEED_AspectMode == rhs.m_CEED_AspectMode;
    case VariantType::CEED_HorizontalAlignment: return m_CEED_HorizontalAlignment == rhs.m_CEED_HorizontalAlignment;
    case VariantType::CEED_VerticalAlignment: return m_CEED_VerticalAlignment == rhs.m_CEED_VerticalAlignment;
    case VariantType::CEED_WindowUpdateMode: return m_CEED_WindowUpdateMode == rhs.m_CEED_WindowUpdateMode;
    case VariantType::CEED_Quaternion: return m_CEED_Quaternion == rhs.m_CEED_Quaternion;
    case VariantType::CEED_XYZRotation: return m_CEED_XYZRotation == rhs.m_CEED_XYZRotation;
    case VariantType::CEED_HorizontalFormatting: return m_CEED_HorizontalFormatting == rhs.m_CEED_HorizontalFormatting;
    case VariantType::CEED_VerticalFormatting: return m_CEED_VerticalFormatting == rhs.m_CEED_VerticalFormatting;
    case VariantType::CEED_HorizontalTextFormatting: return m_CEED_HorizontalTextFormatting == rhs.m_CEED_HorizontalTextFormatting;
    case VariantType::CEED_VerticalTextFormatting: return m_CEED_VerticalTextFormatting == rhs.m_CEED_VerticalTextFormatting;
    case VariantType::CEED_SortMode: return m_CEED_SortMode == rhs.m_CEED_SortMode;
    case VariantType::CEED_Colour: return m_CEED_Colour == rhs.m_CEED_Colour;
    case VariantType::CEED_ColourRect: return m_CEED_ColourRect == rhs.m_CEED_ColourRect;
    case VariantType::CEED_FontRef : return m_CEED_FontRef == rhs.m_CEED_FontRef;
    case VariantType::CEED_ImageRef : return m_CEED_ImageRef == rhs.m_CEED_ImageRef;

    case VariantType::CEGUI_String: return m_CEGUI_String == rhs.m_CEGUI_String;
    case VariantType::CEGUI_UDim: return m_CEGUI_UDim == rhs.m_CEGUI_UDim;
    case VariantType::CEGUI_USize: return m_CEGUI_USize == rhs.m_CEGUI_USize;
    case VariantType::CEGUI_UVector2: return m_CEGUI_UVector2 == rhs.m_CEGUI_UVector2;
    case VariantType::CEGUI_URect: return m_CEGUI_URect == rhs.m_CEGUI_URect;
    case VariantType::CEGUI_UBox: return m_CEGUI_UBox == rhs.m_CEGUI_UBox;
    case VariantType::CEGUI_HorizontalAlignment: return m_CEGUI_HorizontalAlignment == rhs.m_CEGUI_HorizontalAlignment;
    case VariantType::CEGUI_VerticalAlignment: return m_CEGUI_VerticalAlignment == rhs.m_CEGUI_VerticalAlignment;
    case VariantType::CEGUI_WindowUpdateMode: return m_CEGUI_WindowUpdateMode == rhs.m_CEGUI_WindowUpdateMode;
    case VariantType::CEGUI_Quaternion: return m_CEGUI_Quaternion == rhs.m_CEGUI_Quaternion;
    case VariantType::CEGUI_HorizontalFormatting : return m_CEGUI_HorizontalFormatting == rhs.m_CEGUI_HorizontalFormatting;
    case VariantType::CEGUI_VerticalFormatting : return m_CEGUI_VerticalFormatting == rhs.m_CEGUI_VerticalFormatting;
    case VariantType::CEGUI_HorizontalTextFormatting : return m_CEGUI_HorizontalTextFormatting == rhs.m_CEGUI_HorizontalTextFormatting;
    case VariantType::CEGUI_VerticalTextFormatting : return m_CEGUI_VerticalTextFormatting == rhs.m_CEGUI_VerticalTextFormatting;
//    case VariantType::CEGUI_ColourRect: return m_CEGUI_ColourRect == rhs.m_CEGUI_ColourRect;
    case VariantType::CEGUI_Font: return m_CEGUI_Font == rhs.m_CEGUI_Font;
    case VariantType::CEGUI_Image: return m_CEGUI_Image == rhs.m_CEGUI_Image;

    default:
        Q_ASSERT(false);
        return false;
    }
}

CEED::Variant &Variant::toType(VariantType newType)
{
    // TODO used by looknfeel editor
    // CEED type -> CEGUI type
    return *this;
}

QString Variant::toString() const
{
    // TODO
    switch (m_type) {
    case VariantType::NONE: return "";
    case VariantType::Bool: return m_Bool ? QStringLiteral("true") : QStringLiteral("false");
    case VariantType::Float: return QString::number(m_Float);
    case VariantType::Int: return QString::number(m_Int);
    case VariantType::UInt: return QString::number(m_UInt);
    case VariantType::QString: return m_QString;
    case VariantType::CEED_UDim: return m_CEED_UDim.toString(); // UDim.__repr__ from Python I think ???
    case VariantType::CEED_USize: return m_CEED_USize.toString();
    case VariantType::CEED_UVector2: return m_CEED_UVector2.toString();
    case VariantType::CEED_URect: return m_CEED_URect.toString();
    case VariantType::CEED_UBox: return m_CEED_UBox.toString();
    case VariantType::CEED_AspectMode: return m_CEED_AspectMode.toString();
    case VariantType::CEED_HorizontalAlignment: return m_CEED_HorizontalAlignment.toString();
    case VariantType::CEED_VerticalAlignment: return m_CEED_VerticalAlignment.toString();
    case VariantType::CEED_WindowUpdateMode: return m_CEED_WindowUpdateMode.toString();
    case VariantType::CEED_Quaternion: return m_CEED_Quaternion.toString();
    case VariantType::CEED_XYZRotation: return m_CEED_XYZRotation.toString();
    case VariantType::CEED_HorizontalFormatting: return m_CEED_HorizontalFormatting.toString();
    case VariantType::CEED_VerticalFormatting: return m_CEED_VerticalFormatting.toString();
    case VariantType::CEED_HorizontalTextFormatting: return m_CEED_HorizontalTextFormatting.toString();
    case VariantType::CEED_VerticalTextFormatting: return m_CEED_VerticalTextFormatting.toString();
    case VariantType::CEED_SortMode: return m_CEED_SortMode.toString();
    case VariantType::CEED_Colour: return m_CEED_Colour.toString();
    case VariantType::CEED_ColourRect: return m_CEED_ColourRect.toString();
    case VariantType::CEED_FontRef :return m_CEED_FontRef.toString();
    case VariantType::CEED_ImageRef :return m_CEED_ImageRef.toString();

    case VariantType::CEGUI_String: return TO_QSTR(m_CEGUI_String);
    case VariantType::CEGUI_UDim: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::UDim>::toString(m_CEGUI_UDim));
    case VariantType::CEGUI_USize: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::USize>::toString(m_CEGUI_USize));
    case VariantType::CEGUI_UVector2: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::UVector2>::toString(m_CEGUI_UVector2));
    case VariantType::CEGUI_URect: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::URect>::toString(m_CEGUI_URect));
    case VariantType::CEGUI_UBox: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::UBox>::toString(m_CEGUI_UBox));
    case VariantType::CEGUI_HorizontalAlignment : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::HorizontalAlignment>::toString(m_CEGUI_HorizontalAlignment));
    case VariantType::CEGUI_VerticalAlignment : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::VerticalAlignment>::toString(m_CEGUI_VerticalAlignment));
    case VariantType::CEGUI_WindowUpdateMode: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::WindowUpdateMode>::toString(m_CEGUI_WindowUpdateMode));
    case VariantType::CEGUI_Quaternion: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::Quaternion>::toString(m_CEGUI_Quaternion));
    case VariantType::CEGUI_HorizontalFormatting : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::HorizontalFormatting>::toString(m_CEGUI_HorizontalFormatting));
    case VariantType::CEGUI_VerticalFormatting : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::VerticalFormatting>::toString(m_CEGUI_VerticalFormatting));
    case VariantType::CEGUI_HorizontalTextFormatting : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::HorizontalTextFormatting>::toString(m_CEGUI_HorizontalTextFormatting));
    case VariantType::CEGUI_VerticalTextFormatting : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::VerticalTextFormatting>::toString(m_CEGUI_VerticalTextFormatting));
    case VariantType::CEGUI_Colour : return TO_QSTR(CEGUI::PropertyHelper<CEGUI::Colour>::toString(m_CEGUI_Colour));
    case VariantType::CEGUI_ColourRect: return TO_QSTR(CEGUI::PropertyHelper<CEGUI::ColourRect>::toString(m_CEGUI_ColourRect));
    case VariantType::CEGUI_Font: return m_CEGUI_Font ? TO_QSTR(m_CEGUI_Font->getName()) : "";
    case VariantType::CEGUI_Image: return m_CEGUI_Image ? TO_QSTR(m_CEGUI_Image->getName()) : "";
    default:
        Q_ASSERT(false);
        return "TODO";
        break;
    }
}

// namespace cegui
} // namespace CEED
