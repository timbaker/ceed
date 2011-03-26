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
import fnmatch
import os

import undo

import ui.imageseteditor.dockwidget

class ImageLabel(QGraphicsTextItem):
    """Text item showing image's label when the image is hovered or selected.
    You should not use this directly! Use ImageEntry.name instead to get the name.    
    """
    
    def __init__(self, parent):
        super(ImageLabel, self).__init__()
        
        self.parent = parent
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIgnoresTransformations)
        self.setOpacity(0.8)
        
        self.setPlainText("Unknown")
        
        # we make the label a lot more transparent when mouse is over it to make it easier
        # to work around the top edge of the image
        self.setAcceptHoverEvents(True)
        # the default opacity (when mouse is not over the label)
        self.setOpacity(0.8)
        
    def paint(self, painter, option, widget):
        painter.fillRect(self.boundingRect(), QColor(Qt.white))
        painter.drawRect(self.boundingRect())
        
        super(ImageLabel, self).paint(painter, option, widget)
    
    def hoverEnterEvent(self, event):
        super(ImageLabel, self).hoverEnterEvent(event)
        
        self.setOpacity(0.2)
        
    def hoverLeaveEvent(self, event):
        self.setOpacity(0.8)

        super(ImageLabel, self).hoverLeaveEvent(event)

class ImageOffset(QGraphicsPixmapItem):
    """A crosshair showing where the imaginary (0, 0) point of the image is. The actual offset
    is just a negated vector of the crosshair's position but this is easier to work with from
    the artist's point of view.    
    """
    
    def __init__(self, parent):
        super(ImageOffset, self).__init__()
        
        self.parent = parent
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIsMovable |
                      QGraphicsItem.ItemIsSelectable | 
                      QGraphicsItem.ItemIgnoresTransformations |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.setPixmap(QPixmap("icons/imageset_editing/offset_crosshair.png"))
        # the crosshair pixmap is 15x15, (7, 7) is the centre pixel of it,
        # we want that to be the (0, 0) point of the crosshair
        self.setOffset(-7, -7)
        # always show this above the label (which has ZValue = 0)
        self.setZValue(1)
        
        self.setAcceptHoverEvents(True)
        # internal attribute to help decide when to hide/show the offset crosshair
        self.isHovered = False
        
        # used for undo
        self.potentialMove = False
        self.oldPosition = None
        
        # by default Qt considers parts of the image with alpha = 0 not part of the image,
        # that would make it very hard to move the crosshair, we consider the whole
        # bounding rectangle to be part of the image
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
    """Represents the image of the imageset, can be drag moved, selected, resized, ...
    """
    
    # the image's "real parameters" are properties that directly access Qt's
    # facilities, this is done to make the code cleaner and save a little memory
    
    name = property(lambda self: self.label.toPlainText(),
                    lambda self, value: self.label.setPlainText(value))
    
    xpos = property(lambda self: int(self.pos().x()),
                    lambda self, value: self.setPos(value, self.pos().y()))
    ypos = property(lambda self: int(self.pos().y()),
                    lambda self, value: self.setPos(self.pos().x(), value))
    width = property(lambda self: int(self.rect().width()),
                     lambda self, value: self.setRect(0, 0, value, self.height))
    height = property(lambda self: int(self.rect().height()),
                      lambda self, value: self.setRect(0, 0, self.width, value))
    
    xoffset = property(lambda self: int(-(self.offset.pos().x() - 0.5)),
                       lambda self, value: self.offset.setX(-float(value) + 0.5))
    yoffset = property(lambda self: int(-(self.offset.pos().y() - 0.5)),
                       lambda self, value: self.offset.setY(-float(value) + 0.5))
    
    def __init__(self, parent):
        super(ImageEntry, self).__init__()
        
        self.parent = parent
        
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        
        self.setAcceptsHoverEvents(True)
        self.isHovered = False
        
        # used for undo
        self.potentialMove = False
        self.oldPosition = None
        
        self.setParentItem(parent)
        self.setFlags(QGraphicsItem.ItemIsMovable |
                      QGraphicsItem.ItemIsSelectable |
                      #QGraphicsItem.ItemClipsChildrenToShape |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.label = ImageLabel(self)
        self.offset = ImageOffset(self)

        # list item in the dock widget's ListWidget
        # this allows fast updates of the list item without looking it up
        # It is save to assume that this is None or a valid QListWidgetItem        
        self.listItem = None
        
    def loadFromElement(self, element):
        self.name = element.get("Name", "Unknown")
        
        self.xpos = int(element.get("XPos", 0))
        self.ypos = int(element.get("YPos", 0))
        self.width = int(element.get("Width", 1))
        self.height = int(element.get("Height", 1))
        
        self.xoffset = int(element.get("XOffset", 0))
        self.yoffset = int(element.get("YOffset", 0))
                
        self.label.setVisible(False)
        
    def saveToElement(self):
        ret = ElementTree.Element("Image")
        
        ret.set("Name", self.name)
        
        ret.set("XPos", str(self.xpos))
        ret.set("YPos", str(self.ypos))
        ret.set("Width", str(self.width))
        ret.set("Height", str(self.height))
        
        # we write none or both
        if self.xoffset != 0 or self.yoffset != 0:
            ret.set("XOffset", str(self.xoffset))
            ret.set("YOffset", str(self.yoffset))

        return ret

    def getPixmap(self):
        return self.parent.pixmap().copy(int(self.pos().x()), int(self.pos().y()),
                                         int(self.rect().width()), int(self.rect().height()))

    def updateListItem(self):
        if not self.listItem:
            return
        
        self.listItem.setText(self.name)
        
        previewWidth = 24
        previewHeight = 24
        
        preview = QPixmap(previewWidth, previewHeight)
        preview.fill(Qt.transparent)
        painter = QPainter(preview)
        scaledPixmap = self.getPixmap().scaled(QSize(previewWidth, previewHeight), Qt.KeepAspectRatio, Qt.SmoothTransformation)
        painter.drawPixmap((previewWidth - scaledPixmap.width()) / 2,
                           (previewHeight - scaledPixmap.height()) / 2,
                           scaledPixmap)
        painter.end()
        
        self.listItem.setIcon(QIcon(preview))

    def updateListItemSelection(self):
        if not self.listItem:
            return
        
        dockWidget = self.listItem.dockWidget
        
        # the dock widget itself is performing a selection, we shall not interfere
        if dockWidget.selectionUnderway:
            return
        
        dockWidget.selectionSynchronisationUnderway = True
        
        if self.isSelected() or self.offset.isSelected():
            self.listItem.setSelected(True)
        else:
            self.listItem.setSelected(False)
            
        dockWidget.selectionSynchronisationUnderway = False

    def updateDockWidget(self):
        self.updateListItem()
        
        # TODO: update the property editor in the dock widget

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
                
            self.updateListItemSelection()

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
        
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        
        self.setZValue(self.zValue() - 1)
        
        super(ImageEntry, self).hoverLeaveEvent(event)
        
class ImagesetEntry(QGraphicsPixmapItem):
    def __init__(self, parent):
        super(ImagesetEntry, self).__init__()
        
        self.name = "Unknown"
        self.imageFile = ""
        self.nativeHorzRes = 800
        self.nativeVertRes = 600
        self.autoScaled = False
        
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
        self.name = element.get("Name", "Unknown")
        
        self.imageFile = element.get("Imagefile", "")
        absoluteImageFilePath = os.path.join(os.path.dirname(self.parent.parent.filePath), self.imageFile)
        self.setPixmap(QPixmap(absoluteImageFilePath))
        self.transparencyBackground.setRect(self.boundingRect())
        
        self.nativeHorzRes = int(element.get("NativeHorzRes", 800))
        self.nativeVertRes = int(element.get("NativeVertRes", 600))
        self.autoScaled = element.get("AutoScaled", "false") == "true"
        
        for imageElement in element.findall("Image"):
            image = ImageEntry(self)
            image.loadFromElement(imageElement)
            self.imageEntries.append(image)
    
    def saveToElement(self):
        ret = ElementTree.Element("Imageset")
        
        ret.set("Name", self.name)
        ret.set("Imagefile", self.imageFile)
        
        ret.set("NativeHorzRes", str(self.nativeHorzRes))
        ret.set("NativeVertRes", str(self.nativeVertRes))
        ret.set("AutoScaled", "true" if self.autoScaled else "false")
        
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
        
        self.filterBox = self.findChild(QLineEdit, "filterBox")
        self.filterBox.textChanged.connect(self.filterChanged)
        
        self.list = self.findChild(QListWidget, "list")
        self.list.itemSelectionChanged.connect(self.slot_itemSelectionChanged)
        self.list.itemChanged.connect(self.slot_itemChanged)
        
        self.selectionUnderway = False
        self.selectionSynchronisationUnderway = False
        
    def setImagesetEntry(self, imagesetEntry):
        self.imagesetEntry = imagesetEntry
        
    def refresh(self):
        self.list.clear()
        
        for imageEntry in self.imagesetEntry.imageEntries:
            item = QListWidgetItem()
            item.dockWidget = self
            item.setFlags(Qt.ItemIsSelectable |
                          Qt.ItemIsEditable |
                          Qt.ItemIsEnabled)
            
            item.imageEntry = imageEntry
            imageEntry.listItem = item
            # nothing is selected (list was cleared) so we don't need to call
            #  the whole updateDockWidget here
            imageEntry.updateListItem()
            
            self.list.addItem(item)
        
        # explicitly call the filtering again to make sure it's in sync    
        self.filterChanged(self.filterBox.text())

    def slot_itemSelectionChanged(self):
        # we are getting synchronised with the visual editing pane, do not interfere
        if self.selectionSynchronisationUnderway:
            return
        
        self.selectionUnderway = True
        self.parent.scene.clearSelection()
        
        imageEntryNames = self.list.selectedItems()
        for imageEntryName in imageEntryNames:
            imageEntry = imageEntryName.imageEntry
            #imageEntry = self.parent.imagesetEntry.getImageEntry(imageEntryName.text())
            imageEntry.setSelected(True)
            
        if len(imageEntryNames) == 1:
            #imageEntry = self.parent.imagesetEntry.getImageEntry(imageEntryNames[0].text())
            imageEntry = imageEntryNames[0].imageEntry
            self.parent.centerOn(imageEntry)
            
        self.selectionUnderway = False
        
    def slot_itemChanged(self, item):
        oldName = item.imageEntry.name
        newName = item.text()
        
        if oldName == newName:
            # most likely caused by RenameCommand doing it's work or is bogus anyways
            return
        
        cmd = undo.RenameCommand(self.parent, oldName, newName)
        self.parent.parent.undoStack.push(cmd)
    
    def filterChanged(self, filter):
        # we append star at the end by default (makes image filtering much more practical)
        filter = filter + "*"
        
        i = 0
        while i < self.list.count():
            listItem = self.list.item(i)
            match = fnmatch.fnmatch(listItem.text(), filter)
            listItem.setHidden(not match)
            
            i += 1

class VisualEditing(QGraphicsView):
    def __init__(self, parent):
        self.scene = QGraphicsScene()
        super(VisualEditing, self).__init__(self.scene)
        self.scene.selectionChanged.connect(self.slot_selectionChanged)
        
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
            
    def slot_selectionChanged(self):
        # if dockWidget is changing the selection, back off
        if self.dockWidget.selectionUnderway:
            return
        
        selectedItems = self.scene.selectedItems()
        if len(selectedItems) == 1:
            self.dockWidget.list.scrollToItem(selectedItems[0].listItem)
        
    def slot_toggleEditOffsets(self, enabled):
        self.scene.clearSelection()
        
        self.imagesetEntry.showOffsets = enabled
    