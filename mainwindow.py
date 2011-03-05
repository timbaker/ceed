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
        
        self.projectManagement = self.findChild(QDockWidget, "projectManagement")
        self.projectFiles = self.projectManagement.findChild(QTreeWidget, "projectFiles")
        
        self.connectActions()
        
        self.test = tab.MessageTabbedEditor("Test.file", "Test test test")
        self.test.initialise(self)
        
        self.test2 = tab.MessageTabbedEditor("Test2.file", "Test22 test test")
        self.test2.initialise(self)
        
        self.test3 = tab.MessageTabbedEditor("Test3.file", "Test333 test test")
        self.test3.initialise(self)
    
    def openProject(self, path):
        assert(not self.project)
        
        self.project = project.Project()
        self.project.load(path)
        
        # we can enable close project action now since we have one opened
        self.closeProjectAction.setEnabled(True)
        
    def closeProject(self):
        self.project = None
        # no project is opened anymore
        self.closeProjectAction.setEnabled(False)
    
    def registerEditorFactory(self, factory):
        self.editorFactories.append(factory)
        
    def unregisterEditorFactory(self, factory):
        self.editorFactories.remove(factory)
        
    def createEditorForFile(self, fileName):
        for factory in self.editorFactories:
            if factory.canEditFile(fileName):
                return factory.create(fileName)
        
        # at this point, no registered tabbed editor factory wanted to accept
        # the file, so we create MessageTabbedEditor that will simply tell the
        # user that given file can't be edited
        #
        # IMO this is a reasonable compromise and plays well with the rest of 
        # the editor without introducing exceptions, etc...
        
        return tab.MessageTabbedEditor(fileName,
               "No included tabbed editor was able to accept '%s'" % (fileName))    

    def connectActions(self):
        self.openProjectAction = self.findChild(QAction, "actionOpenProject")
        self.openProjectAction.triggered.connect(self.slot_openProject)
        
        self.closeProjectAction = self.findChild(QAction, "actionCloseProject")
        self.closeProjectAction.triggered.connect(self.slot_closeProject)
        # when this starts up, no project is opened, hence you can't close the current project
        self.closeProjectAction.setEnabled(False)
    
    def slot_currentTabChanged(self, index):
        wdt = self.tabs.widget(index)
        
        if self.activeEditor:
            self.activeEditor.deactivate()

        if wdt:
            wdt.tabbedEditor.activate()
    
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
                                          "(Pressing Discard will discard the changes!)" % (editor.fileName),
                                          QMessageBox.Save | QMessageBox.Discard | QMessageBox.Cancel,
                                          QMessageBox.Save)
            
            if result == QMessageBox.Save:
                # lets save changes and then kill the editor (This is the default action)
                editor.saveChanges()
                editor.finalise()
                
            elif result == QMessageBox.Discard:
                # changes will be discarded
                # note: we don't have to call editor.discardChanges here
                
                editor.finalise()
            
            # don't do anything if user selected 'Cancel'
    
    def slot_tabCloseRequested(self, index):
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor
        
        self.closeEditorTab(editor)
    
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
