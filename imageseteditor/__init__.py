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

import tab

import undo
import visual

from xml.etree import ElementTree

class ImagesetTabbedEditor(tab.UndoStackTabbedEditor, QTabWidget):
    """Binds all imageset editing functionality together
    """
    def __init__(self, filePath):
        tab.UndoStackTabbedEditor.__init__(self, filePath)
        QTabWidget.__init__(self)
        
        self.setTabPosition(QTabWidget.South)
        self.setTabShape(QTabWidget.Triangular)
        
        self.visual = visual.VisualEditing(self)
        self.addTab(self.visual, "Visual")
        
        self.tabWidget = self
    
    def initialise(self, mainWindow):
        super(ImagesetTabbedEditor, self).initialise(mainWindow)
        
        tree = ElementTree.parse(self.filePath)
        root = tree.getroot()
        
        self.visual.initialise(root)
    
    def finalise(self):        
        super(ImagesetTabbedEditor, self).finalise()
        
        self.tabWidget = None
    
    def activate(self):
        super(ImagesetTabbedEditor, self).activate()
        
        self.mainWindow.addToolBar(Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()
        
        self.mainWindow.addDockWidget(Qt.LeftDockWidgetArea, self.visual.dockWidget)
        self.visual.dockWidget.setVisible(True)
        
    def deactivate(self):
        self.mainWindow.removeDockWidget(self.visual.dockWidget)
        self.mainWindow.removeToolBar(self.visual.toolBar)
        
        super(ImagesetTabbedEditor, self).deactivate()
        
    def hasChanges(self):
        return False

class ImagesetTabbedEditorFactory(tab.TabbedEditorFactory):
    def canEditFile(self, filePath):
        extensions = [".imageset"]
        
        for extension in extensions:
            if filePath.endswith(extension):
                return True
            
        return False

    def create(self, filePath):
        return ImagesetTabbedEditor(filePath)
