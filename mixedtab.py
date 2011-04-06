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

##
# This module contains interfaces for mixed editing tabbed editors (visual, source, ...)

import tab
import commands

from PySide.QtGui import QTabWidget

class ModeSwitchCommand(commands.UndoCommand):
    def __init__(self, parent, oldTabIndex, newTabIndex):
        super(ModeSwitchCommand, self).__init__()
        
        self.parent = parent
        
        self.oldTabIndex = oldTabIndex
        self.newTabIndex = newTabIndex
        
        # we never every merge edit mode changes, no need to define this as refreshText
        self.setText("Change edit mode to '%s'" % self.parent.tabText(newTabIndex))
        
    def undo(self):
        super(ModeSwitchCommand, self).undo()
        
        self.parent.ignoreCurrentChanged = True
        self.parent.setCurrentIndex(self.oldTabIndex)
        self.parent.ignoreCurrentChanged = False
            
    def redo(self):
        # to avoid multiple event firing
        if self.parent.currentIndex() != self.newTabIndex:
            self.parent.ignoreCurrentChanged = True
            self.parent.setCurrentIndex(self.newTabIndex)
            self.parent.ignoreCurrentChanged = False
        
        super(ModeSwitchCommand, self).redo()

class EditMode(object):
    def __init__(self):
        pass
    
    def activate(self):
        pass
    
    def deactivate(self):
        pass

class MixedTabbedEditor(tab.UndoStackTabbedEditor, QTabWidget):
    """This class represents tabbed editor that has little tabs on the bottom
    allowing you to switch editing "modes" - visual, code, ...
    
    Not all modes have to be able to edit! Switching modes pushes undo actions
    onto the UndoStack to avoid confusion when undoing. These actions never merge
    together.
    
    You yourself are responsible for putting new tabs into this widget!
    You should not add/remove modes after the construction!
    """

    def __init__(self, filePath):
        tab.UndoStackTabbedEditor.__init__(self, filePath)
        QTabWidget.__init__(self)
        
        self.setTabPosition(QTabWidget.South)
        self.setTabShape(QTabWidget.Triangular)
        
        self.currentChanged.connect(self.slot_currentChanged)
        
        # will be -1, that means no tabs are selected
        self.currentTabIndex = self.currentIndex()
        # to avoid unnecessary undo command pushes we ignore currentChanged if we are
        # inside ModeChangeCommand.undo or redo
        self.ignoreCurrentChanged = False
        
    def initialise(self, mainWindow):
        super(MixedTabbedEditor, self).initialise(mainWindow)
        
        # the emitted signal only contains the new signal, we have to keep track
        # of the current index ourselves so that we know which one is the "old" one
        # for the undo command
        
        # by this time tabs should have been added so this should not be -1
        self.currentTabIndex = self.currentIndex()
        assert(self.currentTabIndex != -1)
        
    def slot_currentChanged(self, newTabIndex):
        if not self.ignoreCurrentChanged:
            cmd = ModeSwitchCommand(self, self.currentTabIndex, newTabIndex)
            self.undoStack.push(cmd)
        
        oldTab = self.widget(self.currentTabIndex)
        newTab = self.widget(newTabIndex)
        
        if oldTab:
            oldTab.deactivate()
        
        if newTab:
            newTab.activate()
        
        self.currentTabIndex = newTabIndex
