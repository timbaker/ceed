################################################################################
#   CEED - A unified CEGUI editor
#   Copyright (C) 2011 Martin Preisler <preisler.m@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
################################################################################

from PySide.QtCore import *
from PySide.QtGui import *
from recentlyusedmenuentry import *

import ui.mainwindow

import os

import settings
import action

import commands
import project
import cegui
import filesystembrowser

import editors
import help
import about


class MainWindow(QMainWindow):
    """The central window of the application.
    
    This is a singleton class, it is assured to only be constructed once during runtime.
    """
    
    # singleton pattern implementation
    # (usage: mainwindow.MainWindow.instance.doSomething() where mainwindow is the module)
    instance = None
    
    # TODO: This class has grown too much, I think it has too many responsibilities
    #       and refactoring will be needed in the future.
    
    def __init__(self, app):
        # make sure multiple instantiation won't happen
        assert(MainWindow.instance is None)
        MainWindow.instance = self
        
        super(MainWindow, self).__init__()
        
        self.app = app
        
        self.qsettings = QSettings("CEGUI", "CEED")
        
        self.settings = settings.Settings(self.qsettings)
        # we have to construct ActionManager before settings interface (as it alters the settings declaration)!        
        self.actionManager = action.ActionManager(self, self.settings)
        # download all values from the persistence store
        self.settings.download()
        
        self.settingsInterface = settings.interface.QtSettingsInterface(self.settings)
        
        self.recentlyUsedProjects = RecentlyUsedMenuEntry(self.qsettings, "Projects")
        
        self.editorFactories = [
            editors.animation_list.AnimationListEditorFactory(),
            editors.bitmap.BitmapTabbedEditorFactory(),
            editors.imageset.ImagesetTabbedEditorFactory(),
            editors.layout.LayoutTabbedEditorFactory(),
            editors.property_mappings.PropertyMappingsEditorFactory(),
            editors.text.TextTabbedEditorFactory()
        ]
        
        self.activeEditor = None
        self.project = None
        
        self.ui = ui.mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        
        # we start CEGUI early and we always start it
        self.ceguiInstance = cegui.Instance()
        self.ceguiContainerWidget = cegui.container.ContainerWidget(self.ceguiInstance)
        
        self.tabs = self.centralWidget().findChild(QTabWidget, "tabs")
        self.tabs.currentChanged.connect(self.slot_currentTabChanged)
        self.tabs.tabCloseRequested.connect(self.slot_tabCloseRequested)
        # TODO: this is potentially nasty since the tabBar method is protected
        self.tabBar = self.tabs.tabBar()
        self.tabBar.setContextMenuPolicy(Qt.CustomContextMenu)
        self.tabBar.customContextMenuRequested.connect(self.slot_tabBarCustomContextMenuRequested)
        
        self.tabEditors = []
        
        self.projectManager = project.ProjectManager()
        self.addDockWidget(Qt.RightDockWidgetArea, self.projectManager)
        
        self.fileSystemBrowser = filesystembrowser.FileSystemBrowser()
        self.fileSystemBrowser.setVisible(False)
        self.addDockWidget(Qt.RightDockWidgetArea, self.fileSystemBrowser)
        
        self.undoViewer = commands.UndoViewer()
        self.undoViewer.setVisible(False)
        self.addDockWidget(Qt.RightDockWidgetArea, self.undoViewer)
        
        self.menuPanels = self.findChild(QMenu, "menuPanels")
        self.menuPanels.addAction(self.projectManager.toggleViewAction())
        self.menuPanels.addAction(self.fileSystemBrowser.toggleViewAction())
        self.menuPanels.addAction(self.undoViewer.toggleViewAction())
        
        self.setupActions()
        self.connectSignals()
        
        self.restoreSettings()     
    
    def setupActions(self):
        self.connectionGroup = action.ConnectionGroup(self.actionManager)
        
        self.globalToolbar = self.findChild(QToolBar, "globalToolbar")
        
        self.fileMenu = self.findChild(QMenu, "menuFile")
        self.editMenu = self.findChild(QMenu, "menuEdit")
        
        self.newFileAction = self.actionManager.getAction("all_editors/new_file")
        self.fileMenu.addAction(self.newFileAction)
        self.globalToolbar.addAction(self.newFileAction)
        self.connectionGroup.add(self.newFileAction, receiver = self.slot_newFileDialog)
        
        self.openFileAction = self.actionManager.getAction("all_editors/open_file")
        self.fileMenu.addAction(self.openFileAction)
        self.globalToolbar.addAction(self.openFileAction)
        self.connectionGroup.add(self.openFileAction, receiver = self.slot_openFileDialog)
        
        self.saveAction = self.actionManager.getAction("all_editors/save_file")
        self.saveAction.setEnabled(False)
        self.fileMenu.addAction(self.saveAction)
        self.globalToolbar.addAction(self.saveAction)
        self.connectionGroup.add(self.saveAction, receiver = self.slot_save)
        
        self.saveAllAction = self.actionManager.getAction("all_editors/save_all_files")
        self.saveAllAction.setEnabled(False)
        self.fileMenu.addAction(self.saveAllAction)
        self.globalToolbar.addAction(self.saveAllAction)
        self.connectionGroup.add(self.saveAllAction, receiver = self.slot_saveAll)
        
        self.fileMenu.addSeparator()
        
        # tab bar context menu (but also added to the file menu so it's easy to discover)
        self.closeTabAction = self.actionManager.getAction("all_editors/close_current_tab")
        self.closeTabAction.setEnabled(False)
        self.fileMenu.addAction(self.closeTabAction)
        self.connectionGroup.add(self.closeTabAction, receiver = self.slot_closeTab)

        self.closeOtherTabsAction = self.actionManager.getAction("all_editors/close_other_tabs")
        self.closeOtherTabsAction.setEnabled(False)
        self.fileMenu.addAction(self.closeOtherTabsAction)
        self.connectionGroup.add(self.closeOtherTabsAction, receiver = self.slot_closeOtherTabs)

        self.closeAllTabsAction = self.actionManager.getAction("all_editors/close_all_tabs")
        self.closeAllTabsAction.setEnabled(False)
        self.fileMenu.addAction(self.closeAllTabsAction)
        self.connectionGroup.add(self.closeAllTabsAction, receiver = self.slot_closeAllTabs)
        # end of tab bar context menu

        self.fileMenu.addSeparator()        
        self.globalToolbar.addSeparator()
        
        self.undoAction = self.actionManager.getAction("all_editors/undo")
        self.undoAction.setEnabled(False)
        self.editMenu.addAction(self.undoAction)
        self.globalToolbar.addAction(self.undoAction)
        self.connectionGroup.add(self.undoAction, receiver = self.slot_undo)

        self.redoAction = self.actionManager.getAction("all_editors/redo")
        self.redoAction.setEnabled(False)
        self.editMenu.addAction(self.redoAction)
        self.globalToolbar.addAction(self.redoAction)
        self.connectionGroup.add(self.redoAction, receiver = self.slot_redo)
        
        self.editMenu.addSeparator()
        self.globalToolbar.addSeparator()
        
        self.cutAction = self.actionManager.getAction("all_editors/cut")
        self.editMenu.addAction(self.cutAction)
        self.globalToolbar.addAction(self.cutAction)
        self.connectionGroup.add(self.cutAction, receiver = self.slot_cut)
        
        self.copyAction = self.actionManager.getAction("all_editors/copy")
        self.editMenu.addAction(self.copyAction)
        self.globalToolbar.addAction(self.copyAction)
        self.connectionGroup.add(self.copyAction, receiver = self.slot_copy)
        
        self.pasteAction = self.actionManager.getAction("all_editors/paste")
        self.editMenu.addAction(self.pasteAction)
        self.globalToolbar.addAction(self.pasteAction)
        self.connectionGroup.add(self.pasteAction, receiver = self.slot_paste)
        
        self.editMenu.addSeparator()
        self.globalToolbar.addSeparator()
        
        self.projectSettingsAction = self.actionManager.getAction("project_management/project_settings")
        # when this starts up, no project is opened, hence you can't view/edit settings of the current project
        self.projectSettingsAction.setEnabled(False)
        self.editMenu.addAction(self.projectSettingsAction)
        self.globalToolbar.addAction(self.projectSettingsAction)
        self.connectionGroup.add(self.projectSettingsAction, receiver = self.slot_projectSettings)
        
        self.editMenu.addAction(self.actionManager.getAction("general/application_settings"))
        self.connectionGroup.add("general/application_settings", receiver = lambda: self.settingsInterface.show())
        
        self.fileMenu.addAction(self.actionManager.getAction("project_management/new_project"))
        self.connectionGroup.add("project_management/new_project", receiver = self.slot_newProject)
        
        self.fileMenu.addAction(self.actionManager.getAction("project_management/open_project"))
        self.connectionGroup.add("project_management/open_project", receiver = self.slot_openProject)
        
        self.saveProjectAction = self.actionManager.getAction("project_management/save_project")
        # when this starts up, no project is opened, hence you can't save the "no project"
        self.saveProjectAction.setEnabled(False)
        self.fileMenu.addAction(self.saveProjectAction)
        self.globalToolbar.addAction(self.saveProjectAction)
        self.connectionGroup.add(self.saveProjectAction, receiver = self.slot_saveProject)
        
        self.closeProjectAction = self.actionManager.getAction("project_management/close_project")
        self.fileMenu.addAction(self.closeProjectAction)
        self.connectionGroup.add(self.closeProjectAction, receiver = self.slot_closeProject)
        # when this starts up, no project is opened, hence you can't close the current project
        self.closeProjectAction.setEnabled(False)

        self.fileMenu.addSeparator()
    
        self.recentProjectsMenu = self.findChild(QMenu, "menuRecentProjects")
        self.recentlyUsedProjects.setParentMenu(self.recentProjectsMenu, self.slot_openRecentProject)
        
        self.licenseAction = self.findChild(QAction, "actionLicense")
        self.licenseAction.triggered.connect(self.slot_license)
        
        self.qtAction = self.findChild(QAction, "actionQt")
        self.qtAction.triggered.connect(QApplication.aboutQt)
        
        self.connectionGroup.add("general/quit", receiver = self.slot_quit)
        self.fileMenu.addAction(self.actionManager.getAction("general/quit"))
        
        self.connectionGroup.connectAll()
        
    def connectSignals(self):
        self.projectManager.fileOpenRequested.connect(self.slot_openFile)
        self.fileSystemBrowser.fileOpenRequested.connect(self.slot_openFile)

    def openProject(self, path, fromRecentProject = False):
        assert(not self.project)
        
        self.project = project.Project()
        self.project.load(path)
        self.projectManager.setProject(self.project)
        self.fileSystemBrowser.setDirectory(self.project.getAbsolutePathOf(""))
        # sync up the cegui instance
        self.ceguiInstance.syncToProject(self.project)
        
        self.recentlyUsedProjects.addRecentlyUsed(str(self.project.projectFilePath))
            
        # and enable respective actions
        self.saveProjectAction.setEnabled(True)
        self.closeProjectAction.setEnabled(True)
        self.projectSettingsAction.setEnabled(True)

    def closeProject(self):
        self.projectManager.setProject(None)
        self.recentlyUsedProjects.addRecentlyUsed(str(self.project.projectFilePath))
        self.project.unload()
        self.project = None
        
        # no project is opened anymore
        #self.projectFiles.setEnabled(False)
        
        self.saveProjectAction.setEnabled(False)
        self.closeProjectAction.setEnabled(False)
        self.projectSettingsAction.setEnabled(False)
        
        
        
    def saveProject(self):
        self.project.save()
    
    def saveProjectAs(self, newPath):
        self.project.save(newPath)
        
    def createEditorForFile(self, absolutePath):
        ret = None
                
        projectRelativePath = os.path.relpath(absolutePath, self.project.baseDirectory) if self.project else "<No project opened>"        
        
        if not os.path.exists(absolutePath):
            ret = editors.MessageTabbedEditor(absolutePath,
                   "Couldn't find '%s' (project relative path: '%s'), please check that that your project's "
                   "base directory is set up correctly and that you hadn't deleted "
                   "the file from your HDD. Consider removing the file from the project." % (absolutePath, projectRelativePath))
        else: 
            possibleFactories = []
            
            for factory in self.editorFactories:
                if factory.canEditFile(absolutePath):
                    possibleFactories.append(factory)
                    
            # at this point if possibleFactories is [], no registered tabbed editor factory wanted
            # to accept the file, so we create MessageTabbedEditor that will simply
            # tell the user that given file can't be edited
            #
            # IMO this is a reasonable compromise and plays well with the rest of 
            # the editor without introducing exceptions, etc...
            if len(possibleFactories) == 0:
                ret = editors.MessageTabbedEditor(absolutePath,
                       "No included tabbed editor was able to accept '%s' (project relative path: '%s'), please "
                       "check that it's a file CEED supports and that it has the correct extension "
                       "(CEED enforces proper extensions)" % (absolutePath, projectRelativePath))
                
            else:
                # one or more factories wants to accept the file
                factory = None
                
                if len(possibleFactories) == 1:
                    # it's decided, just one factory wants to accept the file
                    factory = possibleFactories[0]
                    
                else:
                    assert(len(possibleFactories) > 1)
                    
                    # more than 1 factory wants to accept the file, offer a dialog and let user choose
                    dialog = editors.MultiplePossibleFactoriesDialog(possibleFactories)
                    result = dialog.exec_()
                    
                    if result == QDialog.Accepted:
                        selection = dialog.factoryChoice.selectedItems()
                        
                        if len(selection) == 1:
                            factory = selection[0].data(Qt.UserRole)
        
                if factory is None:
                    ret = editors.MessageTabbedEditor(absolutePath,
                       "You failed to choose an editor to open '%s' with (project relative path: '%s')." % (absolutePath, projectRelativePath))
                
                else:
                    ret = factory.create(absolutePath)
        
        if not self.project and ret.requiresProject:
            # the old editor will be destroyed automatically by python GC
            ret = editors.MessageTabbedEditor(absolutePath,
                       "Opening this file requires you to have a project opened!")
        
        try:
            ret.initialise(self)
        
        except Exception as e:
            # it may have been partly constructed at this point
            try:
                ret.finalise()
                
            except:
                # catch all exception the finalisation raises (we can't deal with them anyways)
                pass
            
            raise e
            
        self.tabEditors.append(ret)
        return ret    

    def openEditorTab(self, absolutePath):
        """Opens editor tab. Creates new editor if such file wasn't opened yet and if it was opened,
        it just makes the tab current.
        """
        
        absolutePath = os.path.normpath(absolutePath)
        
        for tabEditor in self.tabEditors:            
            if tabEditor.filePath == absolutePath:
                tabEditor.makeCurrent()
                return
                
        editor = self.createEditorForFile(absolutePath)
        editor.makeCurrent()
        
    def closeEditorTab(self, editor):
        """Closes given editor tab.
        
        note: No checks are made, make sure you pass proper existing editor!
        """
        
        editor.finalise()
        self.tabEditors.remove(editor)
    
    def saveSettings(self):
        #self.qsettings.setValue("geometry", self.saveGeometry())
        #self.qsettings.setValue("state", self.saveState())
        pass
        
    def restoreSettings(self):
        #if self.qsettings.contains("geometry"):
        #    self.restoreGeometry(self.qsettings.value("geometry"))
        #if self.qsettings.contains("state"):
        #    self.restoreState(self.qsettings.value("state"))
        return
        
    def quit(self):
        """Safely quits the editor, prompting user to save changes to files and the project."""
        
        self.saveSettings()
        
        if self.project:
            # if the slot returned False, user pressed Cancel
            if not self.slot_closeProject():
                return False
        
        lastTab = None
        while len(self.tabEditors) > 0:
            currentTab = self.tabs.widget(0)
            if currentTab == lastTab:
                # user pressed cancel on one of the tab editor 'save without changes' dialog,
                # cancel the whole quit operation!
                return False
            lastTab = currentTab
            
            self.slot_tabCloseRequested(0)
            
        self.app.quit()
        
    def slot_newProject(self):
        if self.project:
            # another project is already opened!
            result = QMessageBox.question(self,
                                          "Another project already opened!",
                                          "Before creating a new project, you must close the one currently opened. "
                                          "Do you want to close currently opened project? (all unsaved changes will be lost!)",
                                          QMessageBox.Yes | QMessageBox.Cancel,
                                          QMessageBox.Cancel)
            
            if result == QMessageBox.Yes:
                self.closeProject()
            else:
                # User selected cancel, NOOP
                return
        
        newProjectDialog = project.NewProjectDialog()
        result = newProjectDialog.exec_()
        
        if result == QDialog.Accepted:
            newProject = newProjectDialog.createProject()
            newProject.save()
            
            self.openProject(newProject.projectFilePath)
    
    def slot_openProject(self):
        if self.project:
            # another project is already opened!
            result = QMessageBox.question(self,
                                          "Another project already opened!",
                                          "Before opening a project, you must close the one currently opened. "
                                          "Do you want to close currently opened project? (all unsaved changes will be lost!)",
                                          QMessageBox.Yes | QMessageBox.Cancel,
                                          QMessageBox.Cancel)
            
            if result == QMessageBox.Yes:
                self.closeProject()
            else:
                # User selected cancel, NOOP
                return
        
        file, filter = QFileDialog.getOpenFileName(self,
                                           "Open existing project file",
                                           "",
                                           "Project files (*.project)")
        
        if file != "":
            # user actually selected something ;-)
            
            self.openProject(file)
            
    def slot_saveProject(self):
        self.saveProject()
        
    def slot_closeProject(self):
        assert(self.project)
        
        if self.project.hasChanges():
            result = QMessageBox.question(self,
                                          "Project file has changes!",
                                          "There are unsaved changes in the project file "
                                          "Do you want to save them? "
                                          "(Pressing Discard will discard the changes!)",
                                          QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel,
                                          QMessageBox.Save)
            
            if result == QMessageBox.Save:
                self.saveProject()
                return True
            
            elif result == QMessageBox.Cancel:
                return False
            
        self.closeProject()
        return True
        
    def slot_projectSettings(self):
        dialog = project.ProjectSettingsDialog(self.project)
        
        if dialog.exec_() == QDialog.Accepted:
            dialog.apply(self.project)
            self.ceguiInstance.syncToProject(self.project)
    
    def slot_newFileDialog(self):
        dir = ""
        if self.project:
            dir = self.project.getAbsolutePathOf("")
        
        file, filter = QFileDialog.getSaveFileName(self, "New File", dir)
        
        if file:
            f = open(file, "w")
            f.close()

            self.openEditorTab(file)
    
    def slot_openFile(self, absolutePath):
        self.openEditorTab(absolutePath)
    
    def slot_openFileDialog(self):
        dir = ""
        if self.project:
            dir = self.project.getAbsolutePathOf("")
        
        file, filter = QFileDialog.getOpenFileName(self, "Open File", dir)
        
        if file:
            self.openEditorTab(file)
    
    def slot_currentTabChanged(self, index):
        # to fight flicker
        self.tabs.setUpdatesEnabled(False)
        
        wdt = self.tabs.widget(index)
        
        if self.activeEditor:
            self.activeEditor.deactivate()

        # it's the tabbed editor's responsibility to handle these,
        # we disable them by default
        self.undoAction.setEnabled(False)
        self.redoAction.setEnabled(False)
        # also reset their texts in case the tabbed editor messed with them
        self.undoAction.setText("Undo")
        self.redoAction.setText("Redo")
        # set undo stack to None as we have no idea whether the previous tab editor
        # set it to something else
        self.undoViewer.setUndoStack(None)
        
        # we also clear the status bar
        self.statusBar().clearMessage()

        if wdt:
            self.saveAction.setEnabled(True)
            self.saveAllAction.setEnabled(True)
            
            self.closeTabAction.setEnabled(True)
            self.closeOtherTabsAction.setEnabled(True)
            
            wdt.tabbedEditor.activate()
        else:
            # None is selected right now, lets disable Save and Close actions
            self.saveAction.setEnabled(False)
            self.saveAllAction.setEnabled(False)
            
            self.closeTabAction.setEnabled(False)
            self.closeOtherTabsAction.setEnabled(False)
            
        self.tabs.setUpdatesEnabled(True)
    
    def slot_tabCloseRequested(self, index):
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor
        
        if not editor.hasChanges():
            # we can close immediately
            self.closeEditorTab(editor)
            return True
            
        else:
            # we have changes, lets ask the user whether we should dump them or save them
            result = QMessageBox.question(self,
                                          "Unsaved changes!",
                                          "There are unsaved changes in '%s'. "
                                          "Do you want to save them? "
                                          "(Pressing Discard will discard the changes!)" % (editor.filePath),
                                          QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel,
                                          QMessageBox.Save)
            
            if result == QMessageBox.Save:
                # lets save changes and then kill the editor (This is the default action)
                editor.save()
                self.closeEditorTab(editor)
                return True
                
            elif result == QMessageBox.Discard:
                # changes will be discarded
                # note: we don't have to call editor.discardChanges here
                
                self.closeEditorTab(editor)
                return True
            
            # don't do anything if user selected 'Cancel'
            return False

    def slot_closeTab(self):
        # a simple delegate
        self.slot_tabCloseRequested(self.tabs.currentIndex())

    def slot_closeOtherTabs(self):
        current = self.tabs.currentWidget()
        
        i = 0
        while i < self.tabs.count():
            if self.tabs.widget(i) == current:
                # we skip the current widget
                i += 1
            else:
                if not self.slot_tabCloseRequested(i):
                    # user selected Cancel, we skip this widget
                    i += 1
                    
    def slot_closeAllTabs(self):
        i = 0
        while i < self.tabs.count():
            if not self.slot_tabCloseRequested(i):
                # user selected Cancel, we skip this widget
                i += 1

    def slot_tabBarCustomContextMenuRequested(self, point):
        self.tabs.setCurrentIndex(self.tabBar.tabAt(point))
        
        menu = QMenu(self)
        menu.addAction(self.closeTabAction)
        menu.addSeparator()
        menu.addAction(self.closeOtherTabsAction)
        menu.addAction(self.closeAllTabsAction)
        
        menu.exec_(self.tabBar.mapToGlobal(point))

    def slot_save(self):
        if self.activeEditor:
            self.activeEditor.save()
    
    def slot_saveAll(self):
        for editor in self.tabEditors:
            editor.save()
        
    def slot_undo(self):
        if self.activeEditor:
            self.activeEditor.undo()
            
    def slot_redo(self):
        if self.activeEditor:
            self.activeEditor.redo()
            
    def slot_cut(self):
        if self.activeEditor:
            self.activeEditor.performCut()
    
    def slot_copy(self):
        if self.activeEditor:
            self.activeEditor.performCopy()
    
    def slot_paste(self):
        if self.activeEditor:
            self.activeEditor.performPaste()
    
    def slot_license(self):
        dialog = about.LicenseDialog()
        dialog.exec_()
    
    def slot_quit(self):
        return self.quit()

    def closeEvent(self, event):
        handled = self.slot_quit()
        
        if not handled:
            event.ignore()
            
        else:
            event.accept()
            
        

        
    def slot_openRecentProject(self):
        action = self.sender()
        if action:
            if self.project:
                # give user a chance to save changes if needed
                if not self.slot_closeProject():
                    return
            
            self.openProject(action.data(), True)
