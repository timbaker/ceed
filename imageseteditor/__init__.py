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

from PySide.QtCore import *
from PySide.QtGui import *

import tab

import undo
import editing

from xml.etree import ElementTree

class ImagesetTabbedEditor(tab.TabbedEditor, QGraphicsView):
    def __init__(self, filePath):
        tab.TabbedEditor.__init__(self, filePath)
        
        self.undoStack = QUndoStack()
        self.undoStack.canUndoChanged.connect(self.slot_undoAvailable)
        self.undoStack.canRedoChanged.connect(self.slot_redoAvailable)
        
        self.scene = QGraphicsScene()
        QGraphicsView.__init__(self, self.scene)
        self.setDragMode(QGraphicsView.RubberBandDrag)
        self.setBackgroundBrush(QBrush(Qt.lightGray))
        
        self.zoomFactor = 1.0
        self.imagesetEntry = None
        
        self.tabWidget = self
        
        self.setupToolBar()
    
    def setupToolBar(self):
        self.toolBar = QToolBar()
        
        self.cycleOverlappingAction = QAction(QIcon("icons/imageset_editing/cycle_overlapping.png"), "Cycle overlapping images (Q)", self.toolBar)
        self.cycleOverlappingAction.triggered.connect(self.cycleOverlappingImages)
        self.toolBar.addAction(self.cycleOverlappingAction)
        
        self.toolBar.addSeparator() # ---------------------------
        
        self.zoomOriginalAction = QAction(QIcon("icons/imageset_editing/zoom_original.png"), "Zoom original", self.toolBar)
        self.zoomOriginalAction.triggered.connect(self.zoomOriginal)
        self.toolBar.addAction(self.zoomOriginalAction)
        
        self.zoomInAction = QAction(QIcon("icons/imageset_editing/zoom_in.png"), "Zoom in (mouse wheel)", self.toolBar)
        self.zoomInAction.triggered.connect(self.zoomIn)
        self.toolBar.addAction(self.zoomInAction)
        
        self.zoomOutAction = QAction(QIcon("icons/imageset_editing/zoom_out.png"), "Zoom out (mouse wheel)", self.toolBar)
        self.zoomOutAction.triggered.connect(self.zoomOut)
        self.toolBar.addAction(self.zoomOutAction)
    
    def initialise(self, mainWindow):
        super(ImagesetTabbedEditor, self).initialise(mainWindow)
        
        tree = ElementTree.parse(self.filePath)
        root = tree.getroot()

        self.undoStack.clear()
        
        self.imagesetEntry = editing.ImagesetEntry(self)
        self.imagesetEntry.loadFromElement(root)
        
        boundingRect = self.imagesetEntry.boundingRect()
        boundingRect.adjust(-100, -100, 100, 100)
        self.scene.setSceneRect(boundingRect)
        
        self.scene.addItem(self.imagesetEntry)
    
    def finalise(self):        
        super(ImagesetTabbedEditor, self).finalise()
        
        self.tabWidget = None
    
    def activate(self):
        super(ImagesetTabbedEditor, self).activate()
        
        self.mainWindow.undoAction.setEnabled(self.undoStack.canUndo())
        self.mainWindow.redoAction.setEnabled(self.undoStack.canRedo())
        
        self.mainWindow.addToolBar(Qt.ToolBarArea.LeftToolBarArea, self.toolBar)
        self.toolBar.show()
        
    def deactivate(self):
        self.mainWindow.removeToolBar(self.toolBar)
        
        super(ImagesetTabbedEditor, self).deactivate()
        
    def hasChanges(self):
        return False
    
    def undo(self):
        self.undoStack.undo()
        
    def redo(self):
        self.undoStack.redo()
    
    def performZoom(self):
        transform = QTransform()
        transform.scale(self.zoomFactor, self.zoomFactor)
        self.setTransform(transform)
    
    def zoomOriginal(self):
        self.zoomFactor = 1
        self.performZoom()
    
    def zoomIn(self):
        self.zoomFactor *= 2
        
        if self.zoomFactor > 64:
            self.zoomFactor = 64
        
        self.performZoom()
    
    def zoomOut(self):
        self.zoomFactor /= 2
        
        if self.zoomFactor < 1:
            self.zoomFactor = 1
            
        self.performZoom()
        
    def moveImageEntries(self, imageEntries, delta):
        if delta.manhattanLength() > 0 and len(imageEntries) > 0:
            imageNames = []
            oldPositions = {}
            newPositions = {}
            
            for imageEntry in imageEntries:
                imageNames.append(imageEntry.name)
                oldPositions[imageEntry.name] = imageEntry.pos()
                newPositions[imageEntry.name] = imageEntry.pos() + delta
                
            cmd = undo.MoveCommand(self.imagesetEntry, imageNames, oldPositions, newPositions)
            self.undoStack.push(cmd)
            
            # we handled this
            return True
        
        # we didn't handle this
        return False
        
    def cycleOverlappingImages(self):
        selection = self.scene.selectedItems()
            
        if len(selection) == 1:
            rect = selection[0].boundingRect()
            rect.translate(selection[0].pos())
            
            overlappingItems = self.scene.items(rect)
        
            # first we stack everything before our current selection
            successor = None
            for item in overlappingItems:
                if item == selection[0] or item.parentItem() != selection[0].parentItem():
                    continue
                
                if not successor and isinstance(item, editing.ImageEntry):
                    successor = item
                    
            if successor:
                for item in overlappingItems:
                    if item == successor or item.parentItem() != successor.parentItem():
                        continue
                    
                    successor.stackBefore(item)
                
                # we deselect current
                selection[0].setSelected(False)
                selection[0].hoverLeaveEvent(None)
                # and select what was at the bottom (thus getting this to the top)    
                successor.setSelected(True)
                successor.hoverEnterEvent(None)
        
            # we handled this        
            return True
        
        # we didn't handle this
        return False
    
    def mousePressEvent(self, event): 
        if event.buttons() != Qt.MiddleButton:
            super(ImagesetTabbedEditor, self).mousePressEvent(event) 
            
            if event.buttons() & Qt.LeftButton:
                for imageEntry in self.scene.selectedItems():
                    imageEntry.potentialMove = True
                    imageEntry.oldPosition = None
        else:
            self.lastMousePosition = event.pos()
    
    def mouseReleaseEvent(self, event):
        if event.buttons() != Qt.MiddleButton:
            super(ImagesetTabbedEditor, self).mouseReleaseEvent(event)
            
            imageNames = []
            oldPositions = {}
            newPositions = {}
            
            for imageEntry in self.scene.selectedItems():
                if imageEntry.oldPosition:
                    imageNames.append(imageEntry.name)
                    oldPositions[imageEntry.name] = imageEntry.oldPosition
                    newPositions[imageEntry.name] = imageEntry.pos()
                    
                imageEntry.potentialMove = False
                imageEntry.oldPosition = None
            
            if len(imageNames) > 0:
                cmd = undo.MoveCommand(self.imagesetEntry, imageNames, oldPositions, newPositions)
                self.undoStack.push(cmd)
        else:
            pass
    
    def mouseMoveEvent(self, event): 
        if event.buttons() != Qt.MiddleButton: 
            super(ImagesetTabbedEditor, self).mouseMoveEvent(event)
            
        else:
            horizontal = self.horizontalScrollBar()
            horizontal.setSliderPosition(horizontal.sliderPosition() - (event.pos().x() - self.lastMousePosition.x()))
            vertical = self.verticalScrollBar()
            vertical.setSliderPosition(vertical.sliderPosition() - (event.pos().y() - self.lastMousePosition.y()))
            
            self.lastMousePosition = event.pos() 
    
    def wheelEvent(self, event):
        if event.delta() == 0:
            return
        
        if event.delta() > 0:
            self.zoomIn()
        else:
            self.zoomOut()
            
    def keyReleaseEvent(self, event):
        handled = False
        
        if event.key() in [Qt.Key_A, Qt.Key_D, Qt.Key_W, Qt.Key_S]:
            selection = self.scene.selectedItems()
            
            if len(selection) > 0:
                delta = QPointF()
                
                if event.key() == Qt.Key_A:
                    delta += QPointF(-1, 0)
                elif event.key() == Qt.Key_D:
                    delta += QPointF(1, 0)
                elif event.key() == Qt.Key_W:
                    delta += QPointF(0, -1)
                elif event.key() == Qt.Key_S:
                    delta += QPointF(0, 1)
                
                if event.modifiers() & Qt.ControlModifier:
                    delta *= 10
                
                handled = self.moveImageEntries(selection, delta)
                
        elif event.key() == Qt.Key_Q:
            handled = self.cycleOverlappingImages()
                        
        if not handled:
            super(ImagesetTabbedEditor, self).keyReleaseEvent(event)
            
        else:
            event.accept()
            
    def slot_undoAvailable(self, available):
        self.mainWindow.undoAction.setEnabled(available)
        
    def slot_redoAvailable(self, available):
        self.mainWindow.redoAction.setEnabled(available)

class ImagesetTabbedEditorFactory(tab.TabbedEditorFactory):
    def canEditFile(self, filePath):
        extensions = [".imageset"]
        
        for extension in extensions:
            if filePath.endswith(extension):
                return True
            
        return False

    def create(self, filePath):
        return ImagesetTabbedEditor(filePath)
