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

#ifndef CEED_settings_declaration_
#define CEED_settings_declaration_

#include "CEEDBase.h"

#include <QString>
#include <QVariant>

#include <functional>

// From LumixEngine
#include "delegate_list.h"

namespace CEED {
namespace settings {

namespace persistence {
class PersistenceProvider;
}

namespace declaration {

using persistence::PersistenceProvider;

class Category;
class Section;
class Settings;

static const QString STRING = QStringLiteral("string");

/*!
\brief Entry

    Is the value itself, inside a section. This is what's directly used when
    accessing settings.

    value represents the current value to use
    editedValue represents the value user directly edits
    (it is applied - value = editedValue - when user applies the settings)

*/
class Entry
{
public:
    // For combobox properties
    struct Option
    {
        QVariant key;
        QString label;
    };

    Section* m_section;
    QString m_name;
    QVariant::Type m_type;
    QVariant m_defaultValue;
    QString m_label;
    QString m_help;
    QVariant m_value;
    QVariant m_editedValue;
    bool m_hasChanges;
    QString m_widgetHint;
    bool m_changeRequiresRestart;
    int m_sortingWeight;
    QList<Option> m_optionList;

#if 0
    value = property(fset = lambda entry, value: entry._setValue(value),
                     fget = lambda entry: entry._value)
    editedValue = property(fset = lambda entry, value: entry._setEditedValue(value),
                           fget = lambda entry: entry._editedValue)
#endif
    Entry(Section* section, const QString& name, QVariant::Type type, const QVariant& defaultValue,
              const QString& label, const QString& help, const QString& widgetHint = STRING,
              int sortingWeight = 0, bool changeRequiresRestart = false,
              const QList<Option>& optionList = QList<Option>());

    /** Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example */
    QString getPath();

    PersistenceProvider* getPersistenceProvider();

    Settings& getSettings();

    QVariant sanitizeValue(const QVariant& value)
    {
        if (value.canConvert(m_type)) {
            QVariant converted = value;
            converted.convert(m_type); // may have failed
            return converted;
        }
        return QVariant();
    }

    struct Subscription
    {
        std::function<void(const QVariant&)> cb;
        int id;
    };

    QList<Subscription> m_subscribers;
    int m_nextID = 0;

    /** Subscribes a callable that gets called when the value changes (the real value, not edited value!)
    (with current value as the argument) */
    int subscribe(std::function<void(const QVariant&)> callable)
    {
        m_subscribers += { callable, m_nextID++ };
        return m_nextID - 1;
    }

    void unsubscribe(int id)
    {
        std::remove_if(m_subscribers.begin(), m_subscribers.end(),
                       [=](const Subscription& a) {
            return a.id == id;
        });
    }

    void _setValue(const QVariant& value, bool uploadImmediately = true);

    void _setEditedValue(const QVariant& value)
    {
        m_editedValue = sanitizeValue(value);
        m_hasChanges = true;
    }

    void markAsChanged()
    {
        if (!m_label.startsWith("* ")) {
            m_label.prepend(QStringLiteral("* "));
        }
    }

    void markAsUnchanged()
    {
        if (m_label.startsWith("* ")) {
            m_label.remove(0, 2);
        }
    }

    void applyChanges();

    void discardChanges();

    void upload();

    void download();
};

/*!
\brief Section

Groups entries, is usually represented by a group box in the interface

*/
class Section
{
public:
    Category* m_category;
    QString m_name;
    QString m_label;
    int m_sortingWeight;
    QList<Entry*> m_entries;

    Section(Category* category, const QString& name, const QString& label, int sortingWeight = 0)
        : m_category(category)
        , m_name(name)
        , m_label(label.isEmpty() ? name : label)
        , m_sortingWeight(sortingWeight)
    {
    }

    /** Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example */
    QString getPath();

    PersistenceProvider* getPersistenceProvider();

    Settings& getSettings();

    Entry* createEntry(const QString& name, QVariant::Type type, const QVariant& defaultValue,
                       const QString& label, const QString& help, const QString& widgetHint = STRING,
                       int sortingWeight = 0, bool changeRequiresRestart = false,
                       const QList<Entry::Option>& optionList = QList<Entry::Option>())
    {
        Entry* entry = new Entry(this, name, type, defaultValue, label, help, widgetHint,
                                 sortingWeight, changeRequiresRestart, optionList);
        m_entries += entry;

        return entry;
    }

    Entry* getEntry(const QString& name)
    {
        foreach (Entry* entry, m_entries) {
            if (entry->m_name == name) {
                return entry;
            }
        }

        throw RuntimeError("Entry of name '" + name + "' not found inside section '" + m_name + "' (path: '" + getPath() + "').");
    }

    // - Reserved for possible future use.
    void markAsChanged()
    {
    }

    void markAsUnchanged()
    {
    }

    void applyChanges()
    {
        foreach (Entry* entry, m_entries) {
            entry->applyChanges();
        }
    }

    void discardChanges()
    {
        foreach (Entry* entry, m_entries) {
            entry->discardChanges();
        }
    }

    void upload()
    {
        foreach (Entry* entry, m_entries) {
            entry->upload();
        }
    }

    void download()
    {
        foreach (Entry* entry, m_entries) {
            entry->download();
        }
    }

    void sort()
    {
        std::stable_sort(m_entries.begin(), m_entries.end(), [](Entry* a, Entry* b) {
            return a->m_name < b->m_name;
        });

        std::stable_sort(m_entries.begin(), m_entries.end(), [](Entry* a, Entry* b) {
            return a->m_sortingWeight < b->m_sortingWeight;
        });
    }
};

/*!
\brief Category

Groups sections, is usually represented by a tab in the interface

*/
class Category
{
public:
    Settings* m_settings;
    QString m_name;
    QString m_label;
    int m_sortingWeight;
    QList<Section*> m_sections;

    Category(Settings* settings, const QString& name, const QString& label, int sortingWeight = 0)
        : m_settings(settings)
        , m_name(name)
        , m_label(label.isEmpty() ? name : label)
        , m_sortingWeight(sortingWeight)
    {
    }

    /** Retrieves a unique path in the qsettings tree, this can be used by persistence providers for example */
    QString getPath();

    PersistenceProvider* getPersistenceProvider();

    Settings& getSettings()
    {
        return *m_settings;
    }

    Section* createSection(const QString& name, const QString& label = QString(), int sortingWeight = 0);

    Section* getSection(const QString& name);

    Entry* createEntry(const QString& name, QVariant::Type type, const QVariant& defaultValue,
                       const QString& label, const QString& help, const QString& widgetHint = STRING,
                       int sortingWeight = 0, bool changeRequiresRestart = false,
                       const QList<Entry::Option>& optionList = QList<Entry::Option>());

    Entry* getEntry(const QString& path);

    void markAsChanged()
    {
        if (!m_label.startsWith("* ")) {
            m_label.prepend("* ");
        }
    }

    void markAsUnchanged()
    {
        if (m_label.startsWith("* ")) {
            m_label.remove(0, 2);
        }
    }

    void applyChanges();

    void discardChanges();

    void upload();

    void download();

    void sort(bool recursive = true);
};

class Settings
{
public:
    QString m_name;
    QString m_label;
    QString m_help;
    QList<Category*> m_categories;
    bool m_changesRequireRestart;
    PersistenceProvider* m_persistenceProvider;

    Settings(const QString& name, const QString& label, const QString& help)
        : m_name(name)
        , m_label(label.isEmpty() ? name : label)
        , m_help(help)
        , m_changesRequireRestart(false)
        , m_persistenceProvider(nullptr)
    {
    }

    QString getPath()
    {
        return m_name;
    }

    void setPersistenceProvider(PersistenceProvider* persistenceProvider)
    {
        m_persistenceProvider = persistenceProvider;
    }

    PersistenceProvider* getPersistenceProvider()
    {
        Q_ASSERT(m_persistenceProvider != nullptr);

        return m_persistenceProvider;
    }

    Category* createCategory(const QString& name, const QString& label, int sortingWeight = 0);

    Category* getCategory(const QString& name);

    Entry* getEntry(const QString& path);

    void markRequiresRestart()
    {
        m_changesRequireRestart = true;
    }

    void applyChanges();

    void discardChanges();

    void upload();

    void download();

    void sort(bool recursive = true);
};

} // namespace declaration
} // namespace settings
} // namespace CEED

#endif
