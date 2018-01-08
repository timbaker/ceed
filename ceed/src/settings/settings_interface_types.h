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

#ifndef CEED_settings_interface_types_
#define CEED_settings_interface_types_

#include "CEEDBase.h"

#include "settings/settings_declaration.h"

#include "qtwidgets.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QTabWidget;

namespace CEED {
namespace settings {
namespace interface_types {

using declaration::Category;
using declaration::Entry;
using declaration::Section;

class InterfaceCategory;
class InterfaceSection;

// Implementation notes
// - The "change detection" scheme propagates upwards from the individual Entry
//   types to their parents (currently terminated at the Category/Tab level).
// - In contrast, when a user applies changes, this propagates downwards from
//   the Category/Tab level to the individual (modified) entries.
// - The reason is because the settings widgets (QLineEdit, QCheckBox, etc) are
//   used to notify the application when a change happens; and once changes are
//   applied, it is convenient to use an iterate/apply mechanism.

// Wrapper: Entry types
// - One for each 'widgetHint'.
class InterfaceEntry : public QHBoxLayout
{
    Q_OBJECT
public:
    Entry* m_entry;
    InterfaceSection* m_parent;

    InterfaceEntry(Entry* entry, InterfaceSection* parent);

    void _addBasicWidgets();

    QPushButton* _buildResetButton();

    virtual void discardChanges();

    void onChange();

    void markAsChanged();

    void markAsUnchanged();

    virtual QWidget* widget() = 0;

protected slots:
    virtual void resetToDefaultValue();
};

class InterfaceEntryString : public InterfaceEntry
{
    Q_OBJECT
public:
    QLineEdit *m_entryWidget;

    InterfaceEntryString(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    QWidget* widget() override;

private slots:
    void onChange(const QString& text);
};

class InterfaceEntryInt : public InterfaceEntry
{
    Q_OBJECT
public:
    QLineEdit* m_entryWidget;

    InterfaceEntryInt(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue();

    QWidget* widget() override
    {
        return m_entryWidget;
    }

private slots:
    void onChange(const QString& text);
};

class InterfaceEntryFloat : public InterfaceEntry
{
    Q_OBJECT
public:
    QLineEdit *m_entryWidget;

    InterfaceEntryFloat(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override
    {
        return m_entryWidget;
    }

private slots:
    void onChange(const QString& text);
};

class InterfaceEntryCheckbox : public InterfaceEntry
{
    Q_OBJECT
public:
    QCheckBox* m_entryWidget;

    InterfaceEntryCheckbox(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override;

private slots:
    void onChange(int state);
};

class InterfaceEntryColour : public InterfaceEntry
{
    Q_OBJECT
public:
    qtwidgets::ColourButton* m_entryWidget;

    InterfaceEntryColour(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override
    {
        return m_entryWidget;
    }

private slots:
    void onChange(const QColor& colour);
};

class InterfaceEntryPen : public InterfaceEntry
{
    Q_OBJECT
public:
    qtwidgets::PenButton* m_entryWidget;

    InterfaceEntryPen(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override
    {
        return m_entryWidget;
    }

private slots:
    void onChange(const QPen& pen);
};

class InterfaceEntryKeySequence : public InterfaceEntry
{
    Q_OBJECT
public:
    qtwidgets::KeySequenceButton* m_entryWidget;

    InterfaceEntryKeySequence(Entry* entry, InterfaceSection* parent);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override
    {
        return m_entryWidget;
    }

private slots:
    void onChange(const QKeySequence& keySequence);
};

class InterfaceEntryCombobox : public InterfaceEntry
{
    Q_OBJECT
public:
    QComboBox* m_entryWidget;

    InterfaceEntryCombobox(Entry* entry, InterfaceSection* parent);

    void setCurrentIndexByValue(QVariant& value);

    void discardChanges() override;

    void resetToDefaultValue() override;

    QWidget* widget() override;

private slots:
    void onChange(int index);
};

// Factory: Return appropriate InterfaceEntry
// - Not exported; restricted to use within this module.
// - Could be replaced by a static mapping.
static InterfaceEntry* interfaceEntryFactory(Entry* entry, InterfaceSection* parent);

// Wrapper: Section
class InterfaceSection : public QGroupBox
{
public:
    Section* m_section;
    InterfaceCategory* m_parent;
    QList<InterfaceEntry*> m_modifiedEntries;
    QFormLayout* m_layout;

    InterfaceSection(Section* section, InterfaceCategory* parent);

    void discardChanges();

    void onChange(InterfaceEntry* entry);

    void markAsChanged();

    void markAsUnchanged();
};

// Wrapper: Category
class InterfaceCategory : public QScrollArea
{
public:
    Category* m_category;
    QTabWidget* m_parent;
    QList<InterfaceSection*> m_modifiedSections;
    QWidget* m_inner;
    QVBoxLayout* m_layout;

    InterfaceCategory(Category* category, QTabWidget*parent);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void discardChanges();

    void onChange(InterfaceSection* section);

    void markAsChanged();

    void markAsUnchanged();
};

// Wrapper: Tabs
// TODO

} // namespace interface_types

} // namespace settings

} // namespace CEED

#endif
