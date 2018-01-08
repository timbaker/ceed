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

#include "propertytree_editors.h"

#include "propertytree/propertytree_properties.h"
#include "propertytree/propertytree_utility.h"

#include "cegui/cegui_init.h"
#include "mainwindow.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

namespace CEED {
namespace propertytree {
namespace editors {

QList<PropertyEditorRegisterInfo*> PropertyEditorRegistry::_standardEditors;

void PropertyEditorRegistry::registerEditor(PropertyEditorRegisterInfo *editorType, int priority)
{
    // If the editor hasn't been registered here already
    if (!m_registeredEditorTypes.contains(editorType)) {
        m_registeredEditorTypes.insert(editorType);
        // Get supported types and priorities
        QMap<CEED::VariantType, int> supportedTypes = editorType->getSupportedValueTypes();
        for (auto iter = supportedTypes.begin(); iter != supportedTypes.end(); iter++) {
            CEED::VariantType valueType = iter.key();
            int valuePriority = iter.value();
            QPair<int, PropertyEditorRegisterInfo*> tup(priority + valuePriority, editorType);
            // If we don't know this value type at all,
            // create a new list for this value type having
            // only one element with this editor.
            if (m_editorsForValueType.contains(valueType)) {
                m_editorsForValueType[valueType] = { tup };
            }
            // If we already know this value type,
            // append the new editor and sort again so make
            // those of higher priority be first.
            else {
                auto& list = m_editorsForValueType[valueType];
                list += tup;
                std::sort(list.begin(), list.end(),
                          [](const PropertyEditorRegistry::PriorityInfoPair& a, const PropertyEditorRegistry::PriorityInfoPair& b)
                {
                    return a.first > b.first; // reverse=true
                });
            }
        }
    }
}

PropertyEditor *PropertyEditorRegistry::createEditor(properties::Property *editProperty)
{
    CEED::VariantType valueType = editProperty->valueType();
    if (m_editorsForValueType.contains(valueType)) {
        return m_editorsForValueType[valueType][0].second->create(editProperty, true, false);
    }
    return nullptr;
}

/////

PropertyEditor::PropertyEditor(properties::Property *boundProperty, bool instantApply, bool ownsProperty)
{
    /**Initialise an instance and keeps a reference
        to the property that will be edited.
        */
    m_editWidget = nullptr;
    m_property = boundProperty;
    m_instantApply = m_property->getEditorOption("instantApply", instantApply).toBool();
    m_ownsProperty = ownsProperty;

    // see valueChanging()
    m_widgetValueInitialized = false;
}

void PropertyEditor::finalise()
{
    if (m_ownsProperty)
        m_property->finalise();
}

void PropertyEditor::setPropertyValueFromWidget()
{
    /**Set the value of the property to value of the widget.*/
    bool ok;
    CEED::Variant value = getWidgetValue(&ok);
    if (ok && (value != m_property->m_value)) {
        m_property->setValue(value, properties::ChangeValueReason::Editor);
    }
}

/////

QWidget *StringPropertyEditor::createEditWidget(QWidget *parent)
{
    properties::EditorOptions options = m_property->getEditorOption("string/").toMap();
    properties::EditorOptions comboOptions = m_property->getEditorOption("combo/").toMap();

    if (comboOptions.isEmpty()) {
        m_mode = "line";
        QLineEdit* widget  = new QLineEdit(parent);
        m_editWidget = widget;
        QObject::connect(widget, &QLineEdit::textEdited, [=](const QString&){ valueChanging(); });

        // setup options
        widget->setMaxLength(options.value("maxLength", widget->maxLength()).toInt());
        widget->setPlaceholderText(options.value("placeholderText", widget->placeholderText()).toString());
        widget->setInputMask(options.value("inputMask", widget->inputMask()).toString());
        widget->setValidator(options.value("validator", widget->validator()).value<QValidator*>()); // special handling, see PropertyTreeItemDelegate
    } else {
        m_mode = "combo";
        QComboBox* widget = new QComboBox(parent);
        m_editWidget = widget;
        for (auto it = comboOptions.begin(); it != comboOptions.end(); it++) {
            widget->addItem(it.key(), it.value());
        }
        QObject::connect(widget, (void (QComboBox::*)(int))&QComboBox::activated, [this]() { this->valueChanging(); });
    }

    return m_editWidget;
}

CEED::Variant StringPropertyEditor::getWidgetValue(bool *ok)
{
    if (m_mode == "combo") {
        QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
        int idx = widget->currentIndex();
        if (idx != -1) {
            *ok = true;
            return widget->itemData(idx).value<CEED::Variant>();
        }
    } else {
        QLineEdit* widget = static_cast<QLineEdit*>(m_editWidget);
        if (widget->hasAcceptableInput()) {
            *ok = true;
            return widget->text();
        }
    }
    *ok = false;
    return CEED::Variant();
}

void StringPropertyEditor::setWidgetValueFromProperty()
{
    bool valid;
    CEED::Variant value = getWidgetValue(&valid);
    if (!valid || (m_property->m_value != value)) {
        if (m_mode == "combo") {
            QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
            int idx = widget->findData(QVariant::fromValue(m_property->m_value));
            widget->setCurrentIndex(idx);
        } else {
            QLineEdit* widget = static_cast<QLineEdit*>(m_editWidget);
            widget->setText(m_property->m_value.toString());
        }
    }

    PropertyEditor::setWidgetValueFromProperty();
}

/////

QWidget *NumericPropertyEditor::createEditWidget(QWidget *parent)
{
    QDoubleSpinBox* widget = new QDoubleSpinBox(parent); // FIXME? property could be 'int' not 'double'
    m_editWidget = widget;
    QObject::connect(widget, (void (QDoubleSpinBox::*)(double))&QDoubleSpinBox::valueChanged, [this]() { this->valueChanging(); });

    // setup options
    properties::EditorOptions options = m_property->getEditorOption("numeric/").toMap();

    // set decimals first because according to the docs setting it can change the range.
    widget->setDecimals(options.value("decimals", (m_property->valueType() == VariantType::Int) ? 0 : DefaultDecimals).toInt());
    widget->setRange(options.value("min", DefaultMin).toInt(), (options.value("max", DefaultMax).toInt()));
    // make the default step 0.1 if the value range is 1 or less, otherwise 1
    widget->setSingleStep(options.value("step", abs(widget->maximum() - widget->minimum()) <= 1.0 ? 0.1 : 1).toDouble());
    widget->setWrapping(utility::boolFromString(options.value("wrapping", "false").toString()));

    QString buttons = options.value("buttons").toString();
    if (!buttons.isEmpty()) {
        if (buttons == "UpDownArrows")
            widget->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        else if (buttons == "PlusMinus")
            widget->setButtonSymbols(QAbstractSpinBox::PlusMinus);
        else if (buttons == "NoButtons")
            widget->setButtonSymbols(QAbstractSpinBox::NoButtons);
        else
            Q_ASSERT(false);
    }
//    widget->setButtonSymbols(QAbstractSpinBox::ButtonSymbols(options.value("buttons", widget->buttonSymbols())));

    return m_editWidget;
}

CEED::Variant NumericPropertyEditor::getWidgetValue(bool *ok)
{
    // the call to the constructor of the type is done so we return
    // an integer and not a float if the property's type is an integer.
    // it should be future proof too.
    CEED::VariantType type = m_property->valueType();

    *ok = true;
    QDoubleSpinBox* widget = static_cast<QDoubleSpinBox*>(m_editWidget);
    if (type == VariantType::Float) return (float)widget->value();
    if (type == VariantType::Int) return (int)widget->value();
    if (type == VariantType::UInt) return (uint)widget->value();
    Q_ASSERT(false);
    return (float)widget->value();
}

void NumericPropertyEditor::setWidgetValueFromProperty()
{
    bool valid;
    CEED::Variant value = getWidgetValue(&valid);
    if (!valid || (m_property->m_value != value)) {
        QDoubleSpinBox* widget = static_cast<QDoubleSpinBox*>(m_editWidget);
        CEED::VariantType type = m_property->valueType();
        if (type == VariantType::Float)
            widget->setValue(m_property->m_value.toFloat());
        else if (type == VariantType::Int)
            widget->setValue(m_property->m_value.toInt());
        else if (type == VariantType::UInt)
            widget->setValue(m_property->m_value.toUInt());
        else {
            Q_ASSERT(false);
            widget->setValue(m_property->m_value.toFloat());
        }
    }

    PropertyEditor::setWidgetValueFromProperty();
}

/////

QWidget *BoolPropertyEditor::createEditWidget(QWidget *parent)
{
    QCheckBox *widget = new QCheckBox(parent);
    m_editWidget = widget;
    widget->setAutoFillBackground(true);
    QObject::connect(widget, &QCheckBox::stateChanged, [this]() { this->valueChanging(); });

    return m_editWidget;
}

CEED::Variant BoolPropertyEditor::getWidgetValue(bool *ok)
{
    *ok = true;
    return static_cast<QCheckBox*>(m_editWidget)->isChecked();
}

void BoolPropertyEditor::setWidgetValueFromProperty()
{
    bool valid;
    CEED::Variant value = getWidgetValue(&valid);
    if (!valid || (m_property->m_value != value)) {
        static_cast<QCheckBox*>(m_editWidget)->setChecked(m_property->m_value.toBool());
    }

    PropertyEditor::setWidgetValueFromProperty();
}

/////

QValidator::State StringWrapperValidator::validate(QString &inputStr, int &dummyPos) const
{
    Q_UNUSED(dummyPos)
    CEED::Variant value = m_property->tryParse(inputStr);
    return value.isValid() ? QValidator::Acceptable : QValidator::Intermediate;
}

/////

QMap<VariantType, int> EnumValuePropertyEditor::getSupportedValueTypes()
{
    // Support all types that are subclasses of EnumValue.
#if 1
    return {
        { CEED::VariantType::CEED_AspectMode, -10 },
        { CEED::VariantType::CEED_HorizontalAlignment, -10 },
        { CEED::VariantType::CEED_VerticalAlignment, -10 },
        { CEED::VariantType::CEED_WindowUpdateMode, -10 },
        { CEED::VariantType::CEED_HorizontalFormatting, -10 },
        { CEED::VariantType::CEED_VerticalFormatting, -10 },
        { CEED::VariantType::CEED_HorizontalTextFormatting, -10 },
        { CEED::VariantType::CEED_VerticalTextFormatting, -10 },
        { CEED::VariantType::CEED_SortMode, -10 },
    };
#else
    vts = set();

    addSubclasses(EnumValue)

            return dict((vt, -10) for vt in vts);
#endif
}

QWidget *EnumValuePropertyEditor::createEditWidget(QWidget *parent)
{
    QComboBox* widget = new QComboBox(parent);
    m_editWidget = widget;
    auto enumValues = m_property->m_value.toEnumValues();
    for (auto it = enumValues.begin(); it != enumValues.end(); it++) {
        widget->addItem(it.key(), QVariant::fromValue(it.value()));
    }
    QObject::connect(widget, (void (QComboBox::*)(int))&QComboBox::activated, [this]() { this->valueChanging(); });

    return m_editWidget;
}

CEED::Variant EnumValuePropertyEditor::getWidgetValue(bool *ok)
{
    QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
    int idx = widget->currentIndex();
    if (idx == -1) {
        *ok = false;
        return CEED::Variant();
    }
    *ok = true;
    return widget->itemData(idx).value<CEED::Variant>();
}

void EnumValuePropertyEditor::setWidgetValueFromProperty()
{
    bool valid;
    CEED::Variant value = getWidgetValue(&valid);
    if (!valid || (m_property->m_value != value)) {
        QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
#if 1
        for (int idx = 0; idx < widget->count(); idx++) {
            if (widget->itemData(idx).value<CEED::Variant>() == m_property->m_value) {
                widget->setCurrentIndex(idx);
                break;
            }
        }
#else
        int idx = widget->findData(QVariant::fromValue(m_property->m_value));
        if (idx != -1)
            widget->setCurrentIndex(idx);
#endif
    }

    PropertyEditor::setWidgetValueFromProperty();
}

////

QWidget *DynamicChoicesEditor::createEditWidget(QWidget *parent)
{
    QComboBox* widget = new QComboBox(parent);
    m_editWidget = widget;
    auto choices = getChoices();
    for (auto it = choices.begin(); it != choices.end(); it++) {
        widget->addItem(it.key(), it.value());
    }
    QObject::connect(widget, (void (QComboBox::*)(int))&QComboBox::activated, [this]() { this->valueChanging(); });

    return m_editWidget;
}

CEED::Variant DynamicChoicesEditor::getWidgetValue(bool *ok)
{
    QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
    int idx = widget->currentIndex();
    if (idx == -1) {
        *ok = false;
        return CEED::Variant();
    }
    *ok = true;
    return widget->itemData(idx).value<CEED::Variant>();
}

void DynamicChoicesEditor::setWidgetValueFromProperty()
{
    bool valid;
    CEED::Variant value = getWidgetValue(&valid);
    if (!valid || (m_property->m_value != value)) {
        QComboBox* widget = static_cast<QComboBox*>(m_editWidget);
        for (int idx = 0; idx < widget->count(); idx++) {
            if (widget->itemData(idx).value<CEED::Variant>() == m_property->m_value) {
                widget->setCurrentIndex(idx);
                break;
            }
        }
    }

    PropertyEditor::setWidgetValueFromProperty();
}

/////

QMap<CEED::VariantType, int> FontEditor::getSupportedValueTypes()
{
    return { {CEED::VariantType::CEED_FontRef,  0} };
}

OrderedMap<QString, QVariant> FontEditor::getChoices()
{
    auto ceguiInstance = mainwindow::MainWindow::instance->ceguiInstance;

    OrderedMap<QString, QVariant> ret;
#if 1 // TODO
    ret[""] = QVariant::fromValue(CEED::Variant(cegui::ceguitypes::FontRef(""))); // GUI Context default font
    if (ceguiInstance != nullptr) {
        for (auto font : ceguiInstance->getAvailableFonts()) {
            ret[font] = QVariant::fromValue(CEED::Variant(cegui::ceguitypes::FontRef(font)));
        }
    }
#else
    OrderedMap<QString, QVariant> ret = { { "", ct.FontRef("") } };  // GUI Context default font

    if (ceguiInstance != nullptr) {
        for (auto font : ceguiInstance->getAvailableFonts()) {
            ret[font] = ct.FontRef(font);
        }
    }
#endif

    return ret;
}

/////

QMap<CEED::VariantType, int> ImageEditor::getSupportedValueTypes()
{
    return { {CEED::VariantType::CEED_ImageRef, 0} };
}

OrderedMap<QString, QVariant> ImageEditor::getChoices()
{
    auto ceguiInstance = mainwindow::MainWindow::instance->ceguiInstance;

    OrderedMap<QString, QVariant> ret;
#if 1 // TODO
    ret[""] = QVariant::fromValue(CEED::Variant(cegui::ceguitypes::ImageRef("")));
    if (ceguiInstance != nullptr) {
        for (auto image : ceguiInstance->getAvailableImages()) {
            ret[image] = QVariant::fromValue(CEED::Variant(cegui::ceguitypes::ImageRef(image)));
        }
    }
#else
    ret = { { "", ct.ImageRef("") } };

    if (ceguiInstance != nullptr) {
        for (auto image : ceguiInstance->getAvailableImages()) {
            ret[image] = ct.ImageRef(image);
        }
    }
#endif
    return ret;
}

/////

QMap<CEED::VariantType, int> ColourValuePropertyEditor::getSupportedValueTypes()
{
    return { {CEED::VariantType::CEED_Colour, 0} };
}

QWidget *ColourValuePropertyEditor::createEditWidget(QWidget *parent)
{
    m_editWidget = new QWidget(parent);
    m_editWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_editWidget->setContentsMargins(QMargins(0, 0, 0, 0));

    QHBoxLayout* hBoxLayout = new QHBoxLayout(m_editWidget);
    hBoxLayout->setContentsMargins(QMargins(0, 0, 0, 0));
    hBoxLayout->setSpacing(0);

    m_colourEditbox = new QLineEdit();
    m_colourEditbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    properties::EditorOptions options = m_property->getEditorOption("string/").toMap();
    // setup options
    m_colourEditbox->setMaxLength(options.value("maxLength", m_colourEditbox->maxLength()).toInt());
    m_colourEditbox->setPlaceholderText(options.value("placeholderText", m_colourEditbox->placeholderText()).toString());
    m_colourEditbox->setInputMask(options.value("inputMask", m_colourEditbox->inputMask()).toString());
    m_colourEditbox->setValidator(options.value("validator", m_colourEditbox->validator()).value<QValidator*>());
    hBoxLayout->addWidget(m_colourEditbox, 0);

    m_colourButton = new QPushButton();
    m_colourButton->setMinimumSize(QSize(23, 23));
    m_colourButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    hBoxLayout->addWidget(m_colourButton, 1);
    m_colourDialogParent = parent;

    QObject::connect(m_colourButton, &QPushButton::clicked, [=](){ colourButtonReleased(); });
    QObject::connect(m_colourEditbox, &QLineEdit::textEdited, [=](){ lineEditTextEdited(); });
    QObject::connect(m_colourEditbox, &QLineEdit::editingFinished, [=](){ lineEditEditingFinished(); });

    return m_editWidget;
}

void ColourValuePropertyEditor::lineEditEditingFinished()
{
    m_colourEditbox->setFocus();
    m_colourEditbox->selectAll();
}

void ColourValuePropertyEditor::lineEditTextEdited()
{
    try {
        QColor newQColor = cegui::ceguitypes::Colour::fromString(m_colourEditbox->text()).toQColor();
        if (newQColor.isValid())
            setColour(newQColor, "editbox");
    } catch (...) {
    }
}

void ColourValuePropertyEditor::colourButtonReleased()
{
    QColor colour = QColorDialog::getColor(m_selectedColor, m_colourDialogParent, "",
                                           QColorDialog::ColorDialogOption::ShowAlphaChannel |
                                           QColorDialog::ColorDialogOption::DontUseNativeDialog);

    if (colour.isValid()) {
        setColour(colour);
    }
}

CEED::Variant ColourValuePropertyEditor::getWidgetValue(bool *ok)
{
    if (m_selectedColor.isValid()) {
        *ok = true;
        return cegui::ceguitypes::Colour::fromQColor(m_selectedColor);
    }
    *ok = false;
    return CEED::Variant();
}

void ColourValuePropertyEditor::setWidgetValueFromProperty()
{
    QColor initialColour = m_property->m_value.toCEED_Colour().toQColor();
    if (initialColour.isValid()) {
        if (initialColour != m_selectedColor)
            setColour(initialColour);
    }

    PropertyEditor::setWidgetValueFromProperty();
}

void ColourValuePropertyEditor::setColour(const QColor &newQColor, const QString &source)
{
    QString buttonStyleSheet = R"(
                               QPushButton
                               {{
                               margin: 1px;
                               border-color: {borderColour};
                               border-style: outset;
                               border-radius: 3px;
                               border-width: 1px;
                               background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 {normalFirstColour}, stop: 0.7 {normalFirstColour}, stop: 1 {normalSecondColour});
                               }}

                               QPushButton:pressed
                               {{
                               background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 {pushedFirstColour}, stop: 0.7 {pushedFirstColour}, stop: 1 {pushedSecondColour});
                               }}
                               )";

    QString firstColourStr = newQColor.toRgb().name();
    QColor borderColour;
    int borderLightness;
    if (newQColor.lightness() >= 123)
        borderLightness = 0;
    else
        borderLightness = 255;
    borderColour.setHsl(-1, 0, borderLightness);
    QString borderColourStr = borderColour.toRgb().name();
    QString secondColourStr = newQColor.darker(150).toRgb().name();
    QString thirdColourStr = newQColor.darker(200).toRgb().name();
#if 1
    buttonStyleSheet.replace("{borderColour}", borderColourStr)
            .replace("{normalFirstColour}", firstColourStr)
            .replace("{normalSecondColour}", secondColourStr)
            .replace("{pushedFirstColour}", secondColourStr)
            .replace("{pushedSecondColour}", thirdColourStr);
#else
    buttonStyleSheet = buttonStyleSheet.format(borderColour=borderColourStr,
                                               normalFirstColour=firstColourStr, normalSecondColour=secondColourStr,
                                               pushedFirstColour=secondColourStr, pushedSecondColour=thirdColourStr);
#endif
    m_colourButton->setStyleSheet(buttonStyleSheet);

    if (source != "editbox") {
        QString colourAsAlphaRGBString = cegui::ceguitypes::Colour::fromQColor(newQColor).toString();
        m_colourEditbox->setText(colourAsAlphaRGBString);

        m_colourEditbox->setFocus();
        m_colourEditbox->selectAll();
    }

    m_selectedColor = newQColor;
    PropertyEditor::valueChanging();
}

#define PROPERTY_EDITOR_STATIC_REGISTER(T) \
class PropertyEditorRegisterInfo_##T : public PropertyEditorRegisterInfo \
{ \
public: \
    PropertyEditor* create(Property* boundProperty, bool instantApply, bool ownsProperty) override \
    { \
        return new T(boundProperty, instantApply, ownsProperty); \
    } \
    QMap<CEED::VariantType, int> getSupportedValueTypes() \
    { \
        return T::getSupportedValueTypes(); \
    } \
}; \
    static PropertyEditorRegisterInfo_##T PERI_##T;

PROPERTY_EDITOR_STATIC_REGISTER(StringPropertyEditor)
PROPERTY_EDITOR_STATIC_REGISTER(NumericPropertyEditor)
PROPERTY_EDITOR_STATIC_REGISTER(BoolPropertyEditor)
PROPERTY_EDITOR_STATIC_REGISTER(EnumValuePropertyEditor)
PROPERTY_EDITOR_STATIC_REGISTER(FontEditor)
PROPERTY_EDITOR_STATIC_REGISTER(ImageEditor)
PROPERTY_EDITOR_STATIC_REGISTER(ColourValuePropertyEditor)

PropertyEditorRegisterInfo::PropertyEditorRegisterInfo()
{
    PropertyEditorRegistry::addStandardEditor(this);
}

} // namespace editors
} // namespace propertytree
} // namespace CEED
