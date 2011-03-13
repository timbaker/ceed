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

class PropertyInspectorMapping(object):
    """Maps a CEGUI::Property (by origin and name) to a PropertyInspector to allow
    its viewing and editing"""
    
    def __init__(self, propertyOrigin = "", propertyName = "", targetInspectorName = "", targetInspectorSettings = ""):
        self.propertyOrigin = propertyOrigin
        self.propertyName = propertyName
        self.targetInspectorName = targetInspectorName
        self.targetInspectorSettings = targetInspectorSettings
    
    def loadFromElement(self, element):
        pass
    
    def saveToElement(self, element):
        pass

class PropertyInspector(object):
    """Interface class, derived classes implement the actual editing and viewing
    of properties
    """
    
    def canEdit(self, property):
        return False
    
    def getName(self):
        return None
    
    def createEditWidget(self, parent, mapping):
        pass
    
    def populateEditWidget(self, widget, property, mapping):
        pass
    
class CheckboxPropertyInspector(PropertyInspector):
    def canEdit(self, property):
        return property.getDataType() == "bool"
    
    def getName(self):
        return "Checkbox"
    
    def createEditWidget(self, parent):
        return QCheckBox(parent)
    
    def setEditWidgetData(self, widget, propertyEntry):
        widget.setChecked(propertyEntry.getCurrentValue() == "True")

class SliderPropertyInspector(PropertyInspector):
    def canEdit(self):
        dataTypes = ["int", "float"]
        return property.getDataType() in dataTypes
    
    def getName(self):
        return "Slider"
    
    def createEditWidget(self, parent, propertyEntry):
        return QSlider(parent)
    
    def setEditWidgetData(self, widget, propertyEntry):
        pass
    
class PropertyInspectorManager(object):
    pass