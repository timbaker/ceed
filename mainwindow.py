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

from PySide.QtGui import QMainWindow, QTabWidget
import ui.mainwindow

import tab

class MainWindow(QMainWindow):
    def __init__(self):
        super(MainWindow, self).__init__()
        
        self.editorFactories = []
        self.activeEditor = None
        
        self.ui = ui.mainwindow.Ui_MainWindow()
        self.ui.setupUi(self)
        
        self.tabs = self.centralWidget().findChild(QTabWidget, "tabs")
        self.tabs.currentChanged.connect(self.slot_currentTabChanged)
        self.tabs.tabCloseRequested.connect(self.slot_tabCloseRequested)
        
        self.test = tab.MessageTabbedEditor("Test.file", "Test test test")
        self.test.initialise(self)
        
        self.test2 = tab.MessageTabbedEditor("Test2.file", "Test22 test test")
        self.test2.initialise(self)
        
        self.test3 = tab.MessageTabbedEditor("Test3.file", "Test333 test test")
        self.test3.initialise(self)
    
    def slot_currentTabChanged(self, index):
        wdt = self.tabs.widget(index)
        
        if self.activeEditor:
            self.activeEditor.deactivate()

        if wdt:
            wdt.tabbedEditor.activate()
    
    def slot_tabCloseRequested(self, index):
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor
        
        if not editor.hasChanges():
            # we can close immediately
            editor.finalise()
            
        pass
        
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
