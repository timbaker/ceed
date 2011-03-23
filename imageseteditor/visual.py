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
from PySide.QtCore import *

from xml.etree import ElementTree

import undo

import ui.imageseteditor.dockwidget

class ImageLabel(QGraphicsTextItem):
    def __init__(self, parent):
        super(ImageLabel, self).__init__()
        
        self.parent = parent
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIgnoresTransformations)
        self.setOpacity(0.8)
        
    def paint(self, painter, option, widget):
        painter.fillRect(self.boundingRect(), QColor(Qt.white))
        painter.drawRect(self.boundingRect())
        
        super(ImageLabel, self).paint(painter, option, widget)

class ImageOffset(QGraphicsPixmapItem):
    def __init__(self, parent):
        super(ImageOffset, self).__init__()
        
        self.parent = parent
        
        self.setAcceptsHoverEvents(True)
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIsMovable |
                      QGraphicsItem.ItemIsSelectable | 
                      QGraphicsItem.ItemIgnoresTransformations |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.setPixmap(QPixmap("icons/imageset_editing/offset_crosshair.png"))
        self.setOffset(-7, -7)
        self.setZValue(1)
        
        self.isHovered = False
        
        # used for undo
        self.potentialMove = False
        self.oldPosition = None
        
        self.setShapeMode(QGraphicsPixmapItem.BoundingRectShape)
        self.setVisible(False)
        
    def itemChange(self, change, value):    
        if change == QGraphicsItem.ItemPositionChange:
            if self.potentialMove and not self.oldPosition:
                self.oldPosition = self.pos()
            
            newPosition = value
            
            # now round the position to pixels
            newPosition.setX(round(newPosition.x() - 0.5) + 0.5)
            newPosition.setY(round(newPosition.y() - 0.5) + 0.5)

            return newPosition
        
        elif change == QGraphicsItem.ItemSelectedChange:
            if not value:
                if not self.parent.isSelected():
                    self.setVisible(False)
            else:
                self.setVisible(True)
        
        return super(ImageOffset, self).itemChange(change, value)

    def hoverEnterEvent(self, event):
        super(ImageOffset, self).hoverEnterEvent(event)
        
        self.isHovered = True
    
    def hoverLeaveEvent(self, event):
        self.isHovered = False

        super(ImageOffset, self).hoverLeaveEvent(event)

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
                      QGraphicsItem.ItemIsSelectable |
                      #QGraphicsItem.ItemClipsChildrenToShape |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.name = "Unknown"
        
        self.label = ImageLabel(self)
        self.offset = ImageOffset(self)

    def loadFromElement(self, element):
        self.name = element.get("Name", "Unknown")
        
        self.setPos(float(element.get("XPos", 0)), float(element.get("YPos", 0)))
        self.setRect(0, 0,
                     float(element.get("Width", 1)), float(element.get("Height", 1))
        )
        
        self.label.setPlainText(self.name)
        self.label.setVisible(False)
        
        self.offset.setPos(-float(element.get("XOffset", 0)) + 0.5, -float(element.get("YOffset", 0)) + 0.5)
        
    def saveToElement(self):
        ret = ElementTree.Element("Image")
        
        ret.set("Name", self.name)
        
        ret.set("XPos", str(int(self.pos().x())))
        ret.set("YPos", str(int(self.pos().y())))
        ret.set("Width", str(int(self.rect().width())))
        ret.set("Height", str(int(self.rect().height())))
        
        xoffset = int(-(self.offset.pos().x() - 0.5))
        yoffset = int(-(self.offset.pos().y() - 0.5))
        # we write none or both
        if xoffset != 0 or yoffset != 0:
            ret.set("XOffset", str(xoffset))
            ret.set("YOffset", str(yoffset))

        return ret

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemSelectedHasChanged:
            if value:
                self.label.setVisible(True)
                
                if self.parent.showOffsets:
                    self.offset.setVisible(True)
                
                self.setZValue(self.zValue() + 1)
            else:
                if not self.isHovered:
                    self.label.setVisible(False)
                
                if not self.offset.isSelected() and not self.offset.isHovered:
                    self.offset.setVisible(False)
                    
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
        
        # TODO: very unreadable
        self.parent.parent.parent.mainWindow.statusBar().showMessage("Image: '%s'\t\tXPos: %i, YPos: %i, Width: %i, Height: %i" %
                                                              (self.name, self.pos().x(), self.pos().y(), self.rect().width(), self.rect().height()))
        
        self.isHovered = True
    
    def hoverLeaveEvent(self, event):
        # TODO: very unreadable
        self.parent.parent.parent.mainWindow.statusBar().clearMessage()
        
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
        
        self.imageFile = ""
        
        self.setShapeMode(QGraphicsPixmapItem.BoundingRectShape)
        
        self.parent = parent
        self.imageEntries = []
        
        self.showOffsets = False
        
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
        self.imageFile = element.get("Imagefile", "")
        self.setPixmap(QPixmap("datafiles/imagesets/%s" % (self.imageFile)))
        self.transparencyBackground.setRect(self.boundingRect())
        
        for imageElement in element.findall("Image"):
            image = ImageEntry(self)
            image.loadFromElement(imageElement)
            self.imageEntries.append(image)
    
    def saveToElement(self):
        ret = ElementTree.Element("Imageset")
        
        ret.set("Imagefile", self.imageFile)
        
        for image in self.imageEntries:
            ret.append(image.saveToElement())
            
        return ret

class ImagesetEditorDockWidget(QDockWidget):
    """Provides list of images, property editing of currently selected image and create/delete
    """
    
    def __init__(self, parent):
        super(ImagesetEditorDockWidget, self).__init__()
        
        self.parent = parent
        
        self.ui = ui.imageseteditor.dockwidget.Ui_DockWidget()
        self.ui.setupUi(self)
        
        self.list = self.findChild(QListWidget, "list")
        self.list.itemSelectionChanged.connect(self.slot_itemSelectionChanged)
        self.list.setSelectionMode(QAbstractItemView.SelectionMode.ExtendedSelection)
        
    def setImagesetEntry(self, imagesetEntry):
        self.imagesetEntry = imagesetEntry
        
    def refresh(self):
        self.list.clear()
        
        for imageEntry in self.imagesetEntry.imageEntries:
            self.list.addItem(imageEntry.name)

    def slot_itemSelectionChanged(self):
        self.parent.scene.clearSelection()
        
        imageEntryNames = self.list.selectedItems()
        for imageEntryName in imageEntryNames:
            imageEntry = self.parent.imagesetEntry.getImageEntry(imageEntryName.text())
            imageEntry.setSelected(True)
            
        if len(imageEntryNames) == 1:
            imageEntry = self.parent.imagesetEntry.getImageEntry(imageEntryNames[0].text())
            self.parent.centerOn(imageEntry)

class VisualEditing(QGraphicsView):
    def __init__(self, parent):
        self.scene = QGraphicsScene()
        super(VisualEditing, self).__init__(self.scene)
        
        self.parent = parent

        self.setDragMode(QGraphicsView.RubberBandDrag)
        self.setBackgroundBrush(QBrush(Qt.lightGray))
        
        self.zoomFactor = 1.0
        self.imagesetEntry = None
        
        self.setupToolBar()
        self.dockWidget = ImagesetEditorDockWidget(self)
        
    def setupToolBar(self):
        self.toolBar = QToolBar()
        
        self.editOffsetsAction = QAction(QIcon("icons/imageset_editing/edit_offsets.png"), "Show and edit offsets", self.toolBar)
        self.editOffsetsAction.setCheckable(True)
        self.editOffsetsAction.toggled.connect(self.slot_toggleEditOffsets)
        self.toolBar.addAction(self.editOffsetsAction)
        
        self.toolBar.addSeparator() # ---------------------------
        
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
    
    def initialise(self, rootElement):
        self.loadImagesetEntryFromElement(rootElement)
        
    def loadImagesetEntryFromElement(self, element):
        self.scene.clear()
        
        self.imagesetEntry = None
        self.imagesetEntry = ImagesetEntry(self)
        self.imagesetEntry.loadFromElement(element)
        
        boundingRect = self.imagesetEntry.boundingRect()
        boundingRect.adjust(-100, -100, 100, 100)
        self.scene.setSceneRect(boundingRect)
        
        self.scene.addItem(self.imagesetEntry)
        
        self.dockWidget.setImagesetEntry(self.imagesetEntry)
        self.dockWidget.refresh()
        
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
                
            cmd = undo.MoveCommand(self, imageNames, oldPositions, newPositions)
            self.parent.undoStack.push(cmd)
            
            # we handled this
            return True
        
        # we didn't handle this
        return False
    
    def resizeImageEntries(self, imageEntries, topLeftDelta, bottomRightDelta):
        if (topLeftDelta.manhattanLength() > 0 or bottomRightDelta.manhattanLength() > 0) and len(imageEntries) > 0:
            imageNames = []
            oldPositions = {}
            oldRects = {}
            newPositions = {}
            newRects = {}
            
            for imageEntry in imageEntries:
                imageNames.append(imageEntry.name)
                oldPositions[imageEntry.name] = imageEntry.pos()
                newPositions[imageEntry.name] = imageEntry.pos() - topLeftDelta
                oldRects[imageEntry.name] = imageEntry.rect()
                newRect = imageEntry.rect()
                newRect.setBottomRight(newRect.bottomRight() - topLeftDelta + bottomRightDelta)
                newRects[imageEntry.name] = newRect
                
            cmd = undo.GeometryChangeCommand(self, imageNames, oldPositions, oldRects, newPositions, newRects)
            self.parent.undoStack.push(cmd)
            
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
        
            # we handled this        
            return True
        
        # we didn't handle this
        return False
    
    def showEvent(self, event):
        super(VisualEditing, self).showEvent(event)
        
        self.dockWidget.setEnabled(True)
        self.toolBar.setEnabled(True)
    
    def hideEvent(self, event):
        self.dockWidget.setEnabled(False)
        self.toolBar.setEnabled(False)
        
        super(VisualEditing, self).hideEvent(event)
    
    def mousePressEvent(self, event): 
        if event.buttons() != Qt.MiddleButton:
            super(VisualEditing, self).mousePressEvent(event) 
            
            if event.buttons() & Qt.LeftButton:
                for selectedItem in self.scene.selectedItems():
                    # selectedItem could be ImageEntry or ImageOffset!                    
                    selectedItem.potentialMove = True
                    selectedItem.oldPosition = None
        else:
            self.lastMousePosition = event.pos()
    
    def mouseReleaseEvent(self, event):
        if event.buttons() != Qt.MiddleButton:
            super(VisualEditing, self).mouseReleaseEvent(event)
            
            imageNames = []
            imageOldPositions = {}
            imageNewPositions = {}
            
            offsetNames = []
            offsetOldPositions = {}
            offsetNewPositions = {}
            
            for selectedItem in self.scene.selectedItems():
                if isinstance(selectedItem, ImageEntry):
                    if selectedItem.oldPosition:
                        imageNames.append(selectedItem.name)
                        imageOldPositions[selectedItem.name] = selectedItem.oldPosition
                        imageNewPositions[selectedItem.name] = selectedItem.pos()
                        
                    selectedItem.potentialMove = False
                    selectedItem.oldPosition = None
                    
                elif isinstance(selectedItem, ImageOffset):
                    if selectedItem.oldPosition:
                        offsetNames.append(selectedItem.parent.name)
                        offsetOldPositions[selectedItem.parent.name] = selectedItem.oldPosition
                        offsetNewPositions[selectedItem.parent.name] = selectedItem.pos()
                        
                    selectedItem.potentialMove = False
                    selectedItem.oldPosition = None
            
            if len(imageNames) > 0:
                cmd = undo.MoveCommand(self, imageNames, imageOldPositions, imageNewPositions)
                self.parent.undoStack.push(cmd)
                
            if len(offsetNames) > 0:
                cmd = undo.OffsetMoveCommand(self, offsetNames, offsetOldPositions, offsetNewPositions)
                self.parent.undoStack.push(cmd)
        else:
            pass
    
    def mouseMoveEvent(self, event): 
        if event.buttons() != Qt.MiddleButton: 
            super(VisualEditing, self).mouseMoveEvent(event)
            
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
                
                if event.modifiers() & Qt.ShiftModifier:
                    handled = self.resizeImageEntries(selection, QPointF(0, 0), delta)
                else:
                    handled = self.moveImageEntries(selection, delta)
                
        elif event.key() == Qt.Key_Q:
            handled = self.cycleOverlappingImages()
                        
        if not handled:
            super(VisualEditing, self).keyReleaseEvent(event)
            
        else:
            event.accept()
            
            
    def slot_toggleEditOffsets(self, enabled):
        self.scene.clearSelection()
        
        self.imagesetEntry.showOffsets = enabled
    