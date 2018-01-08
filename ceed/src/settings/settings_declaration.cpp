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

#include "settings_declaration.h"

#include "settings/settings_persistence.h"

namespace CEED {
namespace settings {
namespace declaration {

Entry::Entry(Section *section, const QString &name, QVariant::Type type, const QVariant &defaultValue,
             const QString &label, const QString &help, const QString &widgetHint, int sortingWeight,
             bool changeRequiresRestart,const QList<Entry::Option> &optionList)
    : m_section(section)
    , m_name(name)
    , m_type(type)
//        , m_defaultValue(defaultValue)
    , m_label(label.isEmpty() ? name : label)
    , m_help(help)
    , m_widgetHint(widgetHint)
    , m_hasChanges(false)
    , m_sortingWeight(sortingWeight)
    , m_changeRequiresRestart(changeRequiresRestart)
    , m_optionList(optionList)
{
    m_defaultValue = sanitizeValue(defaultValue);
    m_value = m_defaultValue;
    m_editedValue = m_defaultValue;
}

QString Entry::getPath()
{
    return QString("%1/%2").arg(m_section->getPath()).arg(m_name);
}

persistence::PersistenceProvider *Entry::getPersistenceProvider()
{
    return m_section->getPersistenceProvider();
}

Settings &Entry::getSettings()
{
    return m_section->getSettings();
}

void Entry::_setValue(const QVariant &value, bool uploadImmediately)
{
    m_value = sanitizeValue(value);
    m_editedValue = m_value;

    if (uploadImmediately) {
        upload();
    }

    std::for_each(m_subscribers.begin(), m_subscribers.end(), [=](Subscription& s) {
        s.cb(value); // FIXME: or m_value ?
    });
}

void Entry::applyChanges()
{
    if (m_value != m_editedValue) {
        _setValue(m_editedValue);

        if (m_changeRequiresRestart) {
            getSettings().markRequiresRestart();
        }
    }
}

void Entry::discardChanges()
{
    m_editedValue = m_value;

    // - This is set via the property, but from this context we know that
    //   we really don't have any changes.
    m_hasChanges = false;
}

void Entry::upload()
{
    getPersistenceProvider()->upload(this, m_value);
    m_hasChanges = false;
}

void Entry::download()
{
    QVariant persistedValue = getPersistenceProvider()->download(this);
    if (!persistedValue.isValid())
        persistedValue = m_defaultValue;

    // http://bugs.pyside.org/show_bug.cgi?id=345
    if (m_widgetHint == "checkbox") {
        if (persistedValue == "false") {
            persistedValue = false;
        } else if (persistedValue == "true") {
            persistedValue = true;
        }
    }

    _setValue(persistedValue, false);
    m_hasChanges = false;
}

/////

QString Section::getPath()
{
    return QString("%1/%2").arg(m_category->getPath()).arg(m_name);
}

persistence::PersistenceProvider *Section::getPersistenceProvider()
{
    return m_category->getPersistenceProvider();
}

Settings& Section::getSettings()
{
    return m_category->getSettings();
}

/////

QString Category::getPath()
{
    return QString("%1/%2").arg(m_settings->getPath()).arg(m_name);
}

persistence::PersistenceProvider *Category::getPersistenceProvider()
{
    return m_settings->getPersistenceProvider();
}

/////

Section *Category::createSection(const QString &name, const QString &label, int sortingWeight)
{
    auto section = new Section(this, name, label, sortingWeight);
    m_sections.append(section);
    return section;
}

Section *Category::getSection(const QString &name)
{
    foreach (Section* section, m_sections) {
        if (section->m_name == name) {
            return section;
        }
    }

    throw RuntimeError("Section '" + name + "' not found in category '" + m_name + "' of this settings");
}

Entry *Category::createEntry(const QString &name, QVariant::Type type, const QVariant &defaultValue, const QString &label,
                             const QString &help, const QString &widgetHint, int sortingWeight, bool changeRequiresRestart,
                             const QList<Entry::Option> &optionList)
{
    if (getSection("") == nullptr) {
        Section* section = createSection("");
        section->m_sortingWeight = -1;
    }

    Section* section = getSection("");
    return section->createEntry(name, type, defaultValue, label, help, widgetHint, sortingWeight, changeRequiresRestart, optionList);
}

Entry *Category::getEntry(const QString &path)
{
    // FIXME: Needs better error handling
    QString splitted0 = path.section('/', 0, 0);
    QString splitted1 = path.section('/', 1);
    Q_ASSERT(!splitted0.isEmpty() && !splitted1.isEmpty());

    Section* section = getSection(splitted0);
    return section->getEntry(splitted1);
}

void Category::applyChanges()
{
    for (Section* section : m_sections) {
        section->applyChanges();
    }
}

void Category::discardChanges()
{
    for (Section* section : m_sections) {
        section->discardChanges();
    }
}

void Category::upload()
{
    for (Section* section : m_sections) {
        section->upload();
    }
}

void Category::download()
{
    for (Section* section : m_sections) {
        section->download();
    }
}

void Category::sort(bool recursive)
{
    std::stable_sort(m_sections.begin(), m_sections.end(), [](Section* a, Section* b) {
        return a->m_name < b->m_name;
    });

    std::stable_sort(m_sections.begin(), m_sections.end(), [](Section* a, Section* b) {
        return a->m_sortingWeight < b->m_sortingWeight;
    });

    if (recursive) {
        for (Section* section : m_sections) {
            section->sort();
        }
    }
}

/////

Category *Settings::createCategory(const QString &name, const QString &label, int sortingWeight)
{
    Category* category = new Category(this, name, label, sortingWeight);
    m_categories.append(category);
    return category;
}

Category *Settings::getCategory(const QString &name)
{
    for (Category* category : m_categories) {
        if (category->m_name == name) {
            return category;
        }
    }

    throw RuntimeError("Category '" + name + "' not found in this settings");
}

Entry *Settings::getEntry(const QString &path)
{
    // FIXME: Needs better error handling
    QString splitted0 = path.section('/', 0, 0);
    QString splitted1 = path.section('/', 1);
    Q_ASSERT(!splitted0.isEmpty() && !splitted1.isEmpty());

    Category* category = getCategory(splitted0);
    return category->getEntry(splitted1);
}

void Settings::applyChanges()
{
    for (Category* category : m_categories) {
        category->applyChanges();
    }
}

void Settings::discardChanges()
{
    for (Category* category : m_categories) {
        category->discardChanges();
    }
}

void Settings::upload()
{
    for (Category* category : m_categories) {
        category->upload();
    }
}

void Settings::download()
{
    for (Category* category : m_categories) {
        category->download();
    }

    m_changesRequireRestart = false;
}

void Settings::sort(bool recursive)
{
    std::stable_sort(m_categories.begin(), m_categories.end(), [](Category* a, Category* b) {
        return a->m_name < b->m_name;
    });

    std::stable_sort(m_categories.begin(), m_categories.end(), [](Category* a, Category* b) {
        return a->m_sortingWeight < b->m_sortingWeight;
    });

    if (recursive) {
        for (Category* category : m_categories) {
            category->sort();
        }
    }
}




} // declaration
} // settings
} // CEED
