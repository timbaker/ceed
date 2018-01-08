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

#include "settings_interface_types.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QtEvents>

namespace CEED {
namespace settings {
namespace interface_types {

InterfaceEntry::InterfaceEntry(declaration::Entry *entry, InterfaceSection *parent)
    : QHBoxLayout()
{
    m_entry = entry;
    m_parent = parent;
}

void InterfaceEntry::_addBasicWidgets()
{
    addWidget(widget(), 1);
    addWidget(_buildResetButton());
}

QPushButton *InterfaceEntry::_buildResetButton()
{
    QPushButton* ret = new QPushButton(m_parent);
    ret->setIcon(QIcon(":icons/settings/reset_entry_to_default.png"));
    ret->setIconSize(QSize(16, 16));
    ret->setToolTip("Reset this settings entry to the default value");
    connect(ret, &QPushButton::clicked, this, &InterfaceEntry::resetToDefaultValue);
    return ret;
}

void InterfaceEntry::discardChanges()
{
    m_entry->m_hasChanges = false;
}

void InterfaceEntry::onChange()
{
    markAsChanged();
    m_parent->onChange(this);
}

void InterfaceEntry::markAsChanged()
{
    m_entry->markAsChanged();
}

void InterfaceEntry::markAsUnchanged()
{
    m_entry->markAsUnchanged();
}

void InterfaceEntry::resetToDefaultValue()
{
}

/////

InterfaceEntryString::InterfaceEntryString(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new QLineEdit();
    m_entryWidget->setText(entry->m_value.toString());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &QLineEdit::textChanged, this, &InterfaceEntryString::onChange);
    _addBasicWidgets();
}

void InterfaceEntryString::discardChanges()
{
    m_entryWidget->setText(m_entry->m_value.toString());
    InterfaceEntry::discardChanges();
}

QWidget *InterfaceEntryString::widget()
{
    return m_entryWidget;
}

void InterfaceEntryString::onChange(const QString &text)
{
    m_entry->m_editedValue = text;
    InterfaceEntry::onChange();
}

/////

InterfaceEntryInt::InterfaceEntryInt(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new QLineEdit();
    m_entryWidget->setText(entry->m_value.toString());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &QLineEdit::textEdited, this, &InterfaceEntryInt::onChange);
    _addBasicWidgets();
}

void InterfaceEntryInt::discardChanges()
{
    m_entryWidget->setText(m_entry->m_value.toString());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryInt::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.toString());
        m_entryWidget->setText(defValue.toString());
    }
}

void InterfaceEntryInt::onChange(const QString &text)
{
    bool ok;
    int newValue = text.toInt(&ok);
    if (ok) {
        m_entry->m_editedValue = newValue;
    } else {
        m_entryWidget->setText(m_entry->m_editedValue.toString());
    }
    InterfaceEntry::onChange();
}

/////

InterfaceEntryFloat::InterfaceEntryFloat(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new QLineEdit();
    m_entryWidget->setText(entry->m_value.toString());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &QLineEdit::textEdited, this, &InterfaceEntryFloat::onChange);
    _addBasicWidgets();
}

void InterfaceEntryFloat::discardChanges()
{
    m_entryWidget->setText(m_entry->m_value.toString());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryFloat::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.toString());
        m_entryWidget->setText(defValue.toString());
    }
}

void InterfaceEntryFloat::onChange(const QString &text)
{
    bool ok;
    float newValue = text.toFloat(&ok);
    if (ok) {
        m_entry->m_editedValue = newValue;
    } else {
        m_entryWidget->setText(m_entry->m_editedValue.toString());
    }

    InterfaceEntry::onChange();
}

/////

InterfaceEntryCheckbox::InterfaceEntryCheckbox(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new QCheckBox();
    m_entryWidget->setChecked(entry->m_value.toBool());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &QCheckBox::stateChanged, this, &InterfaceEntryCheckbox::onChange);
    _addBasicWidgets();
}

void InterfaceEntryCheckbox::discardChanges()
{
    m_entryWidget->setChecked(m_entry->m_value.toBool());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryCheckbox::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.toBool());
        m_entryWidget->setChecked(defValue.toBool());
    }
}

QWidget *InterfaceEntryCheckbox::widget()
{
    return m_entryWidget;
}

void InterfaceEntryCheckbox::onChange(int state)
{
    m_entry->m_editedValue = state;
    InterfaceEntry::onChange();
}

/////

InterfaceEntryColour::InterfaceEntryColour(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new qtwidgets::ColourButton();
    m_entryWidget->setColour(entry->m_value.value<QColor>());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &qtwidgets::ColourButton::colourChanged, this, &InterfaceEntryColour::onChange);
    _addBasicWidgets();
}

void InterfaceEntryColour::discardChanges()
{
    m_entryWidget->setColour(m_entry->m_value.value<QColor>());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryColour::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.value<QColor>());
        m_entryWidget->setColour(defValue.value<QColor>());
    }
}

void InterfaceEntryColour::onChange(const QColor &colour)
{
    m_entry->m_editedValue = colour;
    InterfaceEntry::onChange();
}

/////

InterfaceEntryPen::InterfaceEntryPen(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new qtwidgets::PenButton();
    m_entryWidget->m_pen = entry->m_value.value<QPen>();
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &qtwidgets::PenButton::penChanged, this, &InterfaceEntryPen::onChange);
    _addBasicWidgets();
}

void InterfaceEntryPen::discardChanges()
{
    m_entryWidget->setPen(m_entry->m_value.value<QPen>());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryPen::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.value<QPen>());
        m_entryWidget->m_pen = defValue.value<QPen>();
    }
}

void InterfaceEntryPen::onChange(const QPen &pen)
{
    m_entry->m_editedValue = pen;
    InterfaceEntry::onChange();
}

/////

InterfaceEntryKeySequence::InterfaceEntryKeySequence(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new qtwidgets::KeySequenceButton();
    m_entryWidget->setKeySequence(entry->m_value.value<QKeySequence>());
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, &qtwidgets::KeySequenceButton::keySequenceChanged, this, &InterfaceEntryKeySequence::onChange);
    _addBasicWidgets();
}

void InterfaceEntryKeySequence::discardChanges()
{
    static_cast<qtwidgets::KeySequenceButton*>(m_entryWidget)->setKeySequence(m_entry->m_value.value<QKeySequence>());
    InterfaceEntry::discardChanges();
}

void InterfaceEntryKeySequence::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.value<QKeySequence>());
        static_cast<qtwidgets::KeySequenceButton*>(m_entryWidget)->setKeySequence(defValue.value<QKeySequence>());
    }
}

void InterfaceEntryKeySequence::onChange(const QKeySequence &keySequence)
{
    m_entry->m_editedValue = keySequence;
    InterfaceEntry::onChange();
}

/////

InterfaceEntryCombobox::InterfaceEntryCombobox(declaration::Entry *entry, InterfaceSection *parent)
    : InterfaceEntry(entry, parent)
{
    m_entryWidget = new QComboBox();
    // optionList should be a list of lists where the first item is the key (data) and the second is the label
    for (Entry::Option& option : entry->m_optionList) {
        m_entryWidget->addItem(option.label, option.key);
    }
    setCurrentIndexByValue(entry->m_value);
    m_entryWidget->setToolTip(entry->m_help);
    connect(m_entryWidget, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &InterfaceEntryCombobox::onChange);
    _addBasicWidgets();
}

void InterfaceEntryCombobox::setCurrentIndexByValue(QVariant &value)
{
    int index = m_entryWidget->findData(value);
    if (index != -1) {
        m_entryWidget->setCurrentIndex(index);
    }
}

void InterfaceEntryCombobox::discardChanges()
{
    setCurrentIndexByValue(m_entry->m_value);
    InterfaceEntry::discardChanges();
}

void InterfaceEntryCombobox::resetToDefaultValue()
{
    QVariant defValue = m_entry->m_defaultValue;
    if (m_entry->m_editedValue != defValue) {
        onChange(defValue.toInt());
        setCurrentIndexByValue(defValue);
    }
}

QWidget *InterfaceEntryCombobox::widget()
{
    return m_entryWidget;
}

void InterfaceEntryCombobox::onChange(int index)
{
    if (index != -1)
        m_entry->m_editedValue = m_entryWidget->itemData(index);
    InterfaceEntry::onChange();
}

/////

InterfaceEntry *interfaceEntryFactory(declaration::Entry *entry, InterfaceSection *parent)
{
    if (entry->m_widgetHint == "string")
        return new InterfaceEntryString(entry, parent);
    else if (entry->m_widgetHint == "int")
        return new InterfaceEntryInt(entry, parent);
    else if (entry->m_widgetHint == "float")
        return new InterfaceEntryFloat(entry, parent);
    else if (entry->m_widgetHint == "checkbox")
        return new InterfaceEntryCheckbox(entry, parent);
    else if (entry->m_widgetHint == "colour")
        return new InterfaceEntryColour(entry, parent);
    else if (entry->m_widgetHint == "pen")
        return new InterfaceEntryPen(entry, parent);
    else if (entry->m_widgetHint == "keySequence")
        return new InterfaceEntryKeySequence(entry, parent);
    else if (entry->m_widgetHint == "combobox")
        return new InterfaceEntryCombobox(entry, parent);
    else
        throw RuntimeError(QString("I don't understand widget hint '%1'").arg(entry->m_widgetHint));
}

InterfaceSection::InterfaceSection(declaration::Section *section, InterfaceCategory *parent)
    : QGroupBox()
    , m_section(section)
    , m_parent(parent)
{
    setTitle(section->m_label);

    m_layout = new QFormLayout();

    for (Entry* entry : section->m_entries) {
        QLabel* lw = new QLabel(entry->m_label);
        lw->setMinimumWidth(200);
        lw->setWordWrap(true);

        m_layout->addRow(lw, interfaceEntryFactory(entry, this));
    }

    setLayout(m_layout);
}

void InterfaceSection::discardChanges()
{
    for (InterfaceEntry* entry : m_modifiedEntries) {
        entry->discardChanges();
    }
}

void InterfaceSection::onChange(InterfaceEntry *entry)
{
    m_modifiedEntries.append(entry);
    markAsChanged();
    // FIXME: This should be rolled into the InterfaceEntry types.
    QWidget* widget = m_layout->labelForField(entry);
    QLabel* label = dynamic_cast<QLabel*>(widget);
    Q_ASSERT(label != nullptr);
    label->setText(entry->m_entry->m_label);
    m_parent->onChange(this);
}

void InterfaceSection::markAsChanged()
{
    m_section->markAsChanged();
}

void InterfaceSection::markAsUnchanged()
{
    m_section->markAsUnchanged();

    for (InterfaceEntry* entry : m_modifiedEntries) {
        entry->markAsUnchanged();
        // FIXME: This should be rolled into the InterfaceEntry types.
        QWidget* widget = m_layout->labelForField(entry);
        QLabel* label = dynamic_cast<QLabel*>(widget);
        Q_ASSERT(label != nullptr);
        label->setText(entry->m_entry->m_label);
    }
    m_modifiedEntries.clear();
}

/////

InterfaceCategory::InterfaceCategory(declaration::Category *category, QTabWidget *parent)
    : QScrollArea()
    , m_category(category)
    , m_parent(parent)
{
    m_inner = new QWidget();
    m_layout = new QVBoxLayout();

    for (Section* section : category->m_sections) {
        m_layout->addWidget(new InterfaceSection(section, this));
    }

    m_layout->addStretch();
    m_inner->setLayout(m_layout);
    setWidget(m_inner);

    setWidgetResizable(true);
}

bool InterfaceCategory::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        auto wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->delta() < 0)
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepAdd);
        else
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderSingleStepSub);
        return true;
    }

    return QScrollArea::eventFilter(watched, event);
}

void InterfaceCategory::discardChanges()
{
    for (InterfaceSection* section : m_modifiedSections) {
        section->discardChanges();
    }
}

void InterfaceCategory::onChange(InterfaceSection *section)
{
    m_modifiedSections.append(section);
    markAsChanged();
}

void InterfaceCategory::markAsChanged()
{
    m_category->markAsChanged();
    m_parent->setTabText(m_parent->indexOf(this), m_category->m_label);
}

void InterfaceCategory::markAsUnchanged()
{
    m_category->markAsUnchanged();
    m_parent->setTabText(m_parent->indexOf(this), m_category->m_label);

    for (InterfaceSection* section : m_modifiedSections) {
        section->markAsUnchanged();
    }

    m_modifiedSections.clear();
}



} // namespace interface_types
} // namespace settings
} // namespace CEED
