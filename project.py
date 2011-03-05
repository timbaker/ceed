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

from PySide.QtGui import QIcon
from xml.etree import ElementTree

##
# One item in the project
#
# This is usually a file or a folder
class Item:
    def __init__(self, project, parent, path, icon, label):
        self.parent = parent
        self.project = project
        
        self.path = path
        self.icon = icon
        self.label = label
        
    ##
    # Returns path relative to the projects base directory
    def getProjectPath(self):
        return self.path
    
    ##
    # Returns absolute path of the file/folder
    def getAbsolutePath(self):
        return self.project.getAbsolutePathOf(self.getProjectPath())

##
# A file is an item that can't have any children, it is directly opened instead of
# being 
class File(Item):
    ##
    # path is the path relative to project's base dir
    def __init__(self, project, parent, path):
        import os.path
        
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
        
        super(File, self).__init__(project, parent, path,
                                   QIcon(":/icons/project_items/%s.png" % (fileType)),
                                   os.path.basename(path))

class Folder(Item):
    ##
    # path is the path relative to project's base dir
    def __init__(self, project, path):
        import os.path
        
        super(Folder, self).__init__(project, path,
                                     QIcon(":/icons/project_items/folder.png"),
                                     os.path.basename(path))

##
# This class encapsulates a project edited by the editor
#
# A project is basically a set of files and folders that are CEGUI related
# (.font, .imageset, .layout, ...)
class Project(object):
    def __init__(self):
        pass
    
    ##
    # Loads XML project file from given path (preferrably absolute path)
    def load(self, path):
        tree = ElementTree.parse(path)
        
        self.changed = False

    def save(self, path):
        pass
    
    def hasChanges(self):
        return self.changed
        
    ##
    # Converts project relative paths to absolute paths
    def getAbsolutePathOf(self, path):
        # TODO: stub
        return path
            