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

from xml.etree import ElementTree
import ui.projectmanager

##
# One item in the project
#
# This is usually a file or a folder
class Item(QStandardItem):
    def __init__(self, project, parent, icon, label):
        super(Item, self).__init__()
        
        self.parent = parent
        self.project = project
        
        self.setText(label)        
        self.setIcon(icon)
    
    def type(self):
        # Qt docs say we have to overload type and return something over QStandardItem.UserType
        return QStandardItem.UserType + 1
        
    ##
    # Returns path relative to the projects base directory
    def getRelativePath(self):
        return self.path
    
    ##
    # Returns absolute path of the file/folder
    def getAbsolutePath(self):
        return self.project.getAbsolutePathOf(self.getRelativePath())
    
    @staticmethod
    def loadFromElement(project, parent, element):
        item = None
        
        type = element.get("type")
        if type == "file":
            item = File(project, parent, element.get("path"))
        elif type == "folder":
            item = Folder(project, parent, element.get("name"))
            
            subItemElements = element.findall("Item")
            for subItemElement in subItemElements:
                subItem = Item.loadFromElement(project, item, subItemElement)
                item.appendRow(subItem)
                
        else:
            raise Exception("Unknown item type '%s'" % (type))
            
        return item       
    
    def saveToElement(self):
        return ElementTree.Element("Item")

##
# A file is an item that can't have any children, it is directly opened instead of
# being 
class File(Item):
    ##
    # path is the path relative to project's base dir
    def __init__(self, project, parent, path):
        import os.path
        
        # TODO: File type icons are completely independent from tabbed editors,
        #       do we want to couple them or not?
        #
        #       Project management right now has no idea about mainwindow
        #       or tabbed editing (and that's a good thing IMO)
        
        fileType = "unknown"
        if path.endswith(".font"):
            fileType = "font"
        elif path.endswith(".layout"):
            fileType = "layout"
        elif path.endswith(".imageset"):
            fileType = "imageset"
        elif path.endswith(".anim"):
            fileType = "animation"
        elif path.endswith(".scheme"):
            fileType = "scheme"
        elif path.endswith(".looknfeel"):
            fileType = "looknfeel"
        elif path.endswith(".py"):
            fileType = "python_script"
        elif path.endswith(".lua"):
            fileType = "lua_script"
        elif path.endswith(".xml"):
            fileType = "xml"
        elif path.endswith(".txt"):
            fileType = "text"
        else:
            extensions = [".png", ".jpg", ".jpeg", ".tga", ".dds"]
            
            for extension in extensions:
                if path.endswith(extension):
                    fileType = "bitmap"
                    break
        
        super(File, self).__init__(project, parent,
                                   QIcon("icons/project_items/%s.png" % (fileType)),
                                   os.path.basename(path))
        
        self.path = path
    
    def getAbsolutePath(self):
        return self.project.getAbsolutePathOf(self.path)
                
    def saveToElement(self):
        ret = super(File, self).saveToElement()
        
        ret.set("type", "file")
        ret.set("path", self.path)
        
        return ret

class Folder(Item):
    ##
    # path is the path relative to project's base dir
    def __init__(self, project, parent, name):
        super(Folder, self).__init__(project, parent,
                                     QIcon("icons/project_items/folder.png"),
                                     name)
        
        self.name = name
        
    def saveToElement(self):
        ret = super(Folder, self).saveToElement()
        
        ret.set("type", "folder")
        ret.set("name", self.name)
        
        i = 0
        while i < self.rowCount():
            subItemElement = self.child(i).saveToElement()
            ret.append(subItemElement)
            i = i + 1
        
        return ret

##
# This class encapsulates a project edited by the editor
#
# A project is basically a set of files and folders that are CEGUI related
# (.font, .imageset, .layout, ...)
class Project(QStandardItemModel):
    def __init__(self):
        super(Project, self).__init__()
        
        self.setHorizontalHeaderLabels(["Name"])
        
        self.projectFilePath = ""
        self.baseDirectory = "./"
        self.changed = True
    
    ##
    # Loads XML project file from given path (preferably absolute path)
    def load(self, path):
        tree = ElementTree.parse(path)
        
        root = tree.getroot()
        
        assert(root.get("version") == "0.8")
        self.baseDirectory = root.get("base_directory")
        if not self.baseDirectory:
            self.baseDirectory = "./"
        
        items = root.find("Items")
        
        for itemElement in items.findall("Item"):
            item = Item.loadFromElement(self, None, itemElement)
            self.appendRow(item)
            
        self.changed = False
        self.projectFilePath = path
        
    def unload(self):
        pass

    def save(self, path = ""):
        if path == "":
            path = self.projectFilePath
            self.changed = False
        
        root = ElementTree.Element("Project")
        
        # This CEED is built to conform CEGUI 0.8
        root.set("version", "0.8")
        root.set("base_directory", self.baseDirectory)
        
        items = ElementTree.SubElement(root, "Items")
        
        i = 0
        while i < self.rowCount():
            items.append(self.item(i).saveToElement())
            i = i + 1
        
        tree = ElementTree.ElementTree(root)
        tree.write(path)
    
    def hasChanges(self):
        return self.changed
    
    ##
    # Converts project relative paths to absolute paths
    def getAbsolutePathOf(self, path):
        import os
        
        absoluteBaseDirectory = os.path.join(os.path.dirname(self.projectFilePath), self.baseDirectory)
        return os.path.normpath(os.path.join(absoluteBaseDirectory, path))

class ProjectManager(QDockWidget):
    fileOpenRequested = QtCore.Signal(str)
    
    def __init__(self):
        super(ProjectManager, self).__init__()
        
        self.ui = ui.projectmanager.Ui_ProjectManager()
        self.ui.setupUi(self)
        
        self.view = self.findChild(QTreeView, "view")
        self.view.doubleClicked.connect(self.slot_itemDoubleClicked)
        
        self.setProject(None)
        
    def setProject(self, project):
        self.setEnabled(project is not None)
        
        self.view.setModel(project)
        
    def slot_itemDoubleClicked(self, modelIndex):
        if not self.view.model():
            return
        
        item = self.view.model().item(modelIndex.row(), modelIndex.column())
        if isinstance(item, File): # only react to files, expanding folders is handled by Qt
            self.fileOpenRequested.emit(item.getAbsolutePath())
