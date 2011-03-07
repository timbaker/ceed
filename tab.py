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
# This module contains interfaces needed to run editors tabs (multi-file editing)

import os.path

class TabbedEditor(object):
    """This is the base class for a class that takes a file and allows manipulation
    with it. It occupies exactly 1 tab space.
    """
    
    def __init__(self, filePath):
        self.initialised = False
        self.active = False
        
        self.filePath = os.path.normpath(filePath)
        
        self.tabWidget = None
        self.tabLabel = os.path.basename(self.filePath)
    
    def initialise(self, mainWindow):
        """This method loads everything up so this editor is ready to be switched to"""
        
        assert(not self.initialised)
        assert(self.tabWidget)
        
        self.mainWindow = mainWindow
        self.tabWidget.tabbedEditor = self
                
        self.mainWindow.tabs.addTab(self.tabWidget, self.tabLabel)
    
        # we have to subscribe to the QTabWidget's signals so we know when
        # tabs are activated/deactivated and when to close them
    
        self.initialised = True
    
    def finalise(self):
        """Cleans up after itself and removes itself from the tab list
        this is usually called when you want the tab  closed
        """
        
        assert(self.initialised)
        assert(self.tabWidget)
        
        i = 0
        wdt = self.mainWindow.tabs.widget(i)
        tabRemoved = False
        
        while wdt:
            if wdt == self.tabWidget:
                self.mainWindow.tabs.removeTab(i)
                tabRemoved = True
                break
                        
            i = i + 1
            wdt = self.mainWindow.tabs.widget(i)
        
        assert(tabRemoved)
        
        self.initialised = False
    
    def activate(self):
        """The tab gets "on stage", it's been clicked on and is now the only active
        tab. There can be either 0 tabs active (blank screen) or exactly 1 tab
        active.
        """
        
        currentActive = self.mainWindow.activeEditor
        
        # no need to deactivate and then activate again
        if currentActive == self:
            return
        
        if currentActive != None:
            currentActive.deactivate()
        
        self.active = True

        self.mainWindow.activeEditor = self
        
    def deactivate(self):
        """The tab gets "off stage", user switched to another tab.
        This is also called when user closes the tab (deactivate and then finalise
        is called).
        """
        
        self.active = False
        
        if self.mainWindow.activeEditor == self:
            self.mainWindow.activeEditor = None
    
    def makeCurrent(self):
        """Makes this tab editor current (= the selected tab)"""
        
        # (this should automatically handle the respective deactivate and activate calls)   
        
        self.mainWindow.tabs.setCurrentWidget(self.tabWidget)
 
    def hasChanges(self):
        """Checks whether this TabbedEditor contains changes
        (= it should be saved before closing it)"""
        
        return False

    def saveChanges(self):
        """Causes the tabbed editor to save all it's progress to the file"""
        pass

    def discardChanges(self):
        """Causes the tabbed editor to discard all it's progress"""
        
        # early out
        if not self.hasChanges():
            return
        
        # the default but kind of wasteful implementation
        
        # we better store this because it's not specified anywhere that the
        # tabbed editor shouldn't set it to None when finalising, etc...
        mainWindow = self.mainWindow
        
        self.deactivate()
        self.finalise()
        
        self.initialise(mainWindow)
        self.activate()
        
        # the state of the tabbed editor should be valid at this point
        
    def undo(self):
        pass
        
    def redo(self):
        pass

class TabbedEditorFactory(object):
    """Constructs instances of TabbedEditor (multiple instances of one TabbedEditor
    can coexist - user editing 2 layouts for example - with the ability to switch
    from one to another) 
    """
    
    def canEditFile(self, filePath):
        """This checks whether instance created by this factory can edit given file"""
        return False

    def create(self, filePath):
        """Creates the respective TabbedEditor instance
    
        This should only be called with a filePath the factory reported
        as editable by the instances
        """
        
        return None
        
    # note: destroy doesn't really make sense as python is reference counted
    #       and everything is garbage collected

class MessageTabbedEditor(TabbedEditor):
    """This is basically a stub tabbed editor, it simply displays a message
    and doesn't allow any sort of editing at all, all functionality is stubbed

    This is for internal use only so there is no factory for this particular editor
    """
    def __init__(self, filePath, message):
        from PySide.QtGui import QLabel
        
        super(MessageTabbedEditor, self).__init__(filePath)
        
        self.message = message
        self.tabWidget = QLabel(self.message)
        self.tabWidget.setWordWrap(True)
        
    def hasChanges(self):
        return False
