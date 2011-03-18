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

import ui.mainwindow

import os

import project
import cegui
import filesystembrowser
import propertyinspector

import tab
# the various editor imports
import bitmapeditor
import texteditor
import imageseteditor

import about

class MainWindow(QMainWindow):
    """The central window of the application"""
    
    def __init__(self, app):
        super(MainWindow, self).__init__()
        
        self.app = app
        self.settings = QSettings("CEGUI", "CEED")

        self.editorFactories = [
            bitmapeditor.BitmapTabbedEditorFactory(),
            texteditor.TextTabbedEditorFactory(),
            imageseteditor.ImagesetTabbedEditorFactory()
        ]
        
        self.activeEditor = None
        self.project = None
        
        self.ui = ui.mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        
        # we start CEGUI early and we always start it
        self.ceguiWidget = cegui.CEGUIWidget()
        #self.ceguiWidget.setParent(self.centralWidget())
        #self.ceguiWidget.show()
        
        # we don't show the debug widget by default
        self.ceguiWidget.debugInfo.setVisible(False)
        self.ceguiWidget.debugInfo.setFloating(True)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.ceguiWidget.debugInfo)
        
        self.tabs = self.centralWidget().findChild(QTabWidget, "tabs")
        self.tabs.currentChanged.connect(self.slot_currentTabChanged)
        self.tabs.tabCloseRequested.connect(self.slot_tabCloseRequested)
        
        self.tabEditors = []
        
        self.projectManager = project.ProjectManager()
        self.addDockWidget(Qt.RightDockWidgetArea, self.projectManager)
        
        self.fileSystemBrowser = filesystembrowser.FileSystemBrowser()
        self.fileSystemBrowser.setVisible(False)
        self.addDockWidget(Qt.RightDockWidgetArea, self.fileSystemBrowser)
        
        self.connectActions()
        self.connectSignals()
        
        propertyinspector.PropertyInspectorManager.loadMappings("data/StockMappings.xml")
        
        self.restoreSettings()
    
    def connectActions(self):
        self.newProjectAction = self.findChild(QAction, "actionNewProject")
        self.newProjectAction.triggered.connect(self.slot_newProject)
        
        self.saveProjectAction = self.findChild(QAction, "actionSaveProject")
        self.saveProjectAction.triggered.connect(self.slot_saveProject)
        # when this starts up, no project is opened, hence you can't save the "no project"
        self.saveProjectAction.setEnabled(False)
        
        self.openProjectAction = self.findChild(QAction, "actionOpenProject")
        self.openProjectAction.triggered.connect(self.slot_openProject)
        
        self.closeProjectAction = self.findChild(QAction, "actionCloseProject")
        self.closeProjectAction.triggered.connect(self.slot_closeProject)
        # when this starts up, no project is opened, hence you can't close the current project
        self.closeProjectAction.setEnabled(False)
        
        self.projectSettingsAction = self.findChild(QAction, "actionProjectSettings")
        self.projectSettingsAction.triggered.connect(self.slot_projectSettings)
        # when this starts up, no project is opened, hence you can't view/edit settings of the current project
        self.projectSettingsAction.setEnabled(False)
        
        self.saveAction = self.findChild(QAction, "actionSave")
        self.saveAction.setEnabled(False)
        self.saveAllAction = self.findChild(QAction, "actionSaveAll")
        self.saveAllAction.setEnabled(False)
        self.closeAction = self.findChild(QAction, "actionClose")
        self.closeAction.setEnabled(False)
        
        self.undoAction = self.findChild(QAction, "actionUndo")
        self.undoAction.triggered.connect(self.slot_undo)
        self.undoAction.setEnabled(False)
        self.redoAction = self.findChild(QAction, "actionRedo")
        self.undoAction.triggered.connect(self.slot_redo)
        self.redoAction.setEnabled(False)
        
        self.licenseAction = self.findChild(QAction, "actionLicense")
        self.licenseAction.triggered.connect(self.slot_license)
        
        self.quitAction = self.findChild(QAction, "actionQuit")
        self.quitAction.triggered.connect(self.slot_quit)
        
    def connectSignals(self):
        self.findChild(QAction, "actionCEGUIDebugInfoVisible").toggled.connect(self.ceguiWidget.debugInfo.setVisible)
        self.ceguiWidget.debugInfo.visibilityChanged.connect(self.findChild(QAction, "actionCEGUIDebugInfoVisible").setChecked)
        
        self.projectManager.fileOpenRequested.connect(self.slot_openFile)
        self.findChild(QAction, "actionProjectManagerVisible").toggled.connect(self.projectManager.setVisible)
        self.projectManager.visibilityChanged.connect(self.findChild(QAction, "actionProjectManagerVisible").setChecked)
        
        self.fileSystemBrowser.fileOpenRequested.connect(self.slot_openFile)
        self.findChild(QAction, "actionFileSystemBrowserVisible").toggled.connect(self.fileSystemBrowser.setVisible)
        self.fileSystemBrowser.visibilityChanged.connect(self.findChild(QAction, "actionFileSystemBrowserVisible").setChecked)
        
    def openProject(self, path):
        assert(not self.project)
        
        self.project = project.Project()
        self.project.load(path)
        self.projectManager.setProject(self.project)
        self.fileSystemBrowser.setDirectory(self.project.getAbsolutePathOf(""))
        # sync up the cegui instance
        self.ceguiWidget.syncToProject(self.project)
        
        # project has been opened
        # enable the project management tree
        #self.projectFiles.setEnabled(True)        
        
        # and enable respective actions
        self.saveProjectAction.setEnabled(True)
        self.closeProjectAction.setEnabled(True)
        self.projectSettingsAction.setEnabled(True)
        
    def closeProject(self):
        self.projectManager.setProject(None)
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
                
        filePath = os.path.relpath(absolutePath, self.project.baseDirectory) if self.project else "<No project opened>"        
        
        if not os.path.exists(absolutePath):
            ret = tab.MessageTabbedEditor(absolutePath,
                   "Couldn't find '%s' (project relative path: '%s'), please check that that your project's "
                   "base directory is set up correctly and that you hadn't deleted "
                   "the file from your HDD. Consider removing the file from the project." % (absolutePath, filePath))
        else: 
            for factory in self.editorFactories:
                if factory.canEditFile(absolutePath):
                    ret = factory.create(absolutePath)
                    break
            
            # at this point if ret is None, no registered tabbed editor factory wanted
            # to accept the file, so we create MessageTabbedEditor that will simply
            # tell the user that given file can't be edited
            #
            # IMO this is a reasonable compromise and plays well with the rest of 
            # the editor without introducing exceptions, etc...
            if not ret:
                ret = tab.MessageTabbedEditor(absolutePath,
                       "No included tabbed editor was able to accept '%s' (project relative path: '%s'), please "
                       "check that it's a file CEED supports and that it has the correct extension "
                       "(CEED enforces proper extensions)" % (absolutePath, filePath))
        
        ret.initialise(self)
        self.tabEditors.append(ret)
        
        return ret    

    def openFileTab(self, absolutePath):
        absolutePath = os.path.normpath(absolutePath)
        
        for tabEditor in self.tabEditors:            
            if tabEditor.filePath == absolutePath:
                tabEditor.makeCurrent()
                return
                
        editor = self.createEditorForFile(absolutePath)
        editor.makeCurrent()
        
    def closeEditorTab(self, editor):
        editor.finalise()
        self.tabEditors.remove(editor)
    
    def saveSettings(self):
        self.settings.setValue("geometry", self.saveGeometry())
        self.settings.setValue("state", self.saveState())
        
    def restoreSettings(self):
        if self.settings.contains("geometry"):
            self.restoreGeometry(self.settings.value("geometry"))
        if self.settings.contains("state"):
            self.restoreState(self.settings.value("state"))
        
    def quit(self):
        self.saveSettings()
        self.app.exit(0)
        
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
            elif result == QMessageBox.Cancel:
                return
            
        self.closeProject()
        
    def slot_projectSettings(self):
        dialog = project.ProjectSettingsDialog(self.project)
        
        if dialog.exec_() == QDialog.Accepted:
            dialog.apply(self.project)
            self.ceguiWidget.syncToProject(self.project)
    
    def slot_openFile(self, absolutePath):
        self.openFileTab(absolutePath)
    
    def slot_currentTabChanged(self, index):
        wdt = self.tabs.widget(index)
        
        if self.activeEditor:
            self.activeEditor.deactivate()

        # it's the tabbed editor's responsibility to handle these,
        # we disable them by default
        self.undoAction.setEnabled(False)
        self.redoAction.setEnabled(False)

        if wdt:
            self.saveAction.setEnabled(True)
            self.saveAllAction.setEnabled(True)
            self.closeAction.setEnabled(True)
            
            wdt.tabbedEditor.activate()
        else:
            # None is selected right now, lets disable Save and Close actions
            self.saveAction.setEnabled(False)
            self.saveAllAction.setEnabled(False)
            self.closeAction.setEnabled(False)
    
    def slot_tabCloseRequested(self, index):
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor
        
        if not editor.hasChanges():
            # we can close immediately
            self.closeEditorTab(editor)
            
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
                editor.saveChanges()
                self.closeEditorTab(editor)
                
            elif result == QMessageBox.Discard:
                # changes will be discarded
                # note: we don't have to call editor.discardChanges here
                
                self.closeEditorTab(editor)
            
            # don't do anything if user selected 'Cancel'
        
    def slot_undo(self):
        if self.activeEditor:
            self.activeEditor.undo()
            
    def slot_redo(self):
        if self.activeEditor:
            self.activeEditor.redo()
            
    def slot_license(self):
        dialog = about.LicenseDialog()
        dialog.exec_()
    
    def slot_quit(self):
        self.quit()

    def closeEvent(self, event):
        self.slot_quit()
    