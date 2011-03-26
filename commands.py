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

from PySide.QtGui import QUndoCommand, QDockWidget, QUndoView, QIcon

class UndoCommand(QUndoCommand):
    def __init__(self):
        super(UndoCommand, self).__init__()
    
class UndoViewer(QDockWidget):
    def __init__(self):
        super(UndoViewer, self).__init__()
        
        self.setWindowTitle("Undo Viewer")
        
        self.view = QUndoView()
        self.view.setCleanIcon(QIcon("icons/clean_undo_state.png"))
        self.setWidget(self.view)
        
    def setUndoStack(self, stack):
        self.view.setStack(stack)
        