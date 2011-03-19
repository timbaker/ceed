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

from xml.etree import ElementTree

import undo

class ImageLabel(QGraphicsTextItem):
    def __init__(self, parent):
        super(ImageLabel, self).__init__()
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIgnoresTransformations)
        self.setOpacity(0.8)
        
    def paint(self, painter, option, widget):
        painter.fillRect(self.boundingRect(), QColor(Qt.white))
        painter.drawRect(self.boundingRect())
        
        super(ImageLabel, self).paint(painter, option, widget)

class ImageEntry(QGraphicsRectItem):
    def __init__(self, parent):
        super(ImageEntry, self).__init__()
        
        self.parent = parent
        
        self.setAcceptsHoverEvents(True)
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        
        self.isHovered = False
        
        # used for undo
        self.potentialMove = False
        self.oldPosition = None
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIsMovable |
                      QGraphicsItem.ItemIsFocusable |
                      QGraphicsItem.ItemIsSelectable |
                      QGraphicsItem.ItemClipsChildrenToShape |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.name = "Unknown"
        
        self.label = ImageLabel(self)

    def loadFromElement(self, element):
        self.name = element.get("Name", "Unknown")
        
        self.setPos(float(element.get("XPos", 0)), float(element.get("YPos", 0)))
        self.setRect(0, 0,
                     float(element.get("Width", 1)), float(element.get("Height", 1))
        )
        
        self.label.setPlainText(self.name)
        self.label.setVisible(False)
        
    def saveToElement(self):
        pass

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemSelectedHasChanged:
            if value:
                self.label.setVisible(True)
                self.setZValue(self.zValue() + 1)
                #self.setFlags(self.flags() | QGraphicsItem.ItemIsMovable)
            else:
                if not self.isHovered:
                    self.label.setVisible(False)
                self.setZValue(self.zValue() - 1)

        elif change == QGraphicsItem.ItemPositionChange:
            if self.potentialMove and not self.oldPosition:
                self.oldPosition = self.pos()
            
            newPosition = value

            rect = self.parent.boundingRect()
            rect.setWidth(rect.width() - self.rect().width())
            rect.setHeight(rect.height() - self.rect().height())
            
            if not rect.contains(newPosition):
                newPosition.setX(min(rect.right(), max(newPosition.x(), rect.left())))
                newPosition.setY(min(rect.bottom(), max(newPosition.y(), rect.top())))
            
            # now round the position to pixels
            newPosition.setX(round(newPosition.x()))
            newPosition.setY(round(newPosition.y()))

            return newPosition

        return super(ImageEntry, self).itemChange(change, value)
    
    def hoverEnterEvent(self, event):
        super(ImageEntry, self).hoverEnterEvent(event)
        
        self.setZValue(self.zValue() + 1)
        
        pen = QPen()
        pen.setColor(QColor(Qt.black))
        self.setPen(pen)
        
        self.label.setVisible(True)
        self.label.setOpacity(0.3)
        
        self.parent.parent.mainWindow.statusBar().showMessage("Image: '%s'\t\tXPos: %i, YPos: %i, Width: %i, Height: %i" %
                                                              (self.name, self.pos().x(), self.pos().y(), self.rect().width(), self.rect().height()))
        
        self.isHovered = True
    
    def hoverLeaveEvent(self, event):
        self.parent.parent.mainWindow.statusBar().clearMessage()
        
        self.isHovered = False
        
        if not self.isSelected():
            self.label.setVisible(False)
        
        self.label.setOpacity(0.8)
        
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        
        self.setZValue(self.zValue() - 1)
        
        super(ImageEntry, self).hoverLeaveEvent(event)
        
class ImagesetEntry(QGraphicsPixmapItem):
    def __init__(self, parent):
        super(ImagesetEntry, self).__init__()
        
        self.setShapeMode(QGraphicsPixmapItem.BoundingRectShape)
        
        self.parent = parent
        self.imageEntries = []
        
        self.transparencyBackground = QGraphicsRectItem()
        self.transparencyBackground.setParentItem(self)
        self.transparencyBackground.setFlags(QGraphicsItem.ItemStacksBehindParent)
        
        transparentBrush = QBrush()
        transparentTexture = QPixmap(10, 10)
        transparentPainter = QPainter(transparentTexture)
        transparentPainter.fillRect(0, 0, 5, 5, QColor(Qt.darkGray))
        transparentPainter.fillRect(5, 5, 5, 5, QColor(Qt.darkGray))
        transparentPainter.fillRect(5, 0, 5, 5, QColor(Qt.gray))
        transparentPainter.fillRect(0, 5, 5, 5, QColor(Qt.gray))
        transparentPainter.end()
        transparentBrush.setTexture(transparentTexture)
        
        self.transparencyBackground.setBrush(transparentBrush)
        self.transparencyBackground.setPen(QPen(QColor(Qt.transparent)))
        
    def getImageEntry(self, name):
        for image in self.imageEntries:
            if image.name == name:
                return image
            
        assert(False)
        return None
        
    def loadFromElement(self, element):
        self.setPixmap(QPixmap("datafiles/imagesets/%s" % (element.get("Imagefile", ""))))
        self.transparencyBackground.setRect(self.boundingRect())
        
        for imageElement in element.findall("Image"):
            image = ImageEntry(self)
            image.loadFromElement(imageElement)
            self.imageEntries.append(image)
    
    def saveToElement(self, element):
        for image in self.imageEntries:
            element.append(image.saveToElement())

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
    
    def initialise(self, mainWindow):
        super(ImagesetTabbedEditor, self).initialise(mainWindow)
            
        tree = ElementTree.parse(self.filePath)
        root = tree.getroot()

        self.undoStack.clear()
        
        self.imagesetEntry = ImagesetEntry(self)
        self.imagesetEntry.loadFromElement(root)
        
        boundingRect = self.imagesetEntry.boundingRect()
        boundingRect.adjust(-100, -100, 100, 100)
        self.scene.setSceneRect(boundingRect)
        
        self.scene.addItem(self.imagesetEntry)
    
    def finalise(self):
        super(ImagesetTabbedEditor, self).finalise()
        
        self.tabWidget = None
    
    def deactivate(self):
        self.mainWindow.statusBar().clearMessage()
        
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
                dx = 0
                dy = 0
                
                if event.key() == Qt.Key_A:
                    dx -= 1
                elif event.key() == Qt.Key_D:
                    dx += 1
                elif event.key() == Qt.Key_W:
                    dy -= 1
                elif event.key() == Qt.Key_S:
                    dy += 1
                
                if event.modifiers() & Qt.ControlModifier:
                    dx *= 10
                    dy *= 10
                    
                if dx != 0 or dy != 0:
                    handled = True
                    
                    imageNames = []
                    oldPositions = {}
                    newPositions = {}
                    
                    for imageEntry in selection:
                        imageNames.append(imageEntry.name)
                        oldPositions[imageEntry.name] = imageEntry.pos()
                        imageEntry.setPos(imageEntry.pos() + QPointF(dx, dy))
                        newPositions[imageEntry.name] = imageEntry.pos()
                        
                    cmd = undo.MoveCommand(self.imagesetEntry, imageNames, oldPositions, newPositions)
                    self.undoStack.push(cmd)
                    
        elif event.key() == Qt.Key_Q:
            selection = self.scene.selectedItems()
            
            if len(selection) == 1:
                handled = True
                rect = selection[0].boundingRect()
                rect.translate(selection[0].pos())
                
                overlappingItems = self.scene.items(rect)
            
                # first we stack everything before our current selection
                successor = None
                for item in overlappingItems:
                    if item == selection[0] or item.parentItem() != selection[0].parentItem():
                        continue
                    
                    if not successor and isinstance(item, ImageEntry):
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
