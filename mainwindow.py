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

from PySide.QtGui import *
import ui.mainwindow

# the various editor imports
import texteditor

import project
import tab

class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        
        self.editorFactories = []
        self.activeEditor = None
        self.project = None
        
        self.ui = ui.mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        
        self.tabs = self.centralWidget().findChild(QTabWidget, "tabs")
        self.tabs.currentChanged.connect(self.slot_currentTabChanged)
        self.tabs.tabCloseRequested.connect(self.slot_tabCloseRequested)
        
        self.tabEditors = []
        
        self.projectManagement = self.findChild(QDockWidget, "projectManagement")
        self.projectFiles = self.projectManagement.findChild(QTreeWidget, "projectFiles")
        
        self.connectActions()
        self.connectSignals()
        
        self.registerEditorFactories()
    
    def openProject(self, path):
        assert(not self.project)
        
        self.project = project.Project()
        self.project.load(path)
        self.project.syncProjectTree(self.projectFiles)
        
        # project has been opened
        # enable the project management tree
        self.projectFiles.setEnabled(True)        
        
        # and enable respective actions
        self.saveProjectAction.setEnabled(True)
        self.closeProjectAction.setEnabled(True)
        
    def closeProject(self):
        self.project.unload()
        self.projectFiles.clear()
        self.project = None
        
        # no project is opened anymore
        self.projectFiles.setEnabled(False)
        
        self.saveProjectAction.setEnabled(False)
        self.closeProjectAction.setEnabled(False)
        
    def saveProject(self):
        self.project.save()
    
    def saveProjectAs(self, newPath):
        self.project.save(newPath)
    
    def registerEditorFactory(self, factory):
        self.editorFactories.append(factory)
        
    def unregisterEditorFactory(self, factory):
        self.editorFactories.remove(factory)
    
    def registerEditorFactories(self):
        self.registerEditorFactory(texteditor.TextTabbedEditorFactory())
        
    def createEditorForFile(self, filePath):
        absolutePath = self.project.getAbsolutePathOf(filePath)
        
        ret = None
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
                   "No included tabbed editor was able to accept '%s'" % (filePath))
        
        ret.initialise(self)
        self.tabEditors.append(ret)
        
        return ret    

    def connectActions(self):
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
        
        self.saveAction = self.findChild(QAction, "actionSave")
        self.saveAction.setEnabled(False)
        self.saveAllAction = self.findChild(QAction, "actionSaveAll")
        self.saveAllAction.setEnabled(False)
        self.closeAction = self.findChild(QAction, "actionClose")
        self.closeAction.setEnabled(False)
        
    def connectSignals(self):
        self.projectFiles.itemDoubleClicked.connect(self.slot_openFileTab)
    
    def closeEditorTab(self, editor):
        if not editor.hasChanges():
            # we can close immediately
            editor.finalise()
            
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
                editor.finalise()
                self.tabEditors.remove(editor)
                
            elif result == QMessageBox.Discard:
                # changes will be discarded
                # note: we don't have to call editor.discardChanges here
                
                editor.finalise()
                self.tabEditors.remove(editor)
            
            # don't do anything if user selected 'Cancel'
        
    def slot_saveProject(self):
        self.saveProject()
    
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
    
    def slot_openFileTab(self, treeItem, column):
        if (column != 0) or (not treeItem) or (not treeItem.item.type == "file"):
            # silently ignore invalid double click
            return
        
        if treeItem.item.openedTabEditor:
            treeItem.item.openedTabEditor.makeCurrent()
            return
        
        editor = self.createEditorForFile(treeItem.item.path)
        editor.makeCurrent()
        
        treeItem.item.openedTabEditor = editor
        editor.treeItem = treeItem
        
    def slot_currentTabChanged(self, index):
        wdt = self.tabs.widget(index)
        
        if self.activeEditor:
            self.activeEditor.deactivate()

        if wdt:
            wdt.tabbedEditor.activate()
            
            self.saveAction.setEnabled(True)
            self.saveAllAction.setEnabled(True)
            self.closeAction.setEnabled(True)
        else:
            # None is selected right now, lets disable Save and Close actions
            self.saveAction.setEnabled(False)
            self.saveAllAction.setEnabled(False)
            self.closeAction.setEnabled(False)
    
    def slot_tabCloseRequested(self, index):
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor
        
        self.closeEditorTab(editor)
