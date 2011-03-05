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

##
# This is the base class for a class that takes a file and allows manipulation
# with it. It occupies exactly 1 tab space.
class TabbedEditor(object):
    def __init__(self, fileName):
        self.initialised = False
        self.active = False
        
        self.fileName = fileName
        
        self.tabWidget = None
        self.tabLabel = self.fileName
    
    ##
    # This method loads everything up so this editor is ready to be switched to
    def initialise(self, mainWindow):
        assert(not self.initialised)
        assert(self.tabWidget)
        
        self.mainWindow = mainWindow
        self.tabWidget.tabbedEditor = self
                
        mainWindow.tabs.addTab(self.tabWidget, self.tabLabel)
    
        # we have to subscribe to the QTabWidget's signals so we know when
        # tabs are activated/deactivated and when to close them
    
        self.initialised = True
    
    ##
    # Cleans up after itself, this is usually called when the tab is closed    
    def finalise(self):
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
    
    ##
    # The tab gets "on stage", it's been clicked on and is now the only active
    # tab. There can be either 0 tabs active (blank screen) or exactly 1 tab
    # active.
    def activate(self):
        currentActive = self.mainWindow.activeEditor
        
        # no need to deactivate and then activate again
        if currentActive == self:
            return
        
        if currentActive != None:
            currentActive.deactivate()
        
        self.active = True

        self.mainWindow.activeEditor = self
        
    ##
    # The tab gets "off stage", user switched to another tab.
    # This is also called when user closes the tab (deactivate and then finalise
    # is called).
    def deactivate(self):
        self.active = False
        
        if self.mainWindow.activeEditor == self:
            self.mainWindow.activeEditor = None
    
    ##
    # Checks whether this TabbedEditor
    def hasChanges(self):
        return False
        
    ##
    # Causes the tabbed editor to save all it's progress to the file
    def saveChanges(self):
        pass
        
    ##
    # Causes the tabbed editor to discard all it's progress
    def discardChanges(self):
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

##
# Constructs instances of TabbedEditor (multiple instances of one TabbedEditor
# can coexist - user editing 2 layouts for example - with the ability to switch
# from one to another)        
class TabbedEditorFactory(object):
    ##
    # This checks whether instance created by this factory can edit given file
    def canEditFile(self, fileName):
        return False

    ##
    # Creates the respective TabbedEditor instance
    #
    # This should only be called with a fileName the factory reported
    # as editable by the instances
    def create(self, fileName):
        return None
        
    # note: destroy doesn't really make sense as python is reference counted
    #       and everything is garbage collected

##
# This is basically a stub tabbed editor, it simply displays a message
# and doesn't allow any sort of editing at all, all functionality is stubbed
#
# This is for internal use only so there is no factory for this editor
class MessageTabbedEditor(TabbedEditor):
    def __init__(self, fileName, message):
        from PySide.QtGui import QLabel
        
        super(MessageTabbedEditor, self).__init__(fileName)
        
        self.message = message
        self.tabWidget = QLabel(self.message)
        
    def hasChanges(self):
        return False
