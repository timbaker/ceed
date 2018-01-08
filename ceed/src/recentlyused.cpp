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

#include "recentlyused.h"

#include <QMenu>
#include <QSettings>

namespace CEED {
namespace recentlyused {

void RecentlyUsed::addRecentlyUsed(const QString &itemName)
{
    QStringList items;
    if (m_qsettings->contains(m_sectionIdentifier)) {
        QString val = m_qsettings->value(m_sectionIdentifier).toString();
        items = stringToStringList(val);
    }

    bool isInList = false;
    for (QString f : items) {
        if (f == itemName) {
            items.removeOne(f);
            items.insert(0, f);
            isInList = true;
            break;
        }
    }

    // only insert the file if it is not already in list
    if (!isInList) {
        items.insert(0, itemName);
    }

    // while because items could be in a bad state because of previously thrown exceptions
    // make sure we trim them correctly in all circumstances
    while (items.length() > m_maxRecentItems) {
        items.removeLast();
    }

    m_qsettings->setValue(m_sectionIdentifier, RecentlyUsed::stringListToString(items));
}

bool RecentlyUsed::removeRecentlyUsed(const QString &itemname)
{
    QStringList items;
    if (m_qsettings->contains(m_sectionIdentifier)) {
        QString val = m_qsettings->value(m_sectionIdentifier).toString();
        items = stringToStringList(val);
    }

    if (!items.contains(itemname)) {
        return false;
    }

    items.removeOne(itemname);

    m_qsettings->setValue(m_sectionIdentifier, RecentlyUsed::stringListToString(items));
    return true;
}

void RecentlyUsed::clearRecentlyUsed()
{
    m_qsettings->remove(m_sectionIdentifier);
}

QStringList RecentlyUsed::getRecentlyUsed()
{
    QStringList items;
    if (m_qsettings->contains(m_sectionIdentifier)) {
        QString val = m_qsettings->value(m_sectionIdentifier).toString();
        items = stringToStringList(val);
    }

    return items;
}

QString RecentlyUsed::trimItemName(const QString &itemName)
{
    if (itemName.length() > m_recentlItemsNameTrimLength) {
        // + 3 because of the ...
        QString trimedItemName = QString("...%1").arg(itemName.right(m_recentlItemsNameTrimLength));
        return trimedItemName;
    } else {
        return itemName;
    }
}

QString RecentlyUsed::stringListToString(const QStringList &list)
{
    QString temp;

    bool first = true;
    for (QString s : list) {
        QString t = s.replace(";", "\\;");
        if (first) {
            temp = t;
            first = false;
        } else {
            temp = temp + ";" + t;
        }
    }

    return temp;
}

QStringList RecentlyUsed::stringToStringList(const QString &instr)
{
    QString workStr = instr;
    QStringList list;

    if (workStr.indexOf("\\;") == -1) {
        return instr.split(";");
    } else {
        QStringList tempList;
        int pos = workStr.indexOf(';');
        while (pos != -1) {
            // if we find a string that is escaped split the text and add it
            // to the temporary list
            // the path will be reconstructed when we don't find any escaped
            // semicolons anymore (in else)

            if (workStr.mid(pos-1, 3) == "\\;") {
                tempList.append(workStr.mid(0, pos+1).replace("\\;", ";"));
                workStr = workStr.mid(pos+1);
                pos = workStr.indexOf(';');
                if (pos < 0) {
                    // end reached, finalize
                    list.append(reconstructString(tempList) + workStr);
                    workStr = workStr.mid(pos + 1);
                    tempList.clear();
                }
            } else {
                // found a unescaped ; so we reconstruct the string before it,
                // it should be a complete item
                list.append(reconstructString(tempList) + workStr.mid(pos));
                workStr = workStr.mid(pos + 1);
                pos = workStr.indexOf(';');
                tempList.clear();
            }
        }

        return list;
    }
}

/////

void RecentlyUsedMenuEntry::setParentMenu(QMenu *menu, std::function<void(const QString&)> slot, QAction *clearAction)
{
    m_menu = menu;
    m_slot = slot;
    m_clearAction = clearAction;
    QObject::connect(m_clearAction, &QAction::triggered, [this]() { clear(); });
    updateMenu();
}

void RecentlyUsedMenuEntry::onTriggered()
{
    if (auto action = dynamic_cast<QAction*>(sender())) {
        QString filePath = action->data().toString();
        m_slot(filePath);
    }
}

void RecentlyUsedMenuEntry::updateMenu()
{
    m_menu->clear();
    QStringList items = getRecentlyUsed();

    int i = 1;
    for (QString& f : items) {
        QAction* actionRP = new QAction(m_menu);
        QString text = trimItemName(f);
        if (i <= 10) {
            text = "&" + QString::number(i % 10) + ". " + text;
        }

        actionRP->setText(text);
        actionRP->setData(f);
        actionRP->setVisible(true);
//        QObject::connect(actionRP, &QAction::triggered, m_slot);
        QObject::connect(actionRP, &QAction::triggered, this, &RecentlyUsedMenuEntry::onTriggered);
        m_menu->addAction(actionRP);
        i += 1;
    }

    m_menu->addSeparator();
    m_menu->addAction(m_clearAction);
    m_clearAction->setEnabled(items.length() > 0);
}


} // namespace recentlyused
} // namespace CEED
