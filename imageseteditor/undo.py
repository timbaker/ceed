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

from PySide.QtGui import QUndoCommand
import commands

class MoveCommand(commands.UndoCommand):
    def __init__(self, imageset, imageNames, oldPositions, newPositions):
        super(MoveCommand, self).__init__()
        
        self.imageset = imageset
        
        self.imageNames = imageNames
        self.oldPositions = oldPositions
        self.newPositions = newPositions
        
    def undo(self):
        super(MoveCommand, self).undo()
        
        for imageName in self.imageNames:
            image = self.imageset.getImageEntry(imageName)
            image.setPos(self.oldPositions[imageName])
            
    def redo(self):
        for imageName in self.imageNames:
            image = self.imageset.getImageEntry(imageName)
            image.setPos(self.newPositions[imageName])
            
        super(MoveCommand, self).redo()
        