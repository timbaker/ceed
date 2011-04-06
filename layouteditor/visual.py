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
from PySide.QtCore import *

import mixedtab

import PyCEGUI

class VisualEditing(QWidget, mixedtab.EditMode):
    def __init__(self, parent):
        super(VisualEditing, self).__init__()
        
        self.parent = parent
        self.rootWidget = None

    def initialise(self, rootWidget):
        self.replaceRootWidget(rootWidget)
    
    def replaceRootWidget(self, newRoot):
        oldRoot = self.rootWidget
            
        self.rootWidget = newRoot
        PyCEGUI.System.getSingleton().setGUISheet(self.rootWidget)
    
        if oldRoot:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(oldRoot)
        
        # cause full redraw to ensure nothing gets stuck
        PyCEGUI.System.getSingleton().signalRedraw()
    
    def showEvent(self, event):
        super(VisualEditing, self).showEvent(event)
        
        self.parent.mainWindow.ceguiWidget.setParent(self)
        self.parent.mainWindow.ceguiWidget.setGeometry(0, 0, 1024, 768)
        self.parent.mainWindow.ceguiWidget.show()
        
        PyCEGUI.System.getSingleton().setGUISheet(self.rootWidget)
        #self.dockWidget.setEnabled(True)
        #self.toolBar.setEnabled(True)
    
    def hideEvent(self, event):
        #self.dockWidget.setEnabled(False)
        #self.toolBar.setEnabled(False)
        
        self.parent.mainWindow.ceguiWidget.hide()
        self.parent.mainWindow.ceguiWidget.setParent(None)
        
        super(VisualEditing, self).hideEvent(event)
    