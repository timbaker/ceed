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
from xml.etree import ElementTree

##
# One item in the project
#
# This is usually a file or a folder
class Item(object):
    def __init__(self, type, project, parent, icon, label):
        self.type = type
        
        self.parent = parent
        self.project = project
        
        self.icon = icon
        self.label = label
        
        self.treeItem = None
        self.openedTabEditor = None
        
    ##
    # Returns path relative to the projects base directory
    def getProjectPath(self):
        return self.path
    
    ##
    # Returns absolute path of the file/folder
    def getAbsolutePath(self):
        return self.project.getAbsolutePathOf(self.getProjectPath())
    
    def createTreeItem(self):
        ret = QTreeWidgetItem()
        ret.setText(0, self.label)
        ret.setIcon(0, self.icon)
        ret.item = self
        
        return ret
    
    def getTreeItem(self):
        if not self.treeItem:
            self.treeItem = self.createTreeItem()
            
        return self.treeItem
    
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
                item.subItems.append(subItem)
                
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
        
        super(File, self).__init__("file", project, parent,
                                   QIcon("icons/project_items/%s.png" % (fileType)),
                                   os.path.basename(path))
        
        self.path = path
    
    def createTreeItem(self):
        ret = super(File, self).createTreeItem()
        
        return ret
                
    def saveToElement(self):
        ret = super(File, self).saveToElement()
        
        ret.set("type", "file")
        ret.set("path", self.path)
        
        return ret

class Folder(Item):
    ##
    # path is the path relative to project's base dir
    def __init__(self, project, parent, name):
        super(Folder, self).__init__("folder", project, parent,
                                     QIcon("icons/project_items/folder.png"),
                                     name)
        
        self.name = name
        self.subItems = []
        
    def createTreeItem(self):
        ret = super(Folder, self).createTreeItem()
        
        for item in self.subItems:
            treeItem = item.getTreeItem()
            ret.addChild(treeItem)
        
        return ret
        
    def saveToElement(self):
        ret = super(Folder, self).saveToElement()
        
        ret.set("type", "folder")
        ret.set("name", self.name)
        
        for subItem in self.subItems:
            subItemElement = subItem.saveToElement()
            ret.append(subItemElement)
        
        return ret

##
# This class encapsulates a project edited by the editor
#
# A project is basically a set of files and folders that are CEGUI related
# (.font, .imageset, .layout, ...)
class Project(object):
    def __init__(self):
        self.rootItems = []
        self.projectFilePath = ""
        self.baseDir = "./"
        self.changed = True
    
    ##
    # Loads XML project file from given path (preferably absolute path)
    def load(self, path):
        tree = ElementTree.parse(path)
        
        root = tree.getroot()
        
        assert(root.get("version") == "0.8")
        self.baseDir = root.get("base_dir") if not None else "./"
        
        items = root.find("Items")
        
        for itemElement in items.findall("Item"):
            item = Item.loadFromElement(self, None, itemElement)
            self.rootItems.append(item)
            
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
        root.set("base_dir", self.baseDir)
        
        items = ElementTree.SubElement(root, "Items")
        
        for item in self.rootItems:
            items.append(item.saveToElement())
        
        tree = ElementTree.ElementTree(root)
        tree.write(path)
    
    def hasChanges(self):
        return self.changed
    
    ##
    # Converts project relative paths to absolute paths
    def getAbsolutePathOf(self, path):
        import os
        
        absoluteBaseDir = os.path.join(os.path.dirname(self.projectFilePath), self.baseDir)
        return os.path.normpath(os.path.join(absoluteBaseDir, path))
    
    def syncProjectTree(self, projectTree):
        projectTree.clear()
        
        for items in self.rootItems:
            projectTree.addTopLevelItem(items.getTreeItem())
