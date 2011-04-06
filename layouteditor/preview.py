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

class LayoutPreviewer(QWidget, mixedtab.EditMode):
    def __init__(self, parent):
        super(LayoutPreviewer, self).__init__()
        
        self.parent = parent
        self.rootWidget = None

    def activate(self):
        assert(self.rootWidget == None)
        
        # we have to make the context the current context to ensure textures are fine
        self.parent.mainWindow.ceguiWidget.makeCurrent()
        
        # lets clone so we don't affect the layout at all
        self.rootWidget = self.parent.visual.rootWidget.clone("Preview")
        PyCEGUI.System.getSingleton().setGUISheet(self.rootWidget)
        
    def deactivate(self):
        PyCEGUI.WindowManager.getSingleton().destroyWindow(self.rootWidget)
        self.rootWidget = None

    def showEvent(self, event):
        super(LayoutPreviewer, self).showEvent(event)
        
        self.parent.mainWindow.ceguiWidget.injectInput = True
        self.parent.mainWindow.ceguiWidget.setParent(self)
        self.parent.mainWindow.ceguiWidget.setGeometry(0, 0, 1024, 768)
        self.parent.mainWindow.ceguiWidget.show()
        
        if self.rootWidget:
            PyCEGUI.System.getSingleton().setGUISheet(self.rootWidget)

    def hideEvent(self, event):
        if self.rootWidget:
            self.parent.mainWindow.ceguiWidget.injectInput = False
            self.parent.mainWindow.ceguiWidget.hide()
            self.parent.mainWindow.ceguiWidget.setParent(None)
            
            # we have to make the context the current context to ensure textures are fine
            self.parent.mainWindow.ceguiWidget.makeCurrent()
            
            PyCEGUI.System.getSingleton().setGUISheet(None)
            
        super(LayoutPreviewer, self).hideEvent(event)
    