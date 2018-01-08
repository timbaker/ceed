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

#ifndef CEED_recentlyused_
#define CEED_recentlyused_

#include "CEEDBase.h"

#include <QAction>
#include <QString>

#include <functional>

class QSettings;

namespace CEED {
namespace recentlyused {

/** Implements reusable functionality for "recently used" lists/menus.

This is used mainly for "recent files" and "recent projects" in CEED.
We may use it for even more in the future
*/

// Stefan Stammberger is the original author of this file

/*!
\brief RecentlyUsed

This class can be used to store pointers to Items like files and images
    for later reuse within the application.

*/
class RecentlyUsed
{
public:
    QString m_sectionIdentifier;
    QSettings* m_qsettings;
    int m_recentlItemsNameTrimLength;
    int m_maxRecentItems;

    RecentlyUsed(QSettings* qsettings, const QString& sectionIdentifier)
        : m_sectionIdentifier("recentlyUsedIdentifier/" + sectionIdentifier)
        , m_qsettings(qsettings)
    {
        // how many recent items should the editor remember
        m_maxRecentItems = 10;
        // to how many characters should the recent file names be trimmed to
        m_recentlItemsNameTrimLength = 60;
    }

    /** Add an item to the list */
    void addRecentlyUsed(const QString& itemName);

    /** Removes an item from the list. Safe to call even if the item is
    not in the list.
    */
    bool removeRecentlyUsed(const QString& itemname);

    void clearRecentlyUsed();

    /** Returns all items as a string list */
    QStringList getRecentlyUsed();

    /** Trim the itemName to the max. length and return it */
    QString trimItemName(const QString& itemName);

      /** Converts a list into a string for storage in QSettings" */
    static QString stringListToString(const QStringList& list);

    /** Converts a string to a string list */
    QStringList stringToStringList(const QString& instr);

    /** reconstructs the string from a list and return it */
    static QString reconstructString(const QStringList& list)
    {
        QString reconst;
        for (QString s : list) {
            reconst = reconst + s;
        }

        return reconst;
    }
};

/*!
\brief RecentlyUsedMenuEntry

This class can be used to manage a Qt Menu entry to items.

*/
class RecentlyUsedMenuEntry : public QObject, public RecentlyUsed
{
    Q_OBJECT
public:
    QMenu* m_menu;
    std::function<void(const QString &)> m_slot;
    QAction* m_clearAction;

    RecentlyUsedMenuEntry(QSettings* qsettings, const QString& sectionIdentifier)
        : RecentlyUsed(qsettings, sectionIdentifier)
    {
        m_menu = nullptr;
 //       m_slot = nullptr;
        m_clearAction = nullptr;
    }

    /** Sets the parent menu and the slot that is called when clicked on an item
    */
    void setParentMenu(QMenu* menu, std::function<void(const QString &)> slot, QAction* clearAction);

    void addRecentlyUsed(const QString& itemName)
    {
        /** Adds an item to the list */

        RecentlyUsed::addRecentlyUsed(itemName);
        updateMenu();
    }

    void removeRecentlyUsed(const QString& itemName)
    {
        /** Removes an item from the list. Safe to call even if the item is not
        in the list.
        */

        RecentlyUsed::removeRecentlyUsed(itemName);
        updateMenu();
    }

private slots:
    void onTriggered();

    void clear()
    {
        RecentlyUsed::clearRecentlyUsed();
        updateMenu();
    }

public:
    void updateMenu();
};

} // namespace recentlyused
} // namespace CEED

#endif
