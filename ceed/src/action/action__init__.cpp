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

#include "action/action__init__.h"

namespace CEED {
namespace action {

ActionManager* ActionManager::instance = nullptr;

/////

Connection *ConnectionGroup::add(const QString &path, std::function<void ()> receiver, const QString &signalName, Qt::ConnectionType connectionType)
{
    if (m_actionManager == nullptr)
        throw RuntimeError("Action added by it's string name but action manager wasn't set, can't retrieve the action object!");
    return add(m_actionManager->getAction(path), receiver, signalName, connectionType);
}

Connection *ConnectionGroup::add(const QString &path, std::function<void (bool)> receiver, const QString &signalName, Qt::ConnectionType connectionType)
{
    if (m_actionManager == nullptr)
        throw RuntimeError("Action added by it's string name but action manager wasn't set, can't retrieve the action object!");
    return add(m_actionManager->getAction(path), receiver, signalName, connectionType);
}

Connection *ConnectionGroup::add(declaration::Action *action, std::function<void ()> receiver, const QString &signalName, Qt::ConnectionType connectionType)
{
    Connection* connection = new ConnectionVoid(action, receiver, signalName, connectionType);
    m_connections.append(connection);
    return connection;
}

Connection *ConnectionGroup::add(declaration::Action *action, std::function<void (bool)> receiver, const QString &signalName, Qt::ConnectionType connectionType)
{
    Connection* connection = new ConnectionBool(action, receiver, signalName, connectionType);
    m_connections.append(connection);
    return connection;
}

void ConnectionGroup::remove(Connection *connection, bool ensureDisconnected)
{
    if (!m_connections.contains(connection))
        throw RuntimeError("Can't remove given connection, it wasn't added!");

    m_connections.removeAll(connection);

    if (ensureDisconnected && connection->m_connected)
        connection->disconnect();
}

void ConnectionGroup::connectAll(bool skipConnected)
{
    for (Connection* connection : m_connections) {
        if (connection->m_connected && skipConnected)
            continue;

        connection->connect();
    }
}

void ConnectionGroup::disconnectAll(bool skipDisconnected)
{
    for (Connection* connection : m_connections) {
        if (!connection->m_connected && skipDisconnected)
            continue;

        connection->disconnect();
    }
}

/////

ActionManager::ActionManager(mainwindow::MainWindow* mainWindow, settings::Settings* settings)
    : declaration::ActionManager(mainWindow, settings)
{
    declaration::ActionCategory* general = createCategory("general", "General");
    general->createAction("application_settings", "Prefere&nces",
                          "Displays and allows changes of the application settings (persistent settings)",
                          QIcon(":/icons/actions/application_settings.png"),
                          QKeySequence(QKeySequence::Preferences),
                          "",
                          QAction::PreferencesRole);
    general->createAction("quit", "Quit",
                          "Exits the entire application safely with confirmation dialogs for all unsaved data.",
                          QIcon(":/icons/actions/quit.png"),
                          QKeySequence(QKeySequence::Quit),
                          "",
                          QAction::QuitRole);
    general->createAction("help_quickstart", "&Quickstart Guide",
                          "Opens the quick start guide (inside editor).",
                          QIcon(":/icons/actions/help_quickstart.png"),
                          QKeySequence(QKeySequence::HelpContents));
    general->createAction("help_user_manual", "&User manual",
                          "Opens the user manual (inside editor).",
                          QIcon(":/icons/actions/help_user_manual.png"));
    general->createAction("help_wiki_page", "&Wiki page",
                          "Opens the wiki page.",
                          QIcon(":/icons/actions/help_wiki_page.png"));
    general->createAction("send_feedback", "Send &Feedback",
                          "Opens the feedback forum.",
                          QIcon(":/icons/actions/help_feedback.png"));
    general->createAction("report_bug", "Report &Bug",
                          "Opens the issue tracker application.");
    general->createAction("cegui_debug_info", "&CEGUI debug info",
                          "Opens CEGUI debug info (FPS and log). Only useful when you have issues.");
    general->createAction("view_license", "L&icense",
                          "Displays the application license.");
    general->createAction("about_qt", "About &Qt",
                          "Display information about the Qt framework.",
                          QIcon(),
                          QKeySequence(),
                          "",
                          QAction::AboutQtRole);
    general->createAction("about", "&About",
                          "Display information about the application.",
                          QIcon(":/icons/actions/help_about.png"),
                          QKeySequence(),
                          "",
                          QAction::AboutRole);
    general->createAction("full_screen", "&Full Screen",
                          "Switches between full screen view and normal view.",
                          QIcon(":/icons/actions/view_fullscreen.png"),
                          QKeySequence("F11"),
                          "Toggle Full Screen");
    general->createAction("statusbar", "&Statusbar",
                          "Hides and shows the visibility status bar.",
                          QIcon(),
                          QKeySequence(),
                          "Toggle Statusbar")->setCheckable(true);

    declaration::ActionCategory* projectManagement = createCategory("project_management", "Project Management");
    projectManagement->createAction("new_project", "Pro&ject...",
                                    "Creates a new project from scratch. Only one project can be opened at a time so you will be asked to close your current project if any.",
                                    QIcon(":/icons/actions/new_project.png"),
                                    QKeySequence(),
                                    "New Project");
    projectManagement->createAction("open_project", "Open Pro&ject...",
                                    "Opens a pre-existing project file. Only one project can be opened at a time so you will be asked to close your current project if any.",
                                    QIcon(":/icons/actions/open_project.png"));
    projectManagement->createAction("save_project", "Save Pro&ject",
                                    "Saves currently opened project file to the same location from where it was opened.",
                                    QIcon(":/icons/actions/save_project.png"));
    projectManagement->createAction("close_project", "Close Project",
                                    "Closes currently opened project file.",
                                    QIcon(":/icons/actions/close_project.png"));
    projectManagement->createAction("project_settings", "&Settings",
                                    "Displays and allows changes of the project settings (of the currently opened project).",
                                    QIcon(":/icons/actions/project_settings.png"),
                                    QKeySequence(),
                                    "",
                                    QAction::NoRole);
    projectManagement->createAction("reload_resources", "&Reload Resources",
                                    "Reloads all CEGUI resources associated with currently opened project.",
                                    QIcon(":/icons/project_management/reload_resources.png"));

    declaration::ActionCategory* file = createCategory("files", "Tabs and Files");
    file->createAction("new_file", "&Other...",
                       "Creates a new empty file of any type.",
                       QIcon(":/icons/actions/new_file.png"),
                       QKeySequence(QKeySequence::New),
                       "New File");
    file->createAction("new_layout", "&Layout...",
                       "Creates a new layout file.",
                       QIcon(":/icons/project_items/layout.png"),
                       QKeySequence(),
                       "New Layout");
    file->createAction("new_imageset", "&Imageset...",
                       "Creates a new imageset file.",
                       QIcon(":/icons/project_items/imageset.png"),
                       QKeySequence(),
                       "New Imageset");
    file->createAction("revert_file", "Re&vert",
                       "Reverts the active file to version on disk.",
                       QIcon(),
                       QKeySequence(),
                       "Revert File");
    file->createAction("clear_recent_files", "&Clear",
                       "Empties the recent files list.",
                       QIcon(),
                       QKeySequence(),
                       "Clear Recent Files"
                       );
    file->createAction("clear_recent_projects", "&Clear",
                       "Empties the recent projects list.",
                       QIcon(),
                       QKeySequence(),
                       "Clear Recent Projects"
                       );
    file->createAction("open_file", "&Open File...",
                       "Opens a pre-existing file from any location (if the file is already opened the tab with it will be switched to).",
                       QIcon(":/icons/actions/open_file.png"),
                       QKeySequence(QKeySequence::Open));
    file->createAction("save_file", "&Save",
                       "Saves currently opened (and active - currently edited) file to its original location.",
                       QIcon(":/icons/actions/save.png"),
                       QKeySequence(QKeySequence::Save),
                       "Save File");
    file->createAction("save_file_as", "Save &As",
                       "Saves currently opened (and active - currently edited) file to a custom location.",
                       QIcon(":/icons/actions/save_as.png"),
                       QKeySequence(QKeySequence::SaveAs),
                       "Save File As");
    file->createAction("save_all", "Save A&ll",
                       "Saves currently opened project file (if any) and all currently opened files to their original location.",
                       QIcon(":/icons/actions/save_all.png"),
                       QKeySequence());
    file->createAction("close_current_tab", "&Close",
                       "Closes currently active (and switched to) tab - asks for confirmation if there are unsaved changes.",
                       QIcon(":/icons/actions/close_tab.png"),
                       QKeySequence(QKeySequence::Close),
                       "Close Tab");
    file->createAction("close_other_tabs", "Close &Other",
                       "Closes all tabs except the one that is currently active - asks for confirmation if there are unsaved changes.",
                       QIcon(":/icons/actions/close_other_tabs.png"),
                       QKeySequence(),
                       "Close Other Tabs");
    file->createAction("close_all_tabs", "Close A&ll",
                       "Closes all tabs - asks for confirmation if there are unsaved changes.",
                       QIcon(":/icons/actions/close_all_tabs.png"),
                       QKeySequence(),
                       "Close All Tabs");
    file->createAction("previous_tab", "&Previous Tab",
                       "Activates the previous (left) tab.",
                       QIcon(),
                       QKeySequence("Ctrl+PgUp"));
    file->createAction("next_tab", "&Next Tab",
                       "Activates the next (right) tab.",
                       QIcon(),
                       QKeySequence("Ctrl+PgDown"));

    declaration::ActionCategory* allEditors = createCategory("all_editors", "All Editors");
    allEditors->createAction("zoom_in", "Zoom &In",
                             "Increases the zoom level.",
                             QIcon(":/icons/actions/zoom_in.png"),
                             QKeySequence(QKeySequence::ZoomIn));
    allEditors->createAction("zoom_out", "Zoom &Out",
                             "Decreases the zoom level.",
                             QIcon(":/icons/actions/zoom_out.png"),
                             QKeySequence(QKeySequence::ZoomOut));
    allEditors->createAction("zoom_reset", "&Normal Size",
                             "Resets zoom level to the default (1:1).",
                             QIcon(":/icons/actions/zoom_original.png"),
                             QKeySequence("Ctrl+0"),
                             "Zoom Reset");
    allEditors->createAction("undo", "&Undo",
                             "Undoes the last operation (in the currently active tabbed editor)",
                             QIcon(":/icons/actions/undo.png"),
                             QKeySequence(QKeySequence::Undo));
    allEditors->createAction("redo", "&Redo",
                             "Redoes the last undone operation (in the currently active tabbed editor)",
                             QIcon(":/icons/actions/redo.png"),
                             QKeySequence(QKeySequence::Redo));
    allEditors->createAction("copy", "&Copy",
                             "Performs a clipboard copy",
                             QIcon(":/icons/actions/copy.png"),
                             QKeySequence(QKeySequence::Copy));
    allEditors->createAction("cut", "Cu&t",
                             "Performs a clipboard cut",
                             QIcon(":/icons/actions/cut.png"),
                             QKeySequence(QKeySequence::Cut));
    allEditors->createAction("paste", "&Paste",
                             "Performs a clipboard paste",
                             QIcon(":/icons/actions/paste.png"),
                             QKeySequence(QKeySequence::Paste));
    allEditors->createAction("delete", "&Delete",
                             "Deletes the selected items",
                             QIcon(":/icons/actions/delete.png"),
                             QKeySequence(QKeySequence::Delete));
    // Imageset editor
    editors::imageset::action_decl::declare(this);

    // Layout editor
    editors::layout::action_decl::declare(this);

    // Look n' Feel editor
    editors::looknfeel::action_decl::declare(this);

    Q_ASSERT(ActionManager::instance == nullptr);
    ActionManager::instance = this;
}

/////

declaration::Action *getAction(const QString &path)
{
    Q_ASSERT(ActionManager::instance != nullptr);
    return ActionManager::instance->getAction(path);
}

} // namesapce action
} // namespace CEED
