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

#ifndef CEED_mainwindow_
#define CEED_mainwindow_

#include "CEEDBase.h"

#include <QMainWindow>

class Ui_MainWindow;

class QMenu;
class QTabBar;
class QTabWidget;

namespace CEED {
namespace mainwindow {

/*!
\brief MainWindow

The central window of the application.

    This is a singleton class, it is assured to only be constructed once during runtime.

*/
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    // singleton pattern implementation
    // (usage: mainwindow::MainWindow::instance->doSomething() where mainwindow is the module)
    static MainWindow* instance;

    // TODO: This class has grown too much, I think it has too many responsibilities
    //       and refactoring will be needed in the future.
#if 1
    application::Application& m_app;
    project::Project* m_project;
    editors::TabbedEditor* m_activeEditor;
    bool m_wasMaximized;
    action::ActionManager* m_actionManager;
    settings::interface_::QtSettingsInterface* m_settingsInterface;
    recentlyused::RecentlyUsedMenuEntry* m_recentlyUsedProjects;
    recentlyused::RecentlyUsedMenuEntry* m_recentlyUsedFiles;
    QList<editors::TabbedEditorFactory*> m_editorFactories;
    QStringList m_editorFactoryFileFilters;
    Ui_MainWindow* m_ui;
    QTabWidget* m_tabs;
    QTabBar* m_tabBar;
    QList<editors::TabbedEditor*> m_tabEditors;
    project::ProjectManager* m_projectManager;
    filesystembrowser::FileSystemBrowser* m_fileSystemBrowser;
    commands::UndoViewer* m_undoViewer;
    action::ConnectionGroup* m_connectionGroup;

    cegui::Instance* ceguiInstance;
    cegui::container::ContainerWidget* ceguiContainerWidget;

    using Action = action::declaration::Action;

    Action* m_newFileAction;
    Action* m_newLayoutAction;
    Action* m_newImagesetAction;
    Action* m_openFileAction;
    Action* m_saveAction;
    Action* m_saveAsAction;
    Action* m_saveAllAction;
    Action* m_closeTabAction;
    Action* m_closeOtherTabsAction;
    Action* m_closeAllTabsAction;
    Action* m_previousTabAction;
    Action* m_nextTabAction;
    Action* m_clearRecentFilesAction;
    Action* m_clearRecentProjectsAction;
    Action* m_undoAction;
    Action* m_redoAction;
    Action* m_revertAction;
    Action* m_cutAction;
    Action* m_copyAction;
    Action* m_pasteAction;
    Action* m_deleteAction;
    Action* m_projectSettingsAction;
    Action* m_projectReloadResourcesAction;
    Action* m_preferencesAction;
    Action* m_newProjectAction;
    Action* m_openProjectAction;
    Action* m_saveProjectAction;
    Action* m_closeProjectAction;
    Action* m_quitAction;
    Action* m_helpQuickstartAction;
    Action* m_helpUserManualAction;
    Action* m_helpWikiPageAction;
    Action* m_sendFeedbackAction;
    Action* m_reportBugAction;
    Action* m_ceguiDebugInfoAction;
    Action* m_viewLicenseAction;
    Action* m_aboutQtAction;
    Action* m_aboutAction;
    Action* m_zoomInAction;
    Action* m_zoomOutAction;
    Action* m_zoomResetAction;
    Action* m_statusBarAction;
    Action* m_fullScreenAction;

#else
    project = property(lambda self: self._project,
                       lambda self, value: self._setProject(value))

    activeEditor = property(lambda self: self._activeEditor,
                            lambda self, value: self._setActiveEditor(value))
#endif
    void _setProject(project::Project* value);

    void _setActiveEditor(editors::TabbedEditor* value);

    MainWindow(application::Application& app);

    void setupActions();

    QMenu* m_fileMenu;
    QMenu* m_fileNewMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_docksAndToolbarsMenu;
    QMenu* m_projectMenu;
    QMenu* m_editorMenu;
    QMenu* m_tabsMenu;
    QAction* m_tabsMenuSeparator;
    QMenu* m_helpMenu;

    void setupMenus();

public slots:
    void viewMenuAboutShow();

   void viewMenuAboutHide();

    void tabsMenuAboutShow();

    void tabsMenuAboutHide();

public:

    void updateToolbarIconSize(QToolBar* toolbar, int size);
    void updateToolbarIconSizeCB(const QVariant& setting, QToolBar* tb);

    QToolBar *createToolbar(const QString& name);

    QToolBar* m_globalToolbar;
    QToolBar* m_editToolbar;
    QToolBar* m_viewToolbar;
    QToolBar* m_projectToolbar;

    void setupToolbars();

    /** Synchronises current project to the CEGUI instance
    indicateErrorsWithDialogs - if true a dialog is opened in case of errors
    Returns true if the procedure was successful
    */
    bool syncProjectToCEGUIInstance(bool indicateErrorsWithDialogs = true);

    bool performProjectDirectoriesSanityCheck(bool indicateErrorsWithDialogs = true);

    /** Opens the project file given in 'path'. Assumes no project is opened at the point this is called.
    The slot_openProject method will test if a project is opened and close it accordingly (with a dialog
    being shown if there are changes to it)

    Errors aren't indicated by exceptions or return values, dialogs are shown in case of errors.

    path - Absolute path of the project file
    openSettings - if true, the settings dialog is opened instead of just loading the resources,
                   this is desirable when creating a new project
    */
    void openProject(const QString& path, bool openSettings = false);

    /** Closes currently opened project. Assumes one is opened at the point this is called.
        */
    void closeProject();

    /** Saves currently opened project to the file it was opened from (or the last file it was saved to).
    */
    void saveProject();

    /** Saves currently opened project to a custom path. For best reliability, use absolute file path as newPath
    */
    void saveProjectAs(const QString& newPath);

    /** Creates a new editor for file at given absolutePath. This always creates a new editor,
    it is not advised to use this method directly, use openEditorTab instead.
    */
    editors::TabbedEditor* createEditorForFile(const QString& absolutePath);

    /** Activates (makes current) the tab for the path specified and
    returns true on success, otherwise false.
    */
    bool activateEditorTabByFilePath(const QString& absolutePath_);

    /** Opens editor tab. Creates new editor if such file wasn't opened yet and if it was opened,
    it just makes the tab current.
    */
    void openEditorTab(const QString& absolutePath);

    /** Closes given editor tab.

    note: Make sure you pass proper existing editor!
    */
    void closeEditorTab(editors::TabbedEditor* editor);

    editors::TabbedEditor* editorForTab(QWidget* tabWidget);

    void saveSettings();

    void restoreSettings();

    /** Safely quits the editor, prompting user to save changes to files and the project. */
    bool quit();

    /** Attempts to close all tabs that require a project opened

    This is usually done when project settings are altered and CEGUI instance has to be reloaded
    or when project is being closed and we can no longer rely on resource availability.
    */
    bool closeAllTabsRequiringProject();

    /** Queries file paths of all tabs that require a project opened

    This is used to bring previously closed tabs back up when reloading project resources
    */
    QStringList getFilePathsOfAllTabsRequiringProject();

public slots:
    void slot_newProject();

    void slot_openProject();

    bool slot_closeProject();

    void slot_projectSettings();

    void slot_projectReloadResources();

    void slot_newFileDialog(const QString& title = QStringLiteral("New File"), const QStringList& filtersList = QStringList(), int selectedFilterIndex = 0, bool autoSuffix = false);

    void slot_newLayoutDialog();

    void slot_newImagesetDialog();

    void slot_openFile(const QString& absolutePath)
    {
        openEditorTab(absolutePath);
    }

    void slot_openFileDialog();

    void slot_currentTabChanged(int index);

    bool slot_tabCloseRequested(int index);

    void slot_closeTab();

    void slot_closeOtherTabs();

    void slot_closeAllTabs();

    void slot_previousTab();

    void slot_nextTab();

    void tabBarCustomContextMenuRequested(const QPoint& point);

    void slot_save();

    void slot_saveAs();

    /** Saves all opened tabbed editors and opened project (if any)
    */
    void slot_saveAll();

    void slot_undo();

    void slot_redo();

    void slot_revert();

    void slot_cut();

    void slot_copy();

    void slot_paste();

    void slot_delete();

    void slot_zoomIn();

    void slot_zoomOut();

    void slot_zoomReset();

    void slot_about();

    void slot_license();

    void slot_openRecentFile(const QString &absolutePath);

    void slot_openRecentProject(const QString &absolutePath);

    void slot_tabsMenuActivateTab();

    void slot_toggleFullScreen();

    void slot_toggleStatusbar();

    void slot_helpQuickstart();

    void slot_helpUserManual();

    void slot_helpWikiPage();

    void slot_sendFeedback();

    void slot_reportBug();

    void slot_ceguiDebugInfo();

private:
    void closeEvent(QCloseEvent* event) override;

};

} // namespace mainwindow
} // namespace CEED

#endif
