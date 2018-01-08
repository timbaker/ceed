/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CEED_editors_property_mappings___init___
#define CEED_editors_property_mappings___init___

#if 0 // commented out in Python version

#include "CEEDBase.h"

#include "editors/__init__.h"
#include "propertysetinspector.h"
#include "compatibility/property_mappings/__init__.h"

#include "elementtree.h"


##
# Property mapping file editor
class PropertyMappingsTabbedEditor : public editors.UndoStackTabbedEditor
    def __init__(self, filePath):

        super(PropertyMappingsTabbedEditor, self).__init__(property_mappings_compatibility.manager, filePath)

        self.tabWidget = QTableView()
        self.tabWidget.setDragDropMode(QAbstractItemView.InternalMove)
        self.tabWidget.setDragDropOverwriteMode(false)
        self.tabWidget.setSelectionBehavior(QTableView.SelectionBehavior.SelectRows)
        #self.tabWidget.setRootIsDecorated(false)

        self.propertyMappingList = None

    def initialise(self, mainWindow):
        super(PropertyMappingsTabbedEditor, self).initialise(mainWindow)

        if self.nativeData != "":
            self.propertyMappingList = propertyinspector.PropertyInspectorMappingList()
            self.propertyMappingList.loadFromElement(ElementTree.fromstring(self.nativeData))

            self.tabWidget.setModel(self.propertyMappingList)

    def finalise()
        super(PropertyMappingsTabbedEditor, self).finalise()

class PropertyMappingsTabbedEditorFactory : public editors.TabbedEditorFactory
    def getFileExtensions()
        extensions = set(["pmappings"])
        return extensions

    def canEditFile(self, filePath):
        extensions = self.getFileExtensions()

        for extension in extensions:
            if filePath.endswith("." + extension):
                return true

        return false

    def create(self, filePath):
        return PropertyMappingsTabbedEditor(filePath)


#endif
#endif
