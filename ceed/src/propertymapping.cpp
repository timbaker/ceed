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

#include "propertymapping.h"

#include "compatibility/property_mappings/compat_property_mappings_init.h"

namespace CEED {
namespace propertymapping {

PropertyMappingEntry::PropertyMappingEntry(const QString &propertyOrigin, const QString &propertyName, const QString &typeName,
                                           bool hidden, const QString &editorName, const EditorSettings &editorSettings)
{
    m_propertyOrigin = propertyOrigin;
    m_propertyName = propertyName;
    m_typeName = typeName;
    m_hidden = hidden;
    m_editorName = editorName;
    m_editorSettings = editorSettings;
}

PropertyMappingEntry::PropertyMappingEntry(ElementTree::Element *element)
{
    m_propertyOrigin = element->get("propertyOrigin");
    m_propertyName = element->get("propertyName");
    m_typeName = element->get("typeName");
    QStringList trueStrings = { "true", "yes", "1" };
    m_hidden =  trueStrings.contains(element->get("hidden", "false").toLower());
    m_editorName = element->get("editorName");

    for (ElementTree::Element* settings : element->findall("settings")) {
        QString name = settings->get("name");
        QVariantMap entries;
        for (ElementTree::Element* setting : settings->findall("setting")) {
            entries[setting->get("name")] = setting->get("value");
        }
        m_editorSettings[name] = entries;
    }
}

QString PropertyMappingEntry::makeKey(const QString &propertyOrigin, const QString &propertyName)
{
    return propertyOrigin + "/" + propertyName;
}

QString PropertyMappingEntry::getPropertyKey() const
{
    return makeKey(m_propertyOrigin, m_propertyName);
}

ElementTree::Element *PropertyMappingEntry::saveToElement()
{
    auto element = new ElementTree::Element("mapping");

    element->set("propertyOrigin", m_propertyOrigin);
    element->set("propertyName", m_propertyName);
    if (!m_typeName.isEmpty())
        element->set("typeName", m_typeName);
    if (m_hidden)
        element->set("hidden", true);
    if (!m_editorName.isEmpty())
        element->set("editorName", m_editorName);

    for (auto it = m_editorSettings.begin(); it != m_editorSettings.end(); it++) {
        QString name = it.key();
        auto value = it.value().toMap();
        auto settings = new ElementTree::Element("settings");
        settings->set("name", name);
        for (auto it2 = value.begin(); it2 != value.end(); it2++) {
            QString sname = it2.key();
            QString svalue = it2.value().toString();
            auto setting = new ElementTree::Element("setting");
            setting->set("name", sname);
            setting->set("value", svalue);
            element->append(setting);
        }
        element->append(settings);
    }

    return element;
}

/////

PropertyMap *PropertyMap::fromElement(ElementTree::Element *element)
{
    /**Create and return an instance from an XML element.*/
    Q_ASSERT(element->get("version") == compatibility::property_mappings::manager->EditorNativeType);

    auto pmap = new PropertyMap();
    for (ElementTree::Element* entryElement : element->findall("mapping")) {
        auto entry = new PropertyMappingEntry(entryElement);
        pmap->setEntry(entry);
    }
    return pmap;
}

PropertyMap *PropertyMap::fromXMLString(const QString &text)
{
    auto element = ElementTree::fromstring(text);
    return fromElement(element);
}

PropertyMap *PropertyMap::fromFile(const QString &absolutePath)
{
    QFile file(absolutePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        throw IOError(QString("QFile::open failed for file \"%1\"").arg(absolutePath)); // FIXME: exception?
    }
    QByteArray data = file.readAll();
    return fromXMLString(QString::fromUtf8(data));
}

PropertyMap *PropertyMap::fromFiles(const QStringList &absolutePaths)
{
    auto pmap = new PropertyMap();
    for (QString absolutePath : absolutePaths) {
        pmap->update(fromFile(absolutePath));
    }

    return pmap;
}

ElementTree::Element *PropertyMap::saveToElement()
{
    auto element = new ElementTree::Element("mappings");
    element->set("version", compatibility::property_mappings::manager->EditorNativeType);

    QList<PropertyMappingEntry*> sorted = m_entries.values();
    std::sort(sorted.begin(), sorted.end(),
              [](PropertyMappingEntry*& a, PropertyMappingEntry*& b) {
        return a->getPropertyKey() < b->getPropertyKey();
    });
    for (auto entry : sorted) {
        auto eel = entry->saveToElement();
        element->append(eel);
    }

    return element;
}

PropertyMappingEntry *PropertyMap::getEntry(const QString &propertyOrigin, const QString &propertyName)
{
    QString key = PropertyMappingEntry::makeKey(propertyOrigin, propertyName);
    PropertyMappingEntry* entry = m_entries[key];
    return entry;
}

void PropertyMap::setEntry(PropertyMappingEntry *entry)
{
    m_entries[entry->getPropertyKey()] = entry;
}

void PropertyMap::update(PropertyMap *pmap)
{
    m_entries.unite(pmap->m_entries);
}


} // namespace propertymapping
} // namespace CEED
