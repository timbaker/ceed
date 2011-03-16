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

from PySide import QtCore
from PySide.QtGui import *

import propertyinspector

import ui.propertysetinspector

# Unix like wildcard matching for property filtering
import fnmatch

class PropertyValue(QStandardItem):
    """Standard item displaying and holding the value.
    This is displayed next to a PropertyEntry
    """
    
    def __init__(self, parent):
        """parent is the parent property entry
        """
        
        super(PropertyValue, self).__init__()
        
        self.parent = parent
        self.setText(self.parent.getCurrentValue())
        self.setEditable(True)
        
class PropertyEntry(QStandardItem):
    """Standard item displaying the name of a property
    """
        
    def __init__(self, parent, property):
        """parent is the parent property category,
        property is the property of this entry (the actual CEGUI::Property instance)
        """
        
        super(PropertyEntry, self).__init__()
        
        self.parent = parent
        self.property = property
        self.setText(property.getName())
        self.setEditable(False)
        
        font = QFont()
        font.setPixelSize(14)
        self.setData(font, QtCore.Qt.FontRole)
        
        assert(self.parent.text() == property.getOrigin())
        
        self.value = PropertyValue(self)
        
    def getPropertySet(self):
        return self.parent.getPropertySet()
    
    def getCurrentValue(self):
        return self.property.get(self.getPropertySet())

class PropertyCategory(QStandardItem):
    """Groups properties of the same origin
    """ 
    
    def __init__(self, parent, origin):
        """parent is the parent property editor (property categories shouldn't be nested),
        origin is the origin that all the properties in this category have
        """
        
        super(PropertyCategory, self).__init__()
        
        self.parent = parent
        self.setText(origin)
        self.setEditable(False)
        
        self.setData(QBrush(QtCore.Qt.GlobalColor.lightGray), QtCore.Qt.BackgroundRole)
        font = QFont()
        font.setBold(True)
        font.setPixelSize(16)
        self.setData(font, QtCore.Qt.FontRole)
        
        self.propertyCount = QStandardItem()
        self.propertyCount.setData(QBrush(QtCore.Qt.GlobalColor.lightGray), QtCore.Qt.BackgroundRole)
        self.propertyCount.setEditable(False)
        
    def getPropertySet(self):
        return self.parent.getPropertySet()
        
    def setFilterMatched(self, matched):
        if matched:
            self.setData(QBrush(QtCore.Qt.GlobalColor.black), QtCore.Qt.ForegroundRole)
        else:
            self.setData(QBrush(QtCore.Qt.GlobalColor.gray), QtCore.Qt.ForegroundRole)
        
    def filterProperties(self, filter):
        toShow = []
        toHide = []
        
        matches = 0
        
        i = 0
        while i < self.rowCount():
            propertyEntry = self.child(i, 0)
            
            match = fnmatch.fnmatch(propertyEntry.text(), filter)
            
            if match:
                matches += 1
                toShow.append(propertyEntry)
            else:
                toHide.append(propertyEntry)
            
            i += 1
 
        self.setFilterMatched(matches > 0)
        if filter != "*":
            self.propertyCount.setText("%i matches" % matches)
        else:
            self.propertyCount.setText("%i properties" % matches)
            
        return toShow, toHide

class PropertySetInspectorDelegate(QItemDelegate):
    """Qt model/view delegate that allows delegating editing and viewing to
    PropertyInspectors
    """
    
    def __init__(self, setInspector):
        super(PropertySetInspectorDelegate, self).__init__()
        
        self.setInspector = setInspector
    
    def createEditor(self, parent, option, index):
        propertyEntry = self.setInspector.getPropertyEntry(index)
        inspector, mapping = propertyinspector.PropertyInspectorManager.getInspectorAndMapping(propertyEntry.property)
        
        ret = inspector.createEditWidget(parent, propertyEntry, mapping)
    
        return ret
    
    def setEditorData(self, editorWidget, index):
        propertyEntry = self.setInspector.getPropertyEntry(index)
        if propertyEntry is None:
            return

        editorWidget.inspector.populateEditWidget(editorWidget, propertyEntry, editorWidget.mapping)
        
    def setModelData(self, editorWidget, model, index):
        propertyEntry = self.setInspector.getPropertyEntry(index)
        if propertyEntry is None:
            return
        
        editorWidget.inspector.notifyEditingEnded(editorWidget, propertyEntry, editorWidget.mapping)

class PropertySetInspector(QWidget):
    """Allows browsing and editing of any CEGUI::PropertySet derived class
    """
    
    propertyEditingStarted = QtCore.Signal(str)
    propertyEditingEnded = QtCore.Signal(str, str, str)
    propertyEditingProgress = QtCore.Signal(str, str)
    
    def __init__(self):
        super(PropertySetInspector, self).__init__()
        
        self.ui = ui.propertysetinspector.Ui_PropertySetInspector()
        self.ui.setupUi(self)
        
        self.filterBox = self.findChild(QLineEdit, "filterBox")
        self.filterBox.textChanged.connect(self.filterChanged)
        
        self.view = self.findChild(QTreeView, "view")
        self.model = QStandardItemModel()
        self.model.setColumnCount(2)
        self.model.setHorizontalHeaderLabels(["Name", "Value"])
        self.view.setItemDelegate(PropertySetInspectorDelegate(self))
        self.view.setModel(self.model)
        
        #wnd = PyCEGUI.FrameWindow("type", "name")
        #self.setPropertySet(wnd)
        
    def getPropertyEntry(self, index):
        if not index.parent().isValid():
            # definitely not a property entry if it doesn't have a parent
            return None
        
        category = self.model.item(index.parent().row(), 0)
        return category.child(index.row(), 0)
        
    def setPropertySet(self, set):
        self.propertySet = set
        
        self.model.clear()
        
        propertyIt = self.propertySet.getPropertyIterator()
        propertiesByOrigin = {}
        
        while not propertyIt.isAtEnd():
            property = propertyIt.getCurrentValue()
            origin = property.getOrigin()
            
            if not propertiesByOrigin.has_key(origin):
                propertiesByOrigin[origin] = []
                
            propertiesByOrigin[origin].append(property)
            
            propertyIt.next()
        
        for origin, properties in propertiesByOrigin.iteritems():
            category = PropertyCategory(self, origin)
            
            self.model.appendRow([category, category.propertyCount])
            
            propertyCount = 0
            for property in properties:
                entry = PropertyEntry(category, property)
                category.appendRow([entry, entry.value])
                propertyCount += 1
                
            category.propertyCount.setText("%i properties" % propertyCount)
                
        self.view.expandAll()
        
    def getPropertySet(self):
        return self.propertySet
                
    def filterChanged(self, filter):
        # we append star at the end by default (makes property filtering much more practical)
        filter = filter + "*"
        
        i = 0
        while i < self.model.rowCount():
            category = self.model.item(i, 0)
            toShow, toHide = category.filterProperties(filter)
            
            for entry in toShow:
                self.view.setRowHidden(entry.index().row(), entry.index().parent(), False)
                
            for entry in toHide:
                self.view.setRowHidden(entry.index().row(), entry.index().parent(), True)
            
            i += 1
            
        # TODO: Qt doesn't redraw the items right (it leaves gray marks)
        #       This is a Qt bug but perhaps we can find a workaround?
        