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

from xml.etree import ElementTree

import undo
import xmledit

class XMLEditing(xmledit.XMLEditWidget):
    def __init__(self, parent):
        super(XMLEditing, self).__init__()
        
        self.parent = parent
        
    def refreshFromVisual(self):
        # taken from ElementLib
        # TODO: This will have to be reused by layout editor and maybe others, refactor for reuse
        def indent(elem, level = 0, tabImpostor = "    "):
            i = "\n" + level * tabImpostor
            if len(elem):
                if not elem.text or not elem.text.strip():
                    elem.text = i + tabImpostor
                for e in elem:
                    indent(e, level+1)
                    if not e.tail or not e.tail.strip():
                        e.tail = i + tabImpostor
                if not e.tail or not e.tail.strip():
                    e.tail = i
            else:
                if level and (not elem.tail or not elem.tail.strip()):
                    elem.tail = i
        
        element = self.parent.visual.imagesetEntry.saveToElement()
        indent(element)
        self.setText(ElementTree.tostring(element, "utf-8"))
        
    def propagateChangesToVisual(self):
        element = ElementTree.fromstring(self.document().toPlainText())
        
        self.parent.visual.loadImagesetEntryFromElement(element)
    
    def showEvent(self, event):
        self.refreshFromVisual()
        
        super(XMLEditing, self).showEvent(event)
        
    def hideEvent(self, event):
        super(XMLEditing, self).hideEvent(event)
        
        self.propagateChangesToVisual()
        