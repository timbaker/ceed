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
import tab

##
# Multi purpose text editor with some rudimentary highlighting
#
# TODO: This could get replaced by QScintilla once PySide guys get it to work.
#       Scintilla would probably be overkill though, I can't imagine anyone
#       doing any serious editing in this application
class TextTabbedEditor(tab.TabbedEditor):
    def __init__(self, filePath):
        
        super(TextTabbedEditor, self).__init__(filePath)
        
        self.tabWidget = QTextEdit()
    
    def initialise(self, mainWindow):
        super(TextTabbedEditor, self).initialise(mainWindow)
            
        file = open(self.filePath, "r")
        self.textDocument = QTextDocument()
        self.textDocument.setPlainText(file.read())
        file.close()
        
        self.tabWidget.setDocument(self.textDocument)
        self.textDocument.setModified(False)
    
    def finalise(self):
        super(TextTabbedEditor, self).finalise()
        
        self.tabWidget = None
        self.textDocument = None
        
    def hasChanges(self):
        return self.textDocument.isModified()

class TextTabbedEditorFactory(tab.TabbedEditorFactory):
    def canEditFile(self, filePath):
        extensions = [".py", ".lua", ".txt", ".xml"]
        
        for extension in extensions:
            if filePath.endswith(extension):
                return True
            
        return False

    def create(self, filePath):
        return TextTabbedEditor(filePath)
