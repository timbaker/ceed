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

#ifndef CEED_propertytree_editors_
#define CEED_propertytree_editors_

#include "CEEDBase.h"

#include "cegui/ceguitypes.h"

/**The standard editors for the property tree.

PropertyEditorRegistry -- Maintains a list of the available editors and the types they can edit; creates the appropriate editor for a property.
PropertyEditor -- The base class for all property editors.
StringPropertyEditor -- Editor for strings.
NumericPropertyEditor -- Editor for integers and floating point numbers.
StringWrapperValidator -- Edit widget validator for the StringWrapperProperty.
EnumValuePropertyEditor -- Editor for EnumValue-based values (Combo box).
*/

#include <QColor>
#include <QValidator>

class QLineEdit;
class QPushButton;

//from ceed.cegui import ceguitypes as ct

namespace CEED {
namespace propertytree {
namespace editors {

using properties::Property;

class PropertyEditor;

class PropertyEditorRegisterInfo
{
public:
    PropertyEditorRegisterInfo();
    virtual PropertyEditor* create(Property* boundProperty, bool instantApply, bool ownsProperty) = 0;
    virtual QMap<CEED::VariantType, int> getSupportedValueTypes() = 0;
};

/*!
\brief PropertyEditorRegistry

The registry contains a (sorted) list of property editor
    types that can be used for each value type.

*/
class PropertyEditorRegistry
{
public:
    // Standard editors place themselves in this set,
    // used by registerStandardEditors().
    static QList<PropertyEditorRegisterInfo*> _standardEditors;

    typedef QPair<int, PropertyEditorRegisterInfo*> PriorityInfoPair;
    QMap<CEED::VariantType, QList<PriorityInfoPair>> m_editorsForValueType;
    QSet<PropertyEditorRegisterInfo*> m_registeredEditorTypes;

    /**Add the editor to the list of standard editors.*/
    static void addStandardEditor(PropertyEditorRegisterInfo* editorType)
    {
        _standardEditors += editorType;
    }

    /**Initialise an empty instance by default.
    If "autoRegisterStandardsEditors" is true,
    automatically register the standard editors.
    */
    PropertyEditorRegistry(bool autoRegisterStandardEditors = false)
    {
        if (autoRegisterStandardEditors)
            registerStardardEditors();
    }

    /**Register an editor with the specified priority.
    Higher values equal higher priority.

    Note: The priority is added to the editor's preferred
    priority for each value type supported. For example. if
    an editor specifies priority 10 for integers, registering
    the editor with priority = -3 will register it for
    integers with priority 7.

    Note: Registering the same editor twice does nothing
    and does not change the priority.
    */
    void registerEditor(PropertyEditorRegisterInfo *editorType, int priority=0);

    /**Register the predefined editors to this instance.*/
    void registerStardardEditors()
    {
        for (PropertyEditorRegisterInfo* editor : _standardEditors) {
            registerEditor(editor);
        }
    }

    /**Create and return an editor for the specified property,
    or None if none can be found that have been registered and
    are capable of editing the property.

    If more than one editors that can edit the property have been
    registered, the one with the highest priority will be chosen.
    */
    PropertyEditor* createEditor(Property* editProperty);
};

/*!
\brief PropertyEditor

Abstract base class for a property editor.

    A property editor instance is created when required
    to edit the value of a (supported) property.

*/
class PropertyEditor
{
public:
    QWidget* m_editWidget;
    Property* m_property;
    bool m_instantApply;
    bool m_ownsProperty;
    bool m_widgetValueInitialized;

    /**Return a dictionary containing the types supported by this
    editor and their priorities. The key is the type and the value
    is the priority.

    Example: ``{ int:0, float:-10 }``
        Uses the default priority for integers and is most likely
        the preferred editor unless another editor is registered
        with higher priority. Standard editors should always have
        the default (0) or less priority.
        It can also edit floats but it should have low priority
        for those types (maybe it truncates or rounds them).
    */
//    virtual QMap<QVariant::Type, int> getSupportedValueTypes() = 0;

    PropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false);

    virtual void finalise();

    /**Create and return a widget that will be used
    to edit the property.

    This is a good place to set up minimums and maximums,
    allowed patterns, step sizes etc. but setting the value
    is not required because a call to setWidgetValue
    will follow.
    */
    virtual QWidget* createEditWidget(QWidget* parent) = 0;

    /**Read and return a tuple with the current value of the widget
    and a boolean specifying whether it's a valid value or not.*/
    virtual CEED::Variant getWidgetValue(bool* ok) = 0;

    /**Set the value of the widget to the value of the property.

    Implementors should set m_widgetValueInitialized to true
    *after* setting the widget value.
    */
    virtual void setWidgetValueFromProperty()
    {
        m_widgetValueInitialized = true;
    }

    void setPropertyValueFromWidget();

protected slots:
    /**Callback-style method used when the 'instantApply' option
    is true to update the property's value without waiting for
    the editor to commit the edit.
    Should be called by implementors whenever the value of
    the widget changes, even if it hasn't been committed.
    */
    bool valueChanging()
    {
        if (!m_instantApply)
            return false;

        // We ignore the first event because that one
        // occurs when we initialise the edit widget's value.
        if (!m_widgetValueInitialized)
            return false;

        setPropertyValueFromWidget();
        return true;
    }
};

/*!
\brief StringPropertyEditor

Editor for strings.

    Supports line edit (the default) or combo-box mode.

    The combo-box mode is activated when the "combo/" editor options are set.
    For example, to set it via the property mappings:
        <mapping propertyOrigin="Element" propertyName="HorizontalAlignment">
            <settings name="combo">
                <setting name="Left" value="Left" />
                <setting name="Centre" value="Centre" />
                <setting name="Right" value="Right" />
            </settings>
        </mapping>

    Note that the EnumValuePropertyEditor has similar functionality but
    is based on known types and does not take the list of values from
    the editor options.

*/
class StringPropertyEditor : public PropertyEditor
{
public:
    QString m_mode;

    static QMap<CEED::VariantType, int> getSupportedValueTypes()
    {
        return { {CEED::VariantType::QString, 0} };
    }

    StringPropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
        m_mode = "line";
    }

    QWidget* createEditWidget(QWidget* parent) override;

    CEED::Variant getWidgetValue(bool* ok) override;

    void setWidgetValueFromProperty() override;
};

class NumericPropertyEditor : public PropertyEditor
{
public:
    const int DefaultDecimals = 16;
    const int DefaultMin = -999999;
    const int DefaultMax = 999999;

    NumericPropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
    }

    static QMap<CEED::VariantType, int> getSupportedValueTypes()
    {
        return { { CEED::VariantType::Int, 0 }, { CEED::VariantType::Float,  0} };
    }

    QWidget* createEditWidget(QWidget* parent) override;

    CEED::Variant getWidgetValue(bool* ok) override;

    void setWidgetValueFromProperty() override;
};


class BoolPropertyEditor : public PropertyEditor
{
public:
    BoolPropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
    }

    static QMap<CEED::VariantType, int> getSupportedValueTypes()
    {
        return { { CEED::VariantType::Bool, 0 } };
    }

    QWidget* createEditWidget(QWidget* parent) override;

    CEED::Variant getWidgetValue(bool* ok) override;

    void setWidgetValueFromProperty() override;
};



/*!
\brief StringWrapperValidator

Validate the edit widget value when editing
    a StringWrapperProperty.

    Using this prevents closing the edit widget using
    "Enter" when the value is invalid and allows the
    user the correct their mistake without losing any
    editing they have done.

*/
class StringWrapperValidator : public QValidator
{
public:
    Property* m_property;

    StringWrapperValidator(Property* swProperty, QObject* parent = nullptr)
        : QValidator(parent)
    {
        m_property = swProperty;
    }

    QValidator::State validate(QString& inputStr, int& dummyPos) const override;
};

/*!
\brief EnumValuePropertyEditor

Editor for EnumValue-based values (Combo box).
*/
class EnumValuePropertyEditor : public PropertyEditor
{
public:
    EnumValuePropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
    }

    static QMap<CEED::VariantType, int> getSupportedValueTypes();

#if 0 // TODO
    void addSubclasses(baseType)
    {
        for vt in baseType.__subclasses__():
            vts.add(vt)
            addSubclasses(vt);
    }
#endif

    QWidget* createEditWidget(QWidget* parent) override;

    CEED::Variant getWidgetValue(bool* ok) override;

    void setWidgetValueFromProperty() override;
};



/*!
\brief DynamicChoicesEditor

Editor for strings where user chooses from several options like in a combobox.

    The difference is that this combobox gets the values from an external place
    dynamically. You simply override the getChoices method of this class.

*/
class DynamicChoicesEditor : public PropertyEditor
{
public:
    DynamicChoicesEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
    }

    virtual OrderedMap<QString, QVariant> getChoices() = 0;

    QWidget* createEditWidget(QWidget* parent) override;

    CEED::Variant getWidgetValue(bool* ok) override;

    void setWidgetValueFromProperty() override;
};

class FontEditor : public DynamicChoicesEditor
{
public:
    static QMap<CEED::VariantType, int> getSupportedValueTypes();

    FontEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : DynamicChoicesEditor(boundProperty, instantApply, ownsProperty)
    {

    }

    OrderedMap<QString, QVariant> getChoices() override;
};


class ImageEditor : public DynamicChoicesEditor
{
public:
    static QMap<CEED::VariantType, int> getSupportedValueTypes();

    ImageEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : DynamicChoicesEditor(boundProperty, instantApply, ownsProperty)
    {

    }

    OrderedMap<QString, QVariant> getChoices() override;
};

/*!
\brief ColourValuePropertyEditor

Editor for CEGUI::Colour-based values (QColorDialog).
*/
class ColourValuePropertyEditor : public PropertyEditor
{
public:
    QLineEdit* m_colourEditbox;
    QPushButton* m_colourButton;
    QWidget* m_colourDialogParent;
    QColor m_selectedColor;
    QString m_lastEditSource;

    ColourValuePropertyEditor(Property* boundProperty, bool instantApply = true, bool ownsProperty = false)
        : PropertyEditor(boundProperty, instantApply, ownsProperty)
    {
        m_colourDialogParent = nullptr;
        m_colourEditbox = nullptr;
        m_colourButton = nullptr;

 //       m_selectedColor = None

        // This is used to not update the editbox after filling in a value, which would be annoying while writing
        m_lastEditSource = "";
    }

    static QMap<CEED::VariantType, int> getSupportedValueTypes();

    /**
    Creates a new Widget that contains both a LineEdit and a button. The button displays the selected colour via the button's background colour (using a stylesheet),
    the LineEdit shows the hexadecimal value as string. Both are editable. When clicking the button a ColorDialog pops up. Upon accepting a colour, the colour will be set. The
    LineEdit works analogoues: Changing its text will parse the new text and if the text can be successfully used as a CEGUI Colour then this new Colour will be set.
    :param parent: QWidget
    :return:
    */
    QWidget* createEditWidget(QWidget* parent) override;

private slots:
    /**
    Sets the focus on the editbox and marks all the text, whenever the editing is finished. This is at least necessary to have focus on the beginning of editing.
    :return:
    */
    void lineEditEditingFinished();

    void lineEditTextEdited();

    /**
    Opens a color picker dialog to select the color and calls the color-setter function
    :return:
    */
    void colourButtonReleased();

public:
    CEED::Variant getWidgetValue(bool* ok) override;

    /*
    Retrieves the property colour as QColor and calls the setter function to update the child widgets
    :return:
    */
    void setWidgetValueFromProperty() override;

    /*
        Displays the colour on the button using a stylesheet, and on the editbox using the regular setter for the string.

        :param newQColor: QColor
        :return:
        */
    void setColour(const QColor& newQColor, const QString& source="");
};

} // namespace editors
} // namespace propertytree
} // namespace CEED

Q_DECLARE_METATYPE(QValidator*)
Q_DECLARE_METATYPE(CEED::propertytree::editors::PropertyEditor*)

#endif
