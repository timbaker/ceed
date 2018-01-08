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

#ifndef CEED_action_declaration_
#define CEED_action_declaration_

#include "CEEDBase.h"

/** Provides API to declare actions that are remapable
(shortcuts can be changed) using the settings interface

All actions in the application should be declared using this API, unless
there are serious reasons not to do that.
*/

#include "settings/settings_init.h"

#include <QAction>
#include <QKeySequence>
#include <QIcon>

namespace CEED {
namespace action {
namespace declaration {

class ActionCategory;
class ActionManager;

/*!
\brief Action

The only thing different in this from QAction is the ability to change the shortcut of it
    using CEED's settings API/interface.

    While it isn't needed/required to use this everywhere where QAction is used, it is recommended.

*/
class Action : public QAction
{
public:
    ActionCategory* m_category;
    QString m_name;
    QKeySequence m_defaultShortcut;
    QString m_settingsLabel;
    settings::declaration::Entry* m_settingsEntry;

    Action(ActionCategory* category, const QString& name, const QString& label = QString(), const QString& help = QString(),
           const QIcon& icon = QIcon(), const QKeySequence& defaultShortcut = QKeySequence(), const QString& settingsLabel = QString(),
           QAction::MenuRole menuRole = QAction::TextHeuristicRole);

    ActionManager *getManager();

    void declareSettingsEntry();

    void entryValueChanged(const QVariant& value);
};

/*!
\brief ActionCategory

Actions are grouped into categories

    examples: General for all round actions (Exit, Undo, Redo, ...), Layout editing for layout editing (duh!), ...

*/
class ActionCategory
{
public:
    ActionManager* m_manager;
    QString m_name;
    QString m_label;
    settings::declaration::Section* m_settingsSection;
    QList<Action*> m_actions;

    ActionCategory(ActionManager* manager, const QString& name, const QString& label);

    ActionManager* getManager()
    {
        return m_manager;
    }

    Action* createAction(const QString& name, const QString& label = QString(), const QString& help = QString(),
                         const QIcon& icon = QIcon(), const QKeySequence& defaultShortcut = QKeySequence(), const QString& settingsLabel = QString(),
                         QAction::MenuRole menuRole = QAction::TextHeuristicRole);

    Action* getAction(const QString& name);

    void setEnabled(bool enabled);

    void declareSettingsSection();
};

/*!
\brief ActionManager

Usually a singleton that manages all action categories and therefore
    actions within them.

*/
class ActionManager
{
public:
    mainwindow::MainWindow* m_mainWindow;
    settings::Settings* m_settings;
    settings::declaration::Category* m_settingsCategory;
    QList<ActionCategory*> m_categories;

    ActionManager(mainwindow::MainWindow* mainWindow, settings::Settings* settings);

    ActionCategory* createCategory(const QString& name, const QString& label);

    ActionCategory* getCategory(const QString& name);

    Action* getAction(const QString& path);

    void declareSettingsCategory();
};

} // namespace declaration
} // namespace action
} // namespace CEED

#endif
