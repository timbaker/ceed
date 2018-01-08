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

#include "mainwindow.h"

#include "paths.h"

#include "settings/settings_declaration.h"

#include "action/action__init__.h"
#include "action/declaration.h"

#include "application.h"
#include "commands.h"
#include "project.h"
//from ceed import cegui
#include "filesystembrowser.h"
#include "cegui/cegui_container.h"

#include "editors/animation_list/editor_animation_list_init.h"
#include "editors/bitmap/editor_bitmap_init.h"
#include "editors/imageset/editor_imageset_init.h"
#include "editors/layout/editor_layout_init.h"
#include "editors/looknfeel/editor_looknfeel_init.h"
#include "editors/text/editor_text_init.h"

#include "messages.h"
#include "recentlyused.h"
#include "about.h"

#include "ceed_paths.h"

#include "ui_MainWindow.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QtOpenGL/QGLFramebufferObject>
#include <QToolBar>
#include <QToolButton>

#include <functional>

namespace CEED {
namespace mainwindow {

MainWindow* MainWindow::instance = nullptr;

void MainWindow::_setProject(project::Project *value)
{
    m_project = value;
    if (m_fileSystemBrowser != nullptr)
        m_fileSystemBrowser->m_projectDirectoryButton->setEnabled(value != nullptr);
}

void MainWindow::_setActiveEditor(editors::TabbedEditor *value)
{
    m_activeEditor = value;
    if (m_fileSystemBrowser != nullptr)
        m_fileSystemBrowser->m_activeFileDirectoryButton->setEnabled(value != nullptr);
}

MainWindow::MainWindow(application::Application &app)
    : QMainWindow()
    , m_app(app)
{
    // make sure multiple instantiation won't happen
    Q_ASSERT(instance == nullptr);
    instance = this;

    // whether the app was maximized before going fullscreen
    m_wasMaximized = false;

    // we have to construct ActionManager before settings interface (as it alters the settings declaration)!
    m_actionManager = new action::ActionManager(this, m_app.m_settings);

    m_settingsInterface = new settings::interface_::QtSettingsInterface(m_app.m_settings);

    m_recentlyUsedProjects = new recentlyused::RecentlyUsedMenuEntry(m_app.m_qsettings, "Projects");
    m_recentlyUsedFiles = new recentlyused::RecentlyUsedMenuEntry(m_app.m_qsettings, "Files");

    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects()) {
        QMessageBox::warning(this, "No FBO support!",
                             "CEED uses OpenGL frame buffer objects for various tasks, "
                             "most notably to support panning and zooming in the layout editor.\n\n"
                             "FBO support was not detected on your system!\n\n"
                             "The editor will run but you may experience rendering artifacts.",
                             "no_fbo_support");
    }

    m_editorFactories += new editors::animation_list::AnimationListTabbedEditorFactory();
    m_editorFactories += new editors::bitmap::BitmapTabbedEditorFactory();
    m_editorFactories += new editors::imageset::ImagesetTabbedEditorFactory();
    m_editorFactories += new editors::layout::LayoutTabbedEditorFactory();
    m_editorFactories += new editors::looknfeel::LookNFeelTabbedEditorFactory();
    //m_editorFactories += new PropertyMappingsTabbedEditorFactory();
    m_editorFactories += new editors::text::TextTabbedEditorFactory();

    // File dialog filters, keep indices in sync with the list above
    m_editorFactoryFileFilters += QString("Animation files (*.%1)").arg(m_editorFactories[0]->getFileExtensions().toList().join(" *."));
    m_editorFactoryFileFilters += QString("Bitmap files (*.%1)").arg(m_editorFactories[1]->getFileExtensions().toList().join(" *."));
    m_editorFactoryFileFilters += QString("Imageset files (*.%1)").arg(m_editorFactories[2]->getFileExtensions().toList().join(" *."));
    m_editorFactoryFileFilters += QString("Layout files (*.%1)").arg(m_editorFactories[3]->getFileExtensions().toList().join(" *."));
    // m_editorFactoryFileFilters += QString("Property Mapping files (*. %1)").arg(m_editorFactories[4].getFileExtensions().join(" *."));
    m_editorFactoryFileFilters += QString("Text files (*.%1)").arg(m_editorFactories[4]->getFileExtensions().toList().join(" *."));

    QStringList allExt;
    for (auto* factory : m_editorFactories) {
        allExt += factory->getFileExtensions().toList();
    }
    m_editorFactoryFileFilters.insert(0, QString("All known files (*.%1)").arg(allExt.join(" *.")));
    m_editorFactoryFileFilters.insert(1, "All files (*)");

    m_activeEditor = nullptr;
    m_project = nullptr;

    m_ui = new Ui_MainWindow();
    m_ui->setupUi(this);

    // for now we can't use unified title and toolbar, it doesn't have toolbar "ellipsis"
    // and that makes the main window jump around when switching tabs
    setUnifiedTitleAndToolBarOnMac(false);

    // we start CEGUI early and we always start it
    ceguiInstance = new cegui::Instance();
    ceguiContainerWidget = new cegui::container::ContainerWidget(ceguiInstance, this);

    m_tabs = centralWidget()->findChild<QTabWidget*>("tabs");
    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::slot_currentTabChanged);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::slot_tabCloseRequested);
    // TODO: this is potentially nasty since the tabBar method is protected
    // we need this to implement the context menu when you right click on the tab bar
    m_tabBar = m_tabs->tabBar();
    m_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabBar, &QTabBar::customContextMenuRequested, this, &MainWindow::tabBarCustomContextMenuRequested);

    // stores all active tab editors
    //        self.tabEditors = []

    m_projectManager = new project::ProjectManager();
    connect(m_projectManager, &project::ProjectManager::fileOpenRequested, this, &MainWindow::slot_openFile);
    addDockWidget(Qt::RightDockWidgetArea, m_projectManager);

    m_fileSystemBrowser = new filesystembrowser::FileSystemBrowser();
    m_fileSystemBrowser->setVisible(false);
    connect(m_fileSystemBrowser, &filesystembrowser::FileSystemBrowser::fileOpenRequested, this, &MainWindow::slot_openFile);
    addDockWidget(Qt::RightDockWidgetArea, m_fileSystemBrowser);

    m_undoViewer = new commands::UndoViewer();
    m_undoViewer->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, m_undoViewer);

    //import propertytree
    //test = propertytree.TestDock()
    //self.addDockWidget(Qt.LeftDockWidgetArea, test)

    setupActions();
    setupMenus();
    setupToolbars();

    restoreSettings();
}

void MainWindow::setupActions()
{
    // usage of a connection group in mainwindow may be unnecessary,
    // we never use disconnectAll and/or connectAll, it is just used as a convenient
    // way to group connections
    m_connectionGroup =  new action::ConnectionGroup(m_actionManager);

    //
    // get and connect all actions we care about
    //
    m_newFileAction = m_actionManager->getAction("files/new_file");
    m_connectionGroup->add(m_newFileAction, [=](){ slot_newFileDialog(); });

    m_newLayoutAction = m_actionManager->getAction("files/new_layout");
    m_connectionGroup->add(m_newLayoutAction, [=](){ slot_newLayoutDialog(); });

    m_newImagesetAction = m_actionManager->getAction("files/new_imageset");
    m_connectionGroup->add(m_newImagesetAction, [=](){ slot_newImagesetDialog(); });

    m_openFileAction = m_actionManager->getAction("files/open_file");
    m_connectionGroup->add(m_openFileAction, [=](){ slot_openFileDialog(); });

    m_saveAction = m_actionManager->getAction("files/save_file");
    m_saveAction->setEnabled(false);
    m_connectionGroup->add(m_saveAction, [=](){ slot_save(); });

    m_saveAsAction = m_actionManager->getAction("files/save_file_as");
    m_saveAsAction->setEnabled(false);
    m_connectionGroup->add(m_saveAsAction, [=](){ slot_saveAs(); });

    m_saveAllAction = m_actionManager->getAction("files/save_all");
    m_connectionGroup->add(m_saveAllAction, [=](){ slot_saveAll(); });

    // tab bar context menu (but also added to the file menu so it's easy to discover)
    m_closeTabAction = m_actionManager->getAction("files/close_current_tab");
    m_closeTabAction->setEnabled(false);
    m_connectionGroup->add(m_closeTabAction, [=](){ slot_closeTab(); });

    m_closeOtherTabsAction = m_actionManager->getAction("files/close_other_tabs");
    m_closeOtherTabsAction->setEnabled(false);
    m_connectionGroup->add(m_closeOtherTabsAction, [=](){ slot_closeOtherTabs(); });

    m_closeAllTabsAction = m_actionManager->getAction("files/close_all_tabs");
    m_connectionGroup->add(m_closeAllTabsAction, [=](){ slot_closeAllTabs(); });
    // end of tab bar context menu

    m_previousTabAction = m_actionManager->getAction("files/previous_tab");
    m_connectionGroup->add(m_previousTabAction, [=](){ slot_previousTab(); });

    m_nextTabAction = m_actionManager->getAction("files/next_tab");
    m_connectionGroup->add(m_nextTabAction, [=](){ slot_nextTab(); });

    // the clear action will be handled by the RecentlyUsed manager, no need to connect
    m_clearRecentFilesAction = m_actionManager->getAction("files/clear_recent_files");

    // the clear action will be handled by the RecentlyUsed manager, no need to connect
    m_clearRecentProjectsAction = m_actionManager->getAction("files/clear_recent_projects");

    m_undoAction = m_actionManager->getAction("all_editors/undo");
    m_undoAction->setEnabled(false);
    m_connectionGroup->add(m_undoAction, [=](){ slot_undo(); });

    m_redoAction = m_actionManager->getAction("all_editors/redo");
    m_redoAction->setEnabled(false);
    m_connectionGroup->add(m_redoAction, [=](){ slot_redo(); });

    m_revertAction = m_actionManager->getAction("files/revert_file");
    m_revertAction->setEnabled(false);
    m_connectionGroup->add(m_revertAction, [=](){ slot_revert(); });

    m_cutAction = m_actionManager->getAction("all_editors/cut");
    m_connectionGroup->add(m_cutAction, [=](){ slot_cut(); });

    m_copyAction = m_actionManager->getAction("all_editors/copy");
    m_connectionGroup->add(m_copyAction, [=](){ slot_copy(); });

    m_pasteAction = m_actionManager->getAction("all_editors/paste");
    m_connectionGroup->add(m_pasteAction, [=](){ slot_paste(); });

    m_deleteAction = m_actionManager->getAction("all_editors/delete");
    m_connectionGroup->add(m_deleteAction, [=](){ slot_delete(); });

    m_projectSettingsAction = m_actionManager->getAction("project_management/project_settings");
    m_projectSettingsAction->setEnabled(false);
    m_connectionGroup->add(m_projectSettingsAction, [=](){ slot_projectSettings(); });

    m_projectReloadResourcesAction = m_actionManager->getAction("project_management/reload_resources");
    m_projectReloadResourcesAction->setEnabled(false);
    m_connectionGroup->add(m_projectReloadResourcesAction, [=](){ slot_projectReloadResources(); });

    m_preferencesAction = m_actionManager->getAction("general/application_settings");
    m_connectionGroup->add(m_preferencesAction, [=](){ m_settingsInterface->show(); });

    m_newProjectAction = m_actionManager->getAction("project_management/new_project");
    m_connectionGroup->add(m_newProjectAction, [=](){ slot_newProject(); });

    m_openProjectAction = m_actionManager->getAction("project_management/open_project");
    m_connectionGroup->add(m_openProjectAction, [=](){ slot_openProject(); });

    m_saveProjectAction = m_actionManager->getAction("project_management/save_project");
    m_saveProjectAction->setEnabled(false);
    m_connectionGroup->add(m_saveProjectAction, [=](){ saveProject(); });

    m_closeProjectAction = m_actionManager->getAction("project_management/close_project");
    m_closeProjectAction->setEnabled(false);
    m_connectionGroup->add(m_closeProjectAction, [=](){ slot_closeProject(); });

    m_quitAction = m_actionManager->getAction("general/quit");
    m_connectionGroup->add(m_quitAction, [=](){ quit(); });

    m_helpQuickstartAction = m_actionManager->getAction("general/help_quickstart");
    m_connectionGroup->add(m_helpQuickstartAction, [=](){ slot_helpQuickstart(); });

    m_helpUserManualAction = m_actionManager->getAction("general/help_user_manual");
    m_connectionGroup->add(m_helpUserManualAction, [=](){ slot_helpUserManual(); });

    m_helpWikiPageAction = m_actionManager->getAction("general/help_wiki_page");
    m_connectionGroup->add(m_helpWikiPageAction, [=](){ slot_helpWikiPage(); });

    m_sendFeedbackAction = m_actionManager->getAction("general/send_feedback");
    m_connectionGroup->add(m_sendFeedbackAction, [=](){ slot_sendFeedback(); });

    m_reportBugAction = m_actionManager->getAction("general/report_bug");
    m_connectionGroup->add(m_reportBugAction, [=](){ slot_reportBug(); });

    m_ceguiDebugInfoAction = m_actionManager->getAction("general/cegui_debug_info");
    m_connectionGroup->add(m_ceguiDebugInfoAction, [=](){ slot_ceguiDebugInfo(); });

    m_viewLicenseAction = m_actionManager->getAction("general/view_license");
    m_connectionGroup->add(m_viewLicenseAction, [=](){ slot_license(); });

    m_aboutQtAction = m_actionManager->getAction("general/about_qt");
    m_connectionGroup->add(m_aboutQtAction, []() { QApplication::aboutQt(); });

    m_aboutAction = m_actionManager->getAction("general/about");
    m_connectionGroup->add(m_aboutAction, [=](){ slot_about(); });

    m_zoomInAction = m_actionManager->getAction("all_editors/zoom_in");
    m_connectionGroup->add(m_zoomInAction, [=](){ slot_zoomIn(); });
    m_zoomOutAction = m_actionManager->getAction("all_editors/zoom_out");
    m_connectionGroup->add(m_zoomOutAction, [=](){ slot_zoomOut(); });
    m_zoomResetAction = m_actionManager->getAction("all_editors/zoom_reset");
    m_connectionGroup->add(m_zoomResetAction, [=](){ slot_zoomReset(); });

    m_statusBarAction = m_actionManager->getAction("general/statusbar");
    m_statusBarAction->setChecked(true);
    m_connectionGroup->add(m_statusBarAction, [=](){ slot_toggleStatusbar(); });

    m_fullScreenAction = m_actionManager->getAction("general/full_screen");
    m_connectionGroup->add(m_fullScreenAction, [=](){ slot_toggleFullScreen(); });

    m_connectionGroup->connectAll();
}

void MainWindow::setupMenus()
{
    //
    // Construct File menu
    //
    m_fileMenu = new QMenu("&File");
    menuBar()->addMenu(m_fileMenu);
    QMenu* menu = new QMenu("&New");
    // sorted, generic "file" last
    menu->addActions({m_newImagesetAction, m_newLayoutAction, m_newFileAction});
    menu->addSeparator();
    menu->addAction(m_newProjectAction);
    m_fileMenu->addMenu(menu);
    m_fileNewMenu = menu;
    m_fileMenu->addActions({m_openFileAction, m_openProjectAction});
    m_fileMenu->addSeparator();
    m_fileMenu->addActions({m_closeTabAction, m_closeProjectAction});
    m_fileMenu->addSeparator();
    m_fileMenu->addActions({m_saveAction, m_saveAsAction, m_saveProjectAction, m_saveAllAction});
    // TODO: Revert
    //m_fileMenu->addSeparator()
    //m_fileMenu->addAction(m_revertAction)
    m_fileMenu->addSeparator();
    menu = new QMenu("&Recent Files");
    m_fileMenu->addMenu(menu);
    using namespace std::placeholders;
    m_recentlyUsedFiles->setParentMenu(menu, std::bind(&MainWindow::slot_openRecentFile, this, _1), m_clearRecentFilesAction);
    menu = new QMenu("&Recent Projects");
    m_fileMenu->addMenu(menu);
    m_recentlyUsedProjects->setParentMenu(menu, std::bind(&MainWindow::slot_openRecentProject, this, _1), m_clearRecentProjectsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    //
    // Construct Edit menu
    //
    m_editMenu = new QMenu("&Edit");
    menuBar()->addMenu(m_editMenu);
    m_editMenu->addActions({m_undoAction, m_redoAction});
    m_editMenu->addAction(m_revertAction);
    m_editMenu->addSeparator();
    m_editMenu->addActions({m_cutAction, m_copyAction, m_pasteAction, m_deleteAction});
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_preferencesAction);

    //
    // Construct View menu
    //
    m_viewMenu = new QMenu("&View");
    menuBar()->addMenu(m_viewMenu);

    // the first menu item of the view menu is "Docks & Toolbars"
    // this menu contains checkable "actions" that hide and show the panels
    // even though you can achieve the same thing by right clicking empty space in
    // mainwindow I believe having this has a benefit, it is much easier to find this way
    // this menu is created dynamically when the view menu is about to be shown and we
    // rely on the default implementation to create it.
    // the other option would be to create the menu by iterating all docks and toolbars,
    // like: https://qt.gitorious.org/qt/qt/blobs/4.8/src/gui/widgets/qmainwindow.cpp#line1643
    m_docksAndToolbarsMenu = nullptr;
    connect(m_viewMenu, SIGNAL(aboutToShow()), this, SLOT(viewMenuAboutShow()));
    connect(m_viewMenu, SIGNAL(aboutToHide()), this, SLOT(viewMenuAboutHide()));

    // add the rest of the view menu items
    m_viewMenu->addAction(m_statusBarAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addActions({m_zoomInAction, m_zoomOutAction, m_zoomResetAction});
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_fullScreenAction);

    //
    // Construct Project menu
    //
    m_projectMenu = new QMenu("&Project");
    menuBar()->addMenu(m_projectMenu);
    m_projectMenu->addAction(m_projectReloadResourcesAction);
    m_projectMenu->addSeparator();
    m_projectMenu->addAction(m_projectSettingsAction);

    //
    // Construct active editor menu
    // This is disabled & hidden by default
    // and it's managed by the active editor
    //
    m_editorMenu = new QMenu("EditorMenu");
    m_editorMenu->menuAction()->setEnabled(false);
    m_editorMenu->menuAction()->setVisible(false);
    menuBar()->addMenu(m_editorMenu);

    //
    // Construct Tabs menu
    //
    m_tabsMenu = new QMenu("&Tabs");
    menuBar()->addMenu(m_tabsMenu);
    m_tabsMenu->addActions({m_previousTabAction, m_nextTabAction});
    m_tabsMenu->addSeparator();
    m_tabsMenu->addActions({m_closeOtherTabsAction, m_closeAllTabsAction});
    // add menu items for open tabs
    m_tabsMenuSeparator = nullptr;
    connect(m_tabsMenu, SIGNAL(aboutToShow()), this, SLOT(tabsMenuAboutShow()));
    connect(m_tabsMenu, SIGNAL(aboutToHide()), this, SLOT(tabsMenuAboutHide()));

    //
    // Construct Help menu
    //
    m_helpMenu = new QMenu("&Help");
    menuBar()->addMenu(m_helpMenu);
    m_helpMenu->addAction(m_helpQuickstartAction);
    m_helpMenu->addAction(m_helpUserManualAction);
    m_helpMenu->addAction(m_helpWikiPageAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addActions({m_sendFeedbackAction, m_reportBugAction, m_ceguiDebugInfoAction});
    m_helpMenu->addSeparator();
    m_helpMenu->addActions({m_viewLicenseAction, m_aboutQtAction});
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::viewMenuAboutShow()
{
#if 1
    // Tried doing this in viewMenuAboutHide()... not good
    if (m_docksAndToolbarsMenu != nullptr) {
        delete m_docksAndToolbarsMenu;
        m_docksAndToolbarsMenu = nullptr;
    }
#endif
    // call super to be safe in case we change our own createPopupMenu.
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->setTitle("&Docks && Toolbars");
    // add before the statusbar action
    m_viewMenu->insertMenu(m_statusBarAction, menu);
    // keep reference so we can remove on hide
    m_docksAndToolbarsMenu = menu;
}

void MainWindow::viewMenuAboutHide()
{
    m_viewMenu->removeAction(m_docksAndToolbarsMenu->menuAction());
}

void MainWindow::tabsMenuAboutShow()
{
    m_tabsMenuSeparator = m_tabsMenu->addSeparator();
    QList<QAction*> actions;
    int counter = 1;
    // the items are taken from the 'm_tabEditors' list
    // which always has the order by which the files were
    // opened (not the order of the tabs in the tab bar).
    // this is a feature, maintains the same mnemonic
    // even if a tab is moved.
    for (editors::TabbedEditor* editor : m_tabEditors) {
        // construct the label from the filePath
        QString name = editor->m_filePath;
        // TODO: the next few lines are basically the
        // same as the code in recentlyused. refactor
        // so both places use the same (generic) code.

        // if name length greater than some value, trim
        if (name.length() > 40) {
            name = QString("...%1").arg(name.remove(0, name.length() - 40));
        }
        // if the first 10 tabs, add mnemonic (1 to 9, then 0)
        if (counter <= 10) {
            name = QString("&%1. %2").arg(counter % 10).arg(name);
        }
        counter += 1;
        // create the action object
        auto action = new QAction(name, m_tabsMenu);
        action->setData(editor->m_filePath);
        connect(action, &QAction::triggered, this, &MainWindow::slot_tabsMenuActivateTab);
        actions.append(action);
    }
    // add all to menu
    m_tabsMenu->addActions(actions);
}

void MainWindow::tabsMenuAboutHide()
{
    // remove all from the separator to the end
    QList<QAction*> actions = m_tabsMenu->actions();
    int index = actions.indexOf(m_tabsMenuSeparator);
    for (int i = index; i < actions.length(); i++) {
        m_tabsMenu->removeAction(actions[i]);
    }
    m_tabsMenuSeparator = nullptr;
}

void MainWindow::updateToolbarIconSize(QToolBar *toolbar, int size)
{
    if (size < 16) {
        size = 16;
    }
    toolbar->setIconSize(QSize(size, size));
}

void MainWindow::updateToolbarIconSizeCB(const QVariant &setting, QToolBar* tb)
{
    updateToolbarIconSize(tb, setting.toInt());
}

QToolBar* MainWindow::createToolbar(const QString &name)
{
    QToolBar* tb = new QToolBar(name, this);
    tb->setObjectName(QString("%1 toolbar").arg(name));
    Entry* tbIconSizeEntry = m_app.m_settings->getEntry("global/ui/toolbar_icon_size");

    updateToolbarIconSize(tb, tbIconSizeEntry->m_value.toInt());
    auto cb = [=](const QVariant& setting) -> void
    {
        updateToolbarIconSizeCB(setting, tb);
    };
    tbIconSizeEntry->subscribe(std::bind(&MainWindow::updateToolbarIconSizeCB, this, std::placeholders::_1, tb));
    addToolBar(tb);
    return tb;
}

void MainWindow::setupToolbars()
{
    //
    // Standard toolbar
    //
    QToolBar* tbar;
    tbar = m_globalToolbar = createToolbar("Standard");
    QToolButton* newMenuButton = new QToolButton(tbar);
    newMenuButton->setText("New");
    newMenuButton->setToolTip("New File");
    newMenuButton->setIcon(QIcon(":icons/actions/new_file.png"));
    newMenuButton->setMenu(m_fileNewMenu);
    newMenuButton->setPopupMode(QToolButton::InstantPopup);
    tbar->addWidget(newMenuButton);
    tbar->addActions({m_openFileAction, m_openProjectAction});
    tbar->addSeparator();
    tbar->addActions({m_saveAction, m_saveAsAction, m_saveProjectAction, m_saveAllAction});
    // The menubutton does not resize its icon correctly unless we tell it to do so
    connect(tbar, SIGNAL(iconSizeChanged(QSize)), newMenuButton, SLOT(setIconSize(QSize)));

    //
    // Edit toolbar
    //
    tbar = m_editToolbar = createToolbar("Edit");
    tbar->addActions({m_undoAction, m_redoAction});
    tbar->addSeparator();
    tbar->addActions({m_cutAction, m_copyAction, m_pasteAction, m_deleteAction});

    //
    // View toolbar
    //
    tbar = m_viewToolbar = createToolbar("View");
    tbar->addActions({m_zoomInAction, m_zoomOutAction, m_zoomResetAction});
    tbar->addSeparator();
    tbar->addAction(m_fullScreenAction);

    //
    // Project toolbar
    //
    tbar = m_projectToolbar = createToolbar("Project");
    tbar->addAction(m_projectReloadResourcesAction);
    tbar->addAction(m_projectSettingsAction);
}

bool MainWindow::syncProjectToCEGUIInstance(bool indicateErrorsWithDialogs)
{
    try {
        ceguiInstance->syncToProject(m_project, this);

        return true;

    } catch (Exception e) {
        if (indicateErrorsWithDialogs) {
            QMessageBox::warning(this, QStringLiteral("Failed to synchronise embedded CEGUI to your project"),
                                 QString("An attempt was made to load resources related to the project being opened, \
                                         for some reason the loading didn't succeed so all resources were destroyed!\
                                         The most likely reason is that the resource directories are wrong,\
                                         this can be very easily remedied in the project settings.\
                                         \
                                         This means that editing capabilities of CEED will be limited to editing of files that don't require a project opened (for example: imagesets).\
                                         \
                                         Details of this error: %1").arg(QString::fromUtf8(e.what())));
        }
        return false;
    }
}

bool MainWindow::performProjectDirectoriesSanityCheck(bool indicateErrorsWithDialogs)
{
    try {
        m_project->checkAllDirectories();

        return true;

    } catch (IOError e) {
        if (indicateErrorsWithDialogs) {
            QMessageBox::warning(this, QStringLiteral("At least one of project's resource directories is invalid"),
                                 QString("Project's resource directory paths didn't pass the sanity check, please check projects settings.\
                                         \
                                         Details of this error: %1").arg(QString::fromUtf8(e.what())));

                                         return false;
        }
        return false;
    }
}

void MainWindow::openProject(const QString &path, bool openSettings)
{
    Q_ASSERT(m_project == nullptr);

    // reset project manager to a clean state just in case
    m_projectManager->setProject(nullptr);

    try {
        m_project = new project::Project();
        m_project->load(path);
    } catch (IOError e) {
        QMessageBox::critical(this, QStringLiteral("Error when opening project"),
                              QString("It seems project at path '%1' doesn't exist or you don't have rights to open it.").arg(path));
        delete m_project;
        m_project = nullptr;
        return;
    }

    performProjectDirectoriesSanityCheck();

    // view the newly opened project in the project manager
    m_projectManager->setProject(m_project);
    // and set the filesystem browser path to the base folder of the project
    // TODO: Maybe this could be configurable?
    QString projectBaseDirectory = m_project->getAbsolutePathOf("");
    if (QDir(projectBaseDirectory).exists()) {
        m_fileSystemBrowser->setDirectory(projectBaseDirectory);
    }

    m_recentlyUsedProjects->addRecentlyUsed(m_project->m_projectFilePath);

    // and enable respective actions
    m_saveProjectAction->setEnabled(true);
    m_closeProjectAction->setEnabled(true);
    m_projectSettingsAction->setEnabled(true);
    m_projectReloadResourcesAction->setEnabled(true);

    if (openSettings) {
        slot_projectSettings();
    } else {
        syncProjectToCEGUIInstance();
    }
}

void MainWindow::closeProject()
{
    Q_ASSERT(m_project != nullptr);

    // since we are effectively unloading the project and potentially nuking resources of it
    // we should definitely unload all tabs that rely on it to prevent segfaults and other
    // nasty phenomena
    if (!closeAllTabsRequiringProject()) {
        return;
    }

    m_projectManager->setProject(nullptr);
    // TODO: Do we really want to call this there? This was already called when the project was being opened.
    //       It doesn't do anything harmful but maybe is unnecessary.
    m_recentlyUsedProjects->addRecentlyUsed(m_project->m_projectFilePath);
    // clean resources that were potentially used with this project
    ceguiInstance->cleanCEGUIResources();

    m_project->unload();
    delete m_project;
    m_project = nullptr;

    // as the project was closed be will disable actions related to it
    m_saveProjectAction->setEnabled(false);
    m_closeProjectAction->setEnabled(false);
    m_projectSettingsAction->setEnabled(false);
    m_projectReloadResourcesAction->setEnabled(false);
}

void MainWindow::saveProject()
{
    Q_ASSERT(m_project != nullptr);

    m_project->save();
}

void MainWindow::saveProjectAs(const QString &newPath)
{
    m_project->save(newPath);
    // set the project's file path to newPath so that if you press save next time it will save to the new path
    // (This is what is expected from applications in general I think)
    m_project->m_projectFilePath = newPath;
}

editors::TabbedEditor* MainWindow::createEditorForFile(const QString &absolutePath)
{
    editors::TabbedEditor* ret = nullptr;

    QString projectRelativePath = QStringLiteral("N/A");
    try {
        if (m_project != nullptr) {
            projectRelativePath = os.path.relpath(absolutePath, m_project->getAbsolutePathOf(""));
        } else {
            projectRelativePath = QStringLiteral("<No project opened>");
        }
    } catch (...) {
    }

    if (!QFileInfo(absolutePath).exists()) {
        ret = new editors::MessageTabbedEditor(absolutePath,
                                               QString("Couldn't find '%1' (project relative path: '%2'), please check that that your project's "
                                                       "base directory is set up correctly and that you hadn't deleted "
                                                       "the file from your HDD. Consider removing the file from the project.").arg(absolutePath).arg(projectRelativePath));
    } else {
        QList<editors::TabbedEditorFactory*> possibleFactories;

        for (editors::TabbedEditorFactory* factory : m_editorFactories) {
            if (factory->canEditFile(absolutePath)) {
                possibleFactories.append(factory);
            }
        }

        // at this point if possibleFactories is [], no registered tabbed editor factory wanted
        // to accept the file, so we create MessageTabbedEditor that will simply
        // tell the user that given file can't be edited
        //
        // IMO this is a reasonable compromise and plays well with the rest of
        // the editor without introducing exceptions, etc...
        if (possibleFactories.isEmpty()) {
            if (absolutePath.endsWith(".project")) {
                // provide a more newbie-friendly message in case they are
                // trying to open a project file as if it were a file
                ret = new editors::MessageTabbedEditor(absolutePath,
                                                       QString("You are trying to open '%1' (project relative path: '%2') which "
                                                               "seems to be a CEED project file. "
                                                               "This simply is not how things are supposed to work, please use "
                                                               "File -> Open Project to open your project file instead. "
                                                               "(CEED enforces proper extensions)").arg(absolutePath).arg(projectRelativePath));
            } else {
                ret = new editors::MessageTabbedEditor(absolutePath,
                                                       QString("No included tabbed editor was able to accept '%1' "
                                                               "(project relative path: '%2'), please check that it's a file CEED "
                                                               "supports and that it has the correct extension "
                                                               "(CEED enforces proper extensions)")
                                                       .arg(absolutePath)
                                                       .arg(projectRelativePath));
            }
        } else {
            // one or more factories wants to accept the file
            editors::TabbedEditorFactory* factory = nullptr;

            if (possibleFactories.length() == 1) {
                // it's decided, just one factory wants to accept the file
                factory = possibleFactories[0];

            } else {
                Q_ASSERT(possibleFactories.length() > 1);

                // more than 1 factory wants to accept the file, offer a dialog and let user choose
                editors::MultiplePossibleFactoriesDialog dialog(possibleFactories);
                int result = dialog.exec();

                if (result == QDialog::Accepted) {
                    factory = dialog.selectedFactory();
                }
            }
            if (factory == nullptr) {
                ret = new editors::MessageTabbedEditor(absolutePath,
                                                       QString("You failed to choose an editor to open '%1' with (project relative path: '%2').")
                                                       .arg(absolutePath)
                                                       .arg(projectRelativePath));
            } else {
                ret = factory->create(absolutePath);
            }
        }
    }

    if (m_project == nullptr && ret->m_requiresProject) {
        delete ret;
        ret = new editors::MessageTabbedEditor(absolutePath,
                                               QStringLiteral("Opening this file requires you to have a project opened!"));
    }
    try {
        ret->initialise(this);

        // add successfully opened file to the recent files list
        m_recentlyUsedFiles->addRecentlyUsed(absolutePath);

    } catch (...) {
        // it may have been partly constructed at this point
        try {
            // make sure the finalisation doesn't early out or fail assertion
            ret->m_initialised = true;

            ret->finalise();
            ret->destroy();

        } catch (...) {
            // catch all exception the finalisation raises (we can't deal with them anyways)
        }

        // Can't throw out of Qt signal
//        throw;
    }

//    m_tabEditors.append(ret);
    return ret;
}

bool MainWindow::activateEditorTabByFilePath(const QString &absolutePath_)
{
    QString absolutePath = QFileInfo(absolutePath_).canonicalFilePath();

    for (editors::TabbedEditor* tabEditor : m_tabEditors) {
        if (tabEditor->m_filePath == absolutePath) {
            tabEditor->makeCurrent();
            return true;
        }
    }
    return false;
}

void MainWindow::openEditorTab(const QString &absolutePath)
{
    if (activateEditorTabByFilePath(absolutePath)) {
        return;
    }

    editors::TabbedEditor* editor = createEditorForFile(absolutePath);
    editor->makeCurrent();
}

void MainWindow::closeEditorTab(editors::TabbedEditor *editor)
{
    Q_ASSERT(m_tabEditors.contains(editor));

    editor->finalise();
    editor->destroy();

    m_tabEditors.removeOne(editor);
}

editors::TabbedEditor *MainWindow::editorForTab(QWidget *tabWidget)
{
    for (auto editor : m_tabEditors) {
        if (editor->m_tabWidget == tabWidget)
            return editor;
    }
    Q_ASSERT(false);
    return nullptr;
}

void MainWindow::saveSettings()
{
    /** Saves geometry and state of this window to QSettings.
        */

    try {
        m_app.m_qsettings->setValue("window-geometry", saveGeometry());
        m_app.m_qsettings->setValue("window-state", saveState());
    } catch (...) {
        // we don't really care if this fails
    }
}

void MainWindow::restoreSettings()
{
    /** Restores geometry and state of this window from QSettings.
        */

    try {
        if (m_app.m_qsettings->contains("window-geometry")) {
            restoreGeometry(m_app.m_qsettings->value("window-geometry").toByteArray());
        }
        if (m_app.m_qsettings->contains("window-state")) {
            restoreState(m_app.m_qsettings->value("window-state").toByteArray());
        }
    } catch (...) {
        // we don't really care if this fails
    }
}

bool MainWindow::quit()
{
    saveSettings();

    // we remember last tab we closed to check whether user pressed Cancel in any of the dialogs
    editors::TabbedEditor* lastTab = nullptr;
    while (!m_tabEditors.isEmpty()) {
        auto currentTab = editorForTab(m_tabs->widget(0));
        if (currentTab == lastTab) {
            // user pressed cancel on one of the tab editor 'save without changes' dialog,
            // cancel the whole quit operation!
            return false;
        }
        lastTab = currentTab;

        slot_tabCloseRequested(0);
    }
    // Close project after all tabs have been closed, there may be tabs requiring a project opened!
    if (m_project != nullptr) {
        // if the slot returned false, user pressed Cancel
        if (!slot_closeProject()) {
            // in case user pressed cancel the entire quitting processed has to be terminated
            return false;
        }
    }

    m_app.quit();
    return true;
}

bool MainWindow::closeAllTabsRequiringProject()
{
    int i = 0;
    while (i < m_tabs->count()) {
        editors::TabbedEditor* tabbedEditor = editorForTab(m_tabs->widget(i));

        if (tabbedEditor->m_requiresProject) {
            if (!slot_tabCloseRequested(i)) {
                // if the method returns false user pressed Cancel so in that case
                // we cancel the entire operation
                return false;
            }
            continue;
        }
        i += 1;
    }
    return true;
}

QStringList MainWindow::getFilePathsOfAllTabsRequiringProject()
{
    QStringList ret;
    int i = 0;
    while (i < m_tabs->count()) {
        editors::TabbedEditor* tabbedEditor = editorForTab(m_tabs->widget(i));

        if (tabbedEditor->m_requiresProject) {
            ret.append(tabbedEditor->m_filePath);
        }

        i += 1;
    }

    return ret;
}

void MainWindow::slot_newProject()
{
    if (m_project != nullptr) {
        // another project is already opened!
        int result = QMessageBox::question(this,
                                           QStringLiteral("Another project already opened!"),
                                           QStringLiteral("Before creating a new project, you must close the one currently opened. "
                                                          "Do you want to close currently opened project? (all unsaved changes will be lost!)"),
                                           QMessageBox::Yes | QMessageBox::Cancel,
                                           QMessageBox::Cancel);

        if (result != QMessageBox::Yes) {
            return;
        }

        // don't close the project yet, close it after the user
        // accepts the New Project dialog below because they may cancel
    }

    project::NewProjectDialog newProjectDialog;
    if (newProjectDialog.exec() != QDialog::Accepted) {
        return;
    }

    // The dialog was accepted, close any open project
    if (m_project != nullptr) {
        closeProject();
    }

    project::Project* newProject = newProjectDialog.createProject();
    newProject->save();

    // open the settings window after creation so that user can further customise their
    // new project file
    openProject(newProject->m_projectFilePath, true);

    // save the project with the settings that were potentially set in the project settings dialog
    saveProject();
}

void MainWindow::slot_openProject()
{
    if (m_project != nullptr) {
        // another project is already opened!
        int result = QMessageBox::question(this,
                                           QStringLiteral("Another project already opened!"),
                                           QStringLiteral("Before creating a new project, you must close the one currently opened. "
                                                          "Do you want to close currently opened project? (all unsaved changes will be lost!)"),
                                           QMessageBox::Yes | QMessageBox::Cancel,
                                           QMessageBox::Cancel);

        if (result == QMessageBox::Yes) {
            closeProject();
        } else {
            // User selected cancel, NOOP
            return;
        }
    }

    QString file = QFileDialog::getOpenFileName(this,
                                                QStringLiteral("Open existing project file"),
                                                QStringLiteral(""),
                                                QStringLiteral("Project files (*.project)"));

    if (!file.isEmpty()) {
        // user actually selected something ;-)

        openProject(file);
    }
}

bool MainWindow::slot_closeProject()
{
    Q_ASSERT(m_project != nullptr);

    if (m_project->hasChanges()) {
        int result = QMessageBox::question(this,
                                           QStringLiteral("Project file has changes!"),
                                           QStringLiteral("There are unsaved changes in the project file "
                                                          "Do you want to save them? "
                                                          "(Pressing Discard will discard the changes!)"),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                           QMessageBox::Save);

        if (result == QMessageBox::Save) {
            saveProject();
        } else if (result == QMessageBox::Cancel) {
            return false;
        }
    }

    closeProject();
    return true;
}

void MainWindow::slot_projectSettings()
{
    // since we are effectively unloading the project and potentially nuking resources of it
    // we should definitely unload all tabs that rely on it to prevent segfaults and other
    // nasty phenomena
    if (!closeAllTabsRequiringProject()) {
        QMessageBox::information(this,
                                 QStringLiteral("Project dependent tabs still open!"),
                                 QStringLiteral("You can't alter project's settings while having tabs that "
                                                "depend on the project and its resources opened!"));
        return;
    }

    project::ProjectSettingsDialog dialog(m_project);

    if (dialog.exec() == QDialog::Accepted) {
        dialog.apply(m_project);
        performProjectDirectoriesSanityCheck();
        syncProjectToCEGUIInstance();
    }
}

void MainWindow::slot_projectReloadResources()
{
    // since we are effectively unloading the project and potentially nuking resources of it
    // we should definitely unload all tabs that rely on it to prevent segfaults and other
    // nasty phenomena

    // we will remember previously opened tabs requiring a project so that we can load them up
    // after we are done
    QStringList filePathsToLoad = getFilePathsOfAllTabsRequiringProject();
    QString activeEditorPath = m_activeEditor ? m_activeEditor->m_filePath : QString();

    if (!closeAllTabsRequiringProject()) {
        QMessageBox::information(this,
                                 QStringLiteral("Project dependent tabs still open!"),
                                 QStringLiteral("You can't reload project's resources while having tabs that "
                                                "depend on the project and its resources opened!"));
        return;
    }

    performProjectDirectoriesSanityCheck();
    syncProjectToCEGUIInstance();

    // load previously loaded tabs requiring a project opened
    for (QString& filePath : filePathsToLoad) {
        openEditorTab(filePath);
    }

    // previously active editor to be loaded last, this makes it active again
    if (!activeEditorPath.isEmpty()) {
        openEditorTab(activeEditorPath);
    }
}

void MainWindow::slot_newFileDialog(const QString &title, const QStringList &filtersList, int selectedFilterIndex, bool autoSuffix)
{
    QString defaultDir;
    if (m_project != nullptr) {
        defaultDir = m_project->getAbsolutePathOf("");
    }
    // Qt (as of 4.8) does not support default suffix (extension) unless you use
    // non-native file dialogs with non-static methods (see QFileDialog.setDefaultSuffix).
    // HACK: We handle this differently depending on whether a default suffix is required

    if (filtersList.isEmpty() || !autoSuffix) {
        QString selectedFilter = (filtersList.length() > selectedFilterIndex) ? filtersList[selectedFilterIndex] : QString();
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        title,
                                                        defaultDir,
                                                        filtersList.join(";;"),
                                                        &selectedFilter);
        if (!fileName.isEmpty()) {
            try {
                QFile file(fileName);
                if (file.open(QFile::WriteOnly))
                    file.close();
                else
                    throw IOError(QString("QFile::open() returned false"));
            } catch (IOError e) {
                QMessageBox::critical(this, QStringLiteral("Error creating file!"),
                                      QString("CEED encountered an error trying to create a new file.\n\n(exception details: %1)").arg(QString::fromUtf8(e.what())));
                return;
            }

            openEditorTab(fileName);
        }
    } else {
        while (true) {
            QString selectedFilter = (filtersList.length() > selectedFilterIndex) ? filtersList[selectedFilterIndex] : QString();
            QString fileName = QFileDialog::getSaveFileName(this,
                                                            title,
                                                            defaultDir,
                                                            filtersList.join(";;"),
                                                            &selectedFilter,
                                                            QFileDialog::DontConfirmOverwrite);
            if (fileName.isEmpty()) {
                break;
            }

            // if there is no dot, append the selected filter's extension
            if (fileName.indexOf('.') == -1) {
                // really ugly, handle with care
                // find last open paren
                int i = selectedFilter.lastIndexOf('(');
                if (i != -1) {
                    // find next dot
                    i = selectedFilter.indexOf('.', i);
                    if (i != -1) {
                        // find next space or close paren
                        int k = selectedFilter.indexOf(')', i);
                        int l = selectedFilter.indexOf(' ', i);
                        if (l != -1 && l < k)
                            k = l;
                        if (k != -1) {
                            QString selectedExt = selectedFilter.mid(i, k - 1);
                            if (selectedExt.indexOf('*') == -1 && selectedExt.indexOf('?') == -1) {
                                fileName += selectedExt;
                            }
                        }
                    }
                }
            }

            // and now test & confirm overwrite
            try {
                if (QFileInfo(fileName).exists()) {
                    QMessageBox* msgBox = new QMessageBox(this);
                    msgBox->setText(QString("A file named \"%1\" already exists in \"%2\".").arg(QFileInfo(fileName).fileName(), QFileInfo(fileName).absolutePath()));
                    msgBox->setInformativeText("Do you want to replace it, overwriting its contents?");
                    msgBox->addButton(QMessageBox::Cancel);
                    auto replaceButton = msgBox->addButton("&Replace", QMessageBox::YesRole);
                    msgBox->setDefaultButton(replaceButton);
                    msgBox->setIcon(QMessageBox::Question);
                    msgBox->exec();
                    if (msgBox->clickedButton() != replaceButton) {
                        continue;
                    }
                }
                QFile file(fileName);
                if (file.open(QFile::WriteOnly))
                    file.close();
                else
                    throw IOError(QString("QFile::open returned false"));
            } catch (IOError e) {
                QMessageBox::critical(this, QStringLiteral("Error creating file!"),
                                      QString("CEED encountered an error trying to create a new file.\n\n(exception details: %1)").arg(QString::fromUtf8(e.what())));
                return;
            }

            openEditorTab(fileName);
            break;
        }
    }
}

void MainWindow::slot_newLayoutDialog()
{
    slot_newFileDialog("New Layout",
    { "Layout files (*.layout)" },
                       0,
                       true);
}

void MainWindow::slot_newImagesetDialog()
{
    slot_newFileDialog("New Imageset",
    {"Imageset files (*.imageset)"},
                       0,
                       true);
}

void MainWindow::slot_openFileDialog()
{
    QString defaultDir;
    if (m_project != nullptr) {
        defaultDir = m_project->getAbsolutePathOf("");
    }

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    defaultDir,
                                                    m_editorFactoryFileFilters.join(";;"),
                                                    &m_editorFactoryFileFilters[0]);
    if (!fileName.isEmpty()) {
        openEditorTab(fileName);
    }
}

void MainWindow::slot_currentTabChanged(int index)
{
    // to fight flicker
    m_tabs->setUpdatesEnabled(false);

    QWidget* wdt = m_tabs->widget(index);

    if (m_activeEditor != nullptr) {
        m_activeEditor->deactivate();
    }

    // it's the tabbed editor's responsibility to handle these,
    // we disable them by default
    m_undoAction->setEnabled(false);
    m_redoAction->setEnabled(false);
    // also reset their texts in case the tabbed editor messed with them
    m_undoAction->setText("Undo");
    m_redoAction->setText("Redo");
    // set undo stack to None as we have no idea whether the previous tab editor
    // set it to something else
    m_undoViewer->setUndoStack(nullptr);

    // we also clear the status bar
    statusBar()->clearMessage();

    if (wdt != nullptr) {
        m_revertAction->setEnabled(true);

        m_saveAction->setEnabled(true);
        m_saveAsAction->setEnabled(true);

        m_closeTabAction->setEnabled(true);
        m_closeOtherTabsAction->setEnabled(true);

        editorForTab(wdt)->activate();
    } else {
        // None is selected right now, lets disable appropriate actions
        m_revertAction->setEnabled(false);

        m_saveAction->setEnabled(false);
        m_saveAsAction->setEnabled(false);

        m_closeTabAction->setEnabled(false);
        m_closeOtherTabsAction->setEnabled(false);
    }

    m_tabs->setUpdatesEnabled(true);
}

bool MainWindow::slot_tabCloseRequested(int index)
{
    QWidget* wdt = m_tabs->widget(index);
    editors::TabbedEditor* editor = editorForTab(wdt);

    if (!editor->hasChanges()) {
        // we can close immediately
        closeEditorTab(editor);
        return true;

    } else {
        // we have changes, lets ask the user whether we should dump them or save them
        int result = QMessageBox::question(this,
                                           "Unsaved changes!",
                                           QString("There are unsaved changes in '%1'. "
                                           "Do you want to save them? "
                                           "(Pressing Discard will discard the changes!)").arg(editor->m_filePath),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                           QMessageBox::Save);

        if (result == QMessageBox::Save) {
            // lets save changes and then kill the editor (This is the default action)
            // If there was an error saving the file, stop what we're doing
            // and let the user fix the problem.
            if (!editor->save()) {
                return false;
            }

            closeEditorTab(editor);
            return true;
        }

        else if (result == QMessageBox::Discard) {
            // changes will be discarded
            // note: we don't have to call editor.discardChanges here

            closeEditorTab(editor);
            return true;
        }

        // don't do anything if user selected 'Cancel'
        return false;
    }
}

void MainWindow::slot_closeTab()
{
    // a simple delegate
    slot_tabCloseRequested(m_tabs->currentIndex());
}

void MainWindow::slot_closeOtherTabs()
{
    QWidget* current = m_tabs->currentWidget();

    int i = 0;
    while (i < m_tabs->count()) {
        if (m_tabs->widget(i) == current) {
            // we skip the current widget
            i += 1;
        } else {
            if (!slot_tabCloseRequested(i)) {
                // user selected Cancel, we skip this widget
                i += 1;
            }
        }
    }
}

void MainWindow::slot_closeAllTabs()
{
    int i = 0;
    while (i < m_tabs->count()) {
        if (!slot_tabCloseRequested(i)) {
            // user selected Cancel, we skip this widget
            i += 1;
        }
    }
}

void MainWindow::slot_previousTab()
{
    if (m_tabs->count() <= 1) {
        return;
    }
    int index = m_tabs->currentIndex() - 1;
    if (index < 0) {
        index = m_tabs->count() + index;
    }
    m_tabs->setCurrentIndex(index);
}

void MainWindow::slot_nextTab()
{
    if (m_tabs->count() <= 1) {
        return;
    }
    int index = (m_tabs->currentIndex() + 1) % m_tabs->count();
    m_tabs->setCurrentIndex(index);
}

void MainWindow::tabBarCustomContextMenuRequested(const QPoint &point)
{
    int atIndex = m_tabBar->tabAt(point);
    m_tabs->setCurrentIndex(atIndex);

    QMenu* menu = new QMenu(this);
    menu->addAction(m_closeTabAction);
    menu->addSeparator();
    menu->addAction(m_closeOtherTabsAction);
    menu->addAction(m_closeAllTabsAction);

    QAction* dataTypeAction = nullptr;
    if (atIndex != -1) {
        QWidget* tab = m_tabs->widget(atIndex);
        auto editor = editorForTab(tab);
        menu->addSeparator();
        dataTypeAction = new QAction(QString("Data type: %1").arg(editor->getDesiredSavingDataType()), this);
        dataTypeAction->setToolTip("Displays which data type this file will be saved to (the desired saving data type).");
        menu->addAction(dataTypeAction);
    }

    menu->exec(m_tabBar->mapToGlobal(point));

    delete menu;
    delete dataTypeAction;
}

void MainWindow::slot_save()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->save();
    }
}

void MainWindow::slot_saveAs()
{
    if (m_activeEditor != nullptr) {
        QString filePath = QFileDialog::getSaveFileName(this, "Save as", QFileInfo(m_activeEditor->m_filePath).absolutePath());
        if (!filePath.isEmpty()) { // make sure user hasn't cancelled the dialog
            m_activeEditor->saveAs(filePath);
        }
    }
}

void MainWindow::slot_saveAll()
{
    if (m_project != nullptr) {
        m_project->save();
    }

    for (editors::TabbedEditor* editor : m_tabEditors) {
        editor->save();
    }
}

void MainWindow::slot_undo()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->undo();
    }
}

void MainWindow::slot_redo()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->redo();
    }
}

void MainWindow::slot_revert()
{
    if (m_activeEditor != nullptr) {
        int ret = QMessageBox::question(this,
                                        "Are you sure you want to revert to file on hard disk?",
                                        "Reverting means that the file will be reloaded to the "
                                        "state it is in on the HDD.\n\nRevert?\n\n"
                                        "If you select Yes, ALL UNDO HISTORY MIGHT BE DESTROYED!\n"
                                        "(though I will preserve it if possible)",
                                        QMessageBox::No | QMessageBox::Yes,
                                        QMessageBox::No); // defaulting to No is safer IMO

        if (ret == QMessageBox::Yes) {
            m_activeEditor->revert();
        } else if (ret == QMessageBox::No) {
            // user chickened out
        } else {
            // how did we get here?
            Q_ASSERT(false);
        }
    }
}

void MainWindow::slot_cut()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->performCut();
    }
}

void MainWindow::slot_copy()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->performCopy();
    }
}

void MainWindow::slot_paste()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->performPaste();
    }
}

void MainWindow::slot_delete()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->performDelete();
    }
}

void MainWindow::slot_zoomIn()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->zoomIn();
    }
}

void MainWindow::slot_zoomOut()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->zoomOut();
    }
}

void MainWindow::slot_zoomReset()
{
    if (m_activeEditor != nullptr) {
        m_activeEditor->zoomReset();
    }
}

void MainWindow::slot_about()
{
    about::AboutDialog dialog;
    dialog.exec();
}

void MainWindow::slot_license()
{
    about::LicenseDialog dialog;
    dialog.exec();
}

void MainWindow::slot_openRecentFile(const QString& absolutePath)
{
    /*if (auto action = dynamic_cast<QAction*>(sender()))*/ {
//        QString absolutePath = action->data().toString();
        if (QFileInfo(absolutePath).exists())
            openEditorTab(absolutePath);
        else {
            QMessageBox* msgBox = new QMessageBox(this);
            msgBox->setText(QString("File \"%1\" was not found.").arg(absolutePath));
            msgBox->setInformativeText("The file does not exist; it may have been moved or deleted."
                                       " Do you want to remove it from the recently used list?");
            msgBox->addButton(QMessageBox::Cancel);
            QPushButton* removeButton = msgBox->addButton("&Remove", QMessageBox::YesRole);
            msgBox->setDefaultButton(removeButton);
            msgBox->setIcon(QMessageBox::Question);
            msgBox->exec();

            if (msgBox->clickedButton() == removeButton)
                m_recentlyUsedFiles->removeRecentlyUsed(absolutePath);

            delete msgBox;
        }
    }
}

void MainWindow::slot_openRecentProject(const QString& absolutePath)
{
    /*if (auto action = dynamic_cast<QAction*>(sender()))*/ {
//        QString absolutePath = action->data().toString();
        if (QFileInfo(absolutePath).exists()) {
            if (m_project != nullptr) {
                // give user a chance to save changes if needed
                if (!slot_closeProject())
                    return;
            }
            openProject(absolutePath);
        } else {
            QMessageBox* msgBox = new QMessageBox(this);
            msgBox->setText(QString("Project \"%1\" was not found.").arg(absolutePath));
            msgBox->setInformativeText("The project file does not exist; it may have been moved or deleted."
                                       " Do you want to remove it from the recently used list?");
            msgBox->addButton(QMessageBox::Cancel);
            auto removeButton = msgBox->addButton("&Remove", QMessageBox::YesRole);
            msgBox->setDefaultButton(removeButton);
            msgBox->setIcon(QMessageBox::Question);
            msgBox->exec();

            if (msgBox->clickedButton() == removeButton)
                m_recentlyUsedProjects->removeRecentlyUsed(absolutePath);

            delete msgBox;
        }
    }
}

void MainWindow::slot_tabsMenuActivateTab()
{
    if (auto action = dynamic_cast<QAction*>(sender())) {
        QString absolutePath = action->data().toString();
        activateEditorTabByFilePath(absolutePath);
    }
}

void MainWindow::slot_toggleFullScreen()
{
    if (isFullScreen()) {
        showNormal();
        if (m_wasMaximized)
            showMaximized();
    } else {
        m_wasMaximized = isMaximized();
        showFullScreen();
    }
}

void MainWindow::slot_toggleStatusbar()
{
    if (statusBar()->isVisible()) {
        statusBar()->hide();
    } else {
        statusBar()->show();
    }
}

void MainWindow::slot_helpQuickstart()
{
    QDesktopServices::openUrl(QString("file://%1").arg(QDir(paths::DOC_DIR).absoluteFilePath("quickstart-guide.pdf")));
}

void MainWindow::slot_helpUserManual()
{
    QDesktopServices::openUrl(QString("file://%1").arg(QDir(paths::DOC_DIR).absoluteFilePath("user-manual.pdf")));
}

void MainWindow::slot_helpWikiPage()
{
    QDesktopServices::openUrl(QString("http://www.cegui.org.uk/wiki/index.php/CEED"));
}

void MainWindow::slot_sendFeedback()
{
    QDesktopServices::openUrl(QString("http://www.cegui.org.uk/phpBB2/viewforum.php?f=15"));
}

void MainWindow::slot_reportBug()
{
    QDesktopServices::openUrl(QString("http://www.cegui.org.uk/mantis/bug_report_page.php"));
}

void MainWindow::slot_ceguiDebugInfo()
{
    ceguiContainerWidget->m_debugInfo->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool handled = quit();

    if (!handled)
        event->ignore();
    else
        event->accept();
}


} // namespace mainwindow
} // namespace CEED
