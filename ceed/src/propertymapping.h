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

#ifndef CEED_propertymapping_
#define CEED_propertymapping_

#include "CEEDBase.h"

/**Settings for CEGUI properties.

PropertyMappingEntry -- Settings for one property, identified by its origin and name.
PropertyMap -- PropertyMappingEntry container.
*/

#include "elementtree.h"

#include <QMap>
#include <QString>

namespace CEED {
namespace propertymapping {

/*!
\brief PropertyMappingEntry

Maps a CEGUI::Property (by origin and name) to a CEGUI Type and PropertyEditor
    to allow its viewing and editing.

    If target inspector name is \"\" then this mapping means that the property should
    be ignored in the property set inspector listing.

*/
class PropertyMappingEntry
{
public:
    QString m_propertyOrigin;
    QString m_propertyName;
    QString m_typeName;
    bool m_hidden;
    QString m_editorName;
    typedef QVariantMap EditorSettings;
    EditorSettings m_editorSettings;

    PropertyMappingEntry(const QString& propertyOrigin, const QString& propertyName,
                 const QString& typeName = "", bool hidden = false,
                 const QString& editorName = "", const EditorSettings& editorSettings = EditorSettings());

    PropertyMappingEntry(ElementTree::Element* element);

    static QString makeKey(const QString& propertyOrigin, const QString& propertyName);

    QString getPropertyKey() const;

    ElementTree::Element* saveToElement();
};

/*!
\brief PropertyMap

Container for property mapping entries.
*/
class PropertyMap
{
public:
    QMap<QString, PropertyMappingEntry*> m_entries;

    static PropertyMap* fromElement(ElementTree::Element* element);

    /**Create and return an instance from an XML string.*/
    static PropertyMap* fromXMLString(const QString &text);

    /** Create and return an instance from an XML file path.*/
    static PropertyMap* fromFile(const QString &absolutePath);

    /**Create and return an instance from a list of XML file paths.

    Entries from files later in the list replace entries with the same
    property key from previous files.
    */
    static PropertyMap* fromFiles(const QStringList& absolutePaths);

    PropertyMap()
    {
        /**Initialise an empty property map.*/
    }

    /**Create and return an XML element for this map instance.*/
    ElementTree::Element* saveToElement();

    /**Find and return the entry with the specified origin and name, or None.*/
    PropertyMappingEntry* getEntry(const QString& propertyOrigin, const QString& propertyName);

    /**Set or replace an entry with a new entry.*/
    void setEntry(PropertyMappingEntry* entry);

    /**Update the entries using the entries of another map.*/
    void update(PropertyMap* pmap);
};

//from ceed.compatibility import property_mappings as compat

} // namespace propertymapping
} // namespace CEED

#endif
