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

import commands
import copy
import math

from PySide import QtCore

idbase = 1100

class MoveCommand(commands.UndoCommand):
    """This command simply moves given images from old position to the new
    You can use GeometryChangeCommand instead and use the same rects as old new as current rects,
    this is there just to save memory.
    """
    
    def __init__(self, visual, imageNames, oldPositions, newPositions):
        super(MoveCommand, self).__init__()
        
        self.visual = visual
        
        self.imageNames = imageNames
        self.oldPositions = oldPositions
        self.newPositions = newPositions
        
        self.biggestDelta = 0
        
        for (imageName, oldPosition) in self.oldPositions.iteritems():
            positionDelta = oldPosition - self.newPositions[imageName]
            
            delta = math.sqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y())
            if delta > self.biggestDelta:
                self.biggestDelta = delta
    
        self.refreshText()
    
    def refreshText(self):            
        if len(self.imageNames) == 1:
            self.setText("Move '%s'" % (self.imageNames[0]))
        else:
            self.setText("Move %i images" % (len(self.imageNames)))
                
    def id(self):
        return idbase + 1
        
    def mergeWith(self, cmd):
        if self.imageNames == cmd.imageNames:
            # good, images match
                    
            combinedBiggestDelta = self.biggestDelta + cmd.biggestDelta
            # TODO: 50 used just for testing!
            if combinedBiggestDelta < 50:
                # if the combined delta is reasonably small, we can merge the commands
                self.newPositions = cmd.newPositions
                self.biggestDelta = combinedBiggestDelta
                
                self.refreshText()
                
                return True
            
        return False
        
    def undo(self):
        super(MoveCommand, self).undo()
        
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.setPos(self.oldPositions[imageName])
            
    def redo(self):
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.setPos(self.newPositions[imageName])
            
        super(MoveCommand, self).redo()

class GeometryChangeCommand(commands.UndoCommand):
    """Changes geometry of given images, that means that positions as well as rects might change
    Can even implement MoveCommand as a special case but would eat more RAM.
    """
    
    def __init__(self, visual, imageNames, oldPositions, oldRects, newPositions, newRects):
        super(GeometryChangeCommand, self).__init__()
        
        self.visual = visual
        
        self.imageNames = imageNames
        self.oldPositions = oldPositions
        self.oldRects = oldRects
        self.newPositions = newPositions
        self.newRects = newRects
        
        self.biggestMoveDelta = 0
        
        for (imageName, oldPosition) in self.oldPositions.iteritems():
            moveDelta = oldPosition - self.newPositions[imageName]
            
            delta = math.sqrt(moveDelta.x() * moveDelta.x() + moveDelta.y() * moveDelta.y())
            if delta > self.biggestMoveDelta:
                self.biggestMoveDelta = delta
        
        self.biggestResizeDelta = 0
        
        for (imageName, oldRect) in self.oldRects.iteritems():
            resizeDelta = oldRect.bottomRight() - self.newRects[imageName].bottomRight()
            
            delta = math.sqrt(resizeDelta.x() * resizeDelta.x() + resizeDelta.y() * resizeDelta.y())
            if delta > self.biggestResizeDelta:
                self.biggestResizeDelta = delta
    
        self.refreshText()
    
    def refreshText(self):            
        if len(self.imageNames) == 1:
            self.setText("Geometry change of '%s'" % (self.imageNames[0]))
        else:
            self.setText("Geometry change of %i images" % (len(self.imageNames)))
            
    def id(self):
        return idbase + 2
    
    def mergeWith(self, cmd):
        if self.imageNames == cmd.imageNames:
            # good, images match
                    
            combinedBiggestMoveDelta = self.biggestMoveDelta + cmd.biggestMoveDelta
            combinedBiggestResizeDelta = self.biggestResizeDelta + cmd.biggestResizeDelta
            
            # TODO: 50 and 20 are used just for testing!
            if combinedBiggestMoveDelta < 50 and combinedBiggestResizeDelta < 20:
                # if the combined deltas area reasonably small, we can merge the commands
                self.newPositions = cmd.newPositions
                self.newRects = cmd.newRects
                
                self.biggestMoveDelta = combinedBiggestMoveDelta
                self.biggestResizeDelta = combinedBiggestResizeDelta
                
                self.refreshText()
                
                return True
            
        return False
        
    def undo(self):
        super(GeometryChangeCommand, self).undo()
        
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.setPos(self.oldPositions[imageName])
            image.setRect(self.oldRects[imageName])
            
    def redo(self):
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.setPos(self.newPositions[imageName])
            image.setRect(self.newRects[imageName])
            
        super(GeometryChangeCommand, self).redo()

class OffsetMoveCommand(commands.UndoCommand):
    def __init__(self, visual, imageNames, oldPositions, newPositions):
        super(OffsetMoveCommand, self).__init__()
        
        self.visual = visual
        
        self.imageNames = imageNames
        self.oldPositions = oldPositions
        self.newPositions = newPositions
        
        self.biggestDelta = 0
        
        for (imageName, oldPosition) in self.oldPositions.iteritems():
            positionDelta = oldPosition - self.newPositions[imageName]
            
            delta = math.sqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y())
            if delta > self.biggestDelta:
                self.biggestDelta = delta
                
        self.refreshText()
                
    def refreshText(self):
        if len(self.imageNames) == 1:
            self.setText("Offset move of '%s'" % (self.imageNames[0]))
        else:
            self.setText("Offset move of %i images" % (len(self.imageNames)))
        
    def id(self):
        return idbase + 3
        
    def mergeWith(self, cmd):
        if self.imageNames == cmd.imageNames:
            # good, images match
                    
            combinedBiggestDelta = self.biggestDelta + cmd.biggestDelta
            # TODO: 10 used just for testing!
            if combinedBiggestDelta < 10:
                # if the combined delta is reasonably small, we can merge the commands
                self.newPositions = cmd.newPositions
                self.biggestDelta = combinedBiggestDelta
                
                self.refreshText()
                
                return True
            
        return False
        
    def undo(self):
        super(OffsetMoveCommand, self).undo()
        
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.offset.setPos(self.oldPositions[imageName])
            
    def redo(self):
        for imageName in self.imageNames:
            image = self.visual.imagesetEntry.getImageEntry(imageName)
            image.offset.setPos(self.newPositions[imageName])
            
        super(OffsetMoveCommand, self).redo()
        
class XMLEditingCommand(commands.UndoCommand):
    """Extremely memory hungry implementation for now, I have to figure out how to use my own
    QUndoStack with QTextDocument in the future to fix this.
    """
    def __init__(self, xmlediting, oldText, oldCursor, newText, newCursor, totalChange):
        super(XMLEditingCommand, self).__init__()
        
        self.xmlediting = xmlediting
        self.oldText = oldText
        self.oldCursor = copy.copy(oldCursor)
        self.newText = newText
        self.newCursor = copy.copy(newCursor)
        
        self.totalChange = totalChange
        
        self.dryRun = True
        
        self.refreshText()
        
    def refreshText(self):
        if self.totalChange == 1:
            self.setText("XML edit, changed 1 character")
        else:
            self.setText("XML edit, changed %i characters" % (self.totalChange))
        
    def id(self):
        return idbase + 4
        
    def mergeWith(self, cmd):
        assert(self.xmlediting == cmd.xmlediting)
        
        # TODO: 10 chars for now for testing
        if self.totalChange + cmd.totalChange < 10:
            self.totalChange += cmd.totalChange
            self.newText = cmd.newText
            self.newCursor = cmd.newCursor
            
            self.refreshText()
            
            return True
        
        return False
        
    def undo(self):
        super(XMLEditingCommand, self).undo()
        
        self.xmlediting.ignoreUndoCommands = True
        self.xmlediting.setPlainText(self.oldText)
        self.xmlediting.ignoreUndoCommands = False
        #self.xmlediting.setTextCursor(self.oldCursor)
        
    def redo(self):
        if not self.dryRun:
            self.xmlediting.ignoreUndoCommands = True
            self.xmlediting.setPlainText(self.newText)
            self.xmlediting.ignoreUndoCommands = False
            #self.xmlediting.setTextCursor(self.newCursor)
            
        self.dryRun = False

        super(XMLEditingCommand, self).redo()
        