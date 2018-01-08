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

#include "action/declaration.h"

#include "mainwindow.h"

namespace CEED {
namespace action {
namespace declaration {

Action::Action(ActionCategory *category, const QString &name, const QString &label, const QString &help,
               const QIcon &icon, const QKeySequence &defaultShortcut, const QString &settingsLabel,
               QAction::MenuRole menuRole)
    // QAction needs a QWidget parent, we use the main window as that
    : QAction(icon, label.isEmpty() ? name : label, category->getManager()->m_mainWindow)
{
    QString label_ = label;
    QString settingsLabel_ = settingsLabel;

    if (label_.isEmpty())
        label_ = name;
    if (settingsLabel_.isEmpty()) {
        // remove trailing ellipsis and mnemonics
        settingsLabel_ = label;
        while (settingsLabel_.endsWith('.'))
            settingsLabel_.chop(1);
        settingsLabel_.replace("&&", "%amp%").replace("&", "").replace("%amp%", "&&");
    }

    m_category = category;
    m_name = name;
    m_defaultShortcut = defaultShortcut;
    m_settingsLabel = settingsLabel_;

    setToolTip(settingsLabel_);
    setStatusTip(help);
    setMenuRole(menuRole);

    // we default to application shortcuts and we make sure we always disable the ones we don't need
    // you can override this for individual actions via the setShortcutContext method as seen here
    setShortcutContext(Qt::ApplicationShortcut);
    setShortcut(defaultShortcut);

    m_settingsEntry = nullptr;
    declareSettingsEntry();
}

ActionManager* Action::getManager()
{
    return m_category->getManager();
}

void Action::declareSettingsEntry()
{
    settings::declaration::Section* section = m_category->m_settingsSection;

    m_settingsEntry = section->createEntry("shortcut_" + m_name, QVariant::KeySequence, m_defaultShortcut, m_settingsLabel,
                                           "", "keySequence");

    // when the entry changes, we want to change our shortcut too!
    m_settingsEntry->subscribe([=](const QVariant& v) { entryValueChanged(v); });
}

void Action::entryValueChanged(const QVariant &value)
{
    setShortcut(value.value<QKeySequence>());
}

/////

ActionCategory::ActionCategory(ActionManager *manager, const QString &name, const QString &label)
{
    m_manager = manager;
    m_name = name;
    m_label = label.isEmpty() ? name : label;

    m_settingsSection = nullptr;
    declareSettingsSection();
}

Action *ActionCategory::createAction(const QString &name, const QString &label, const QString &help, const QIcon &icon, const QKeySequence &defaultShortcut, const QString &settingsLabel, QAction::MenuRole menuRole)
{
    Action* action = new Action(this, name, label, help, icon, defaultShortcut, settingsLabel, menuRole);
    m_actions += action;

    return action;
}

Action *ActionCategory::getAction(const QString &name)
{
    for (Action* action : m_actions) {
        if (action->m_name == name)
            return action;
    }

    throw RuntimeError("Action '" + name + "' not found in category '" + m_name + "'.");
}

void ActionCategory::setEnabled(bool enabled)
{
    /** Allows you to enable/disable actions en masse, especially useful when editors are switched.
        This gets rid of shortcut clashes and so on.
        */

    for (Action* action : m_actions) {
        action->setEnabled(enabled);
    }
}

void ActionCategory::declareSettingsSection()
{
    settings::declaration::Category* category = getManager()->m_settingsCategory;

    m_settingsSection = category->createSection(m_name, m_label);
}

/////

ActionManager::ActionManager(mainwindow::MainWindow *mainWindow, settings::Settings *settings)
    : m_mainWindow(mainWindow)
    , m_settings(settings)
    , m_settingsCategory(nullptr)
{
    declareSettingsCategory();
}

ActionCategory *ActionManager::createCategory(const QString &name, const QString &label)
{
    auto category = new ActionCategory(this, name, label);
    m_categories.append(category);

    return category;
}

ActionCategory *ActionManager::getCategory(const QString &name)
{
    for (ActionCategory* category : m_categories) {
        if (category->m_name == name)
            return category;
    }

    throw RuntimeError("Category '" + name + "' not found in this action manager");
}

Action *ActionManager::getAction(const QString &path)
{
    // FIXME: Needs better error handling
    QString splitted0 = path.section('/', 0, 0);
    QString splitted1 = path.section('/', 1);
    Q_ASSERT(!splitted0.isEmpty() && !splitted1.isEmpty());

    ActionCategory* category = getCategory(splitted0);
    return category->getAction(splitted1);
}

void ActionManager::declareSettingsCategory()
{
    m_settingsCategory = m_settings->createCategory("action_shortcuts", "Shortcuts", 999);
}


} // namespace declaration
} // namespace action
} // namespace CEED
