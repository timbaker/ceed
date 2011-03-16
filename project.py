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

import os.path
from xml.etree import ElementTree

import qtwidgets

import ui.projectmanager
import ui.newprojectdialog
import ui.projectsettingsdialog

class Item(QStandardItem):
    """One item in the project
    This is usually a file or a folder
    """
    
    def __init__(self, project, parent, icon, label):
        super(Item, self).__init__()
        
        self.parent = parent
        self.project = project
        
        self.setText(label)        
        self.setIcon(icon)
    
    def type(self):
        # Qt docs say we have to overload type() and return something > QStandardItem.UserType
        return QStandardItem.UserType + 1
        
    def getRelativePath(self):
        """Returns path relative to the projects base directory"""
        return self.path
    
    def getAbsolutePath(self):
        """Returns absolute path of the file/folder"""
        
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

class File(Item):
    """A file is an item that can't have any children, it is directly opened instead of
    being expanded/collapsed like folders
    """
    
    def __init__(self, project, parent, path):
        """path is the path relative to project's base dir"""
        
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
    def __init__(self, project, parent, name):
        """path is the path relative to project's base dir"""
        
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

class Project(QStandardItemModel):
    """This class encapsulates a project edited by the editor

    A project is basically a set of files and folders that are CEGUI related
    (.font, .layout, ...)
    """
    
    def __init__(self):
        super(Project, self).__init__()
        
        self.setHorizontalHeaderLabels(["Name"])
        
        self.name = "Unknown"
        self.projectFilePath = ""
        self.baseDirectory = "./"
        
        self.imagesetsPath = "./imagesets"
        self.fontsPath = "./fonts"
        self.looknfeelsPath = "./looknfeel"
        self.schemesPath = "./schemes"
        self.layoutsPath = "./layouts"
        
        self.changed = True

    def load(self, path):
        """Loads XML project file from given path (preferably absolute path)"""
        
        tree = ElementTree.parse(path)
        root = tree.getroot()
        
        assert(root.get("version") == "0.8")
        
        self.name = root.get("name", "Unknown")
            
        self.baseDirectory = root.get("base_directory", "./")
        
        self.imagesetsPath = root.get("imagesets_path", "./imagesets")            
        self.fontsPath = root.get("fonts_path", "./fonts")            
        self.looknfeelsPath = root.get("looknfeels_path", "./looknfeel")            
        self.schemesPath = root.get("schemes_path", "./schemes")            
        self.layoutsPath = root.get("layouts_path", "./layouts")
        
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
        root.set("name", self.name)
        root.set("base_directory", self.baseDirectory)
        
        root.set("imagesets_path", self.imagesetsPath)
        root.set("fonts_path", self.fontsPath)
        root.set("looknfeels_path", self.looknfeelsPath)
        root.set("schemes_path", self.schemesPath)
        root.set("layouts_path", self.layoutsPath)
        
        items = ElementTree.SubElement(root, "Items")
        
        i = 0
        while i < self.rowCount():
            items.append(self.item(i).saveToElement())
            i = i + 1
        
        tree = ElementTree.ElementTree(root)
        tree.write(path)
    
    def hasChanges(self):
        return self.changed

    def getAbsolutePathOf(self, path):
        """Converts project relative paths to absolute paths"""
 
        absoluteBaseDirectory = os.path.join(os.path.dirname(self.projectFilePath), self.baseDirectory)
        return os.path.normpath(os.path.join(absoluteBaseDirectory, path))

class ProjectManager(QDockWidget):
    """This is basically a view of the Project model class,
    it allows browsing and (in the future) changes
    """
    
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
    
    @staticmethod    
    def getItemFromModelIndex(modelIndex):
        # todo: is this ugly thing really the way to do this?
        
        # Qt says in the docs that returned parent is never None but can be invalid if it's the root
        if not modelIndex.parent().isValid():
            # if it's invalid, we can use the indices as absolute model top level indices
            return modelIndex.model().item(modelIndex.row(), modelIndex.column())
        else:
            # otherwise, resort to recursion
            return ProjectManager.getItemFromModelIndex(modelIndex.parent()).child(modelIndex.row(), modelIndex.column())
        
    def slot_itemDoubleClicked(self, modelIndex):
        if not modelIndex.model():
            return

        item = ProjectManager.getItemFromModelIndex(modelIndex)
        if isinstance(item, File): # only react to files, expanding folders is handled by Qt
            self.fileOpenRequested.emit(item.getAbsolutePath())
            
class NewProjectDialog(QDialog):
    def __init__(self):
        super(NewProjectDialog, self).__init__()
        
        self.ui = ui.newprojectdialog.Ui_NewProjectDialog()
        self.ui.setupUi(self)
        
        self.projectName = self.findChild(QLineEdit, "projectName")
        
        self.projectFilePath = self.findChild(qtwidgets.FileLineEdit, "projectFilePath")
        self.projectFilePath.filter = "Project file (*.project)"
        self.projectFilePath.mode = qtwidgets.FileLineEdit.NewFileMode    
    
    # creates the project using data from this dialog    
    def createProject(self):
        ret = Project()
        ret.name = self.projectName.text()
        ret.projectFilePath = self.projectFilePath.text()
        
        return ret

class ProjectSettingsDialog(QDialog):
    def __init__(self, project):
        super(ProjectSettingsDialog, self).__init__()
        
        self.ui = ui.projectsettingsdialog.Ui_ProjectSettingsDialog()
        self.ui.setupUi(self)
        
        self.projectName = self.findChild(QLineEdit, "projectName")
        self.baseDirectory = self.findChild(qtwidgets.FileLineEdit, "baseDirectory")
        self.baseDirectory.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
    
        self.resourceDirectory = self.findChild(qtwidgets.FileLineEdit, "resourceDirectory")
        self.resourceDirectory.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.resourceDirectoryApplyButton = self.findChild(QPushButton, "resourceDirectoryApplyButton")
        self.resourceDirectoryApplyButton.pressed.connect(self.slot_applyResourceDirectory)
    
        self.imagesetsPath = self.findChild(qtwidgets.FileLineEdit, "imagesetsPath")
        self.imagesetsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.fontsPath = self.findChild(qtwidgets.FileLineEdit, "fontsPath")
        self.fontsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.looknfeelsPath = self.findChild(qtwidgets.FileLineEdit, "looknfeelsPath")
        self.looknfeelsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.schemesPath = self.findChild(qtwidgets.FileLineEdit, "schemesPath")
        self.schemesPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.layoutsPath = self.findChild(qtwidgets.FileLineEdit, "layoutsPath")
        self.layoutsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        
        self.projectName.setText(project.name)
        self.baseDirectory.setText(project.getAbsolutePathOf(""))
        self.imagesetsPath.setText(project.getAbsolutePathOf(project.imagesetsPath))
        self.fontsPath.setText(project.getAbsolutePathOf(project.fontsPath))
        self.looknfeelsPath.setText(project.getAbsolutePathOf(project.looknfeelsPath))
        self.schemesPath.setText(project.getAbsolutePathOf(project.schemesPath))
        self.layoutsPath.setText(project.getAbsolutePathOf(project.layoutsPath))
        
    def apply(self, project):
        project.name = self.projectName.text()
        absBaseDir = os.path.normpath(os.path.abspath(self.baseDirectory.text()))
        project.baseDirectory = os.path.relpath(absBaseDir, os.path.dirname(project.projectFilePath))
        
        project.imagesetsPath = os.path.relpath(self.imagesetsPath.text(), absBaseDir)
        project.fontsPath = os.path.relpath(self.fontsPath.text(), absBaseDir)
        project.looknfeelsPath = os.path.relpath(self.looknfeelsPath.text(), absBaseDir)
        project.schemesPath = os.path.relpath(self.schemesPath.text(), absBaseDir)
        project.layoutsPath = os.path.relpath(self.layoutsPath.text(), absBaseDir)
        
    def slot_applyResourceDirectory(self):
        resourceDir = os.path.normpath(os.path.abspath(self.resourceDirectory.text()))
        
        self.imagesetsPath.setText(os.path.join(resourceDir, "imagesets"))
        self.fontsPath.setText(os.path.join(resourceDir, "fonts"))
        self.looknfeelsPath.setText(os.path.join(resourceDir, "looknfeel"))
        self.schemesPath.setText(os.path.join(resourceDir, "schemes"))
        self.layoutsPath.setText(os.path.join(resourceDir, "layouts"))
        