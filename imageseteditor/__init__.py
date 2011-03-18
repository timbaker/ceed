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
    def __init__(self, parentImageset):
        super(ImageEntry, self).__init__()
        
        self.setAcceptsHoverEvents(True)
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        self.isHovered = False
        
        self.setParentItem(parentImageset)
        self.setFlags(QGraphicsItem.ItemIsFocusable |
                      QGraphicsItem.ItemIsSelectable |
                      QGraphicsItem.ItemClipsChildrenToShape)
        
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
                #self.setFlags(self.flags() | QGraphicsItem.ItemIsMovable)
            else:
                if not self.isHovered:
                    self.label.setVisible(False)
                #self.setFlags(self.flags() ^ QGraphicsItem.ItemIsMovable)

        return super(ImageEntry, self).itemChange(change, value)
    
    def hoverEnterEvent(self, event):
        super(ImageEntry, self).hoverEnterEvent(event)
        
        self.setZValue(1)
        
        pen = QPen()
        pen.setColor(QColor(Qt.black))
        self.setPen(pen)
        
        self.label.setVisible(True)
        self.label.setOpacity(0.3)
        
        self.isHovered = True
    
    def hoverLeaveEvent(self, event):
        self.isHovered = False
        
        if not self.isSelected():
            self.label.setVisible(False)
        
        self.label.setOpacity(0.8)
        
        pen = QPen()
        pen.setColor(QColor(Qt.lightGray))
        self.setPen(pen)
        
        self.setZValue(0)
        
        super(ImageEntry, self).hoverLeaveEvent(event)
        
class ImagesetEntry(QGraphicsPixmapItem):
    def __init__(self, parent):
        super(ImagesetEntry, self).__init__()
        
        self.setShapeMode(QGraphicsPixmapItem.BoundingRectShape)
        
        self.parent = parent
        
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
        
    def loadFromElement(self, element):
        self.setPixmap(QPixmap("datafiles/imagesets/%s" % (element.get("Imagefile", ""))))
        self.transparencyBackground.setRect(self.boundingRect())
        
        for imageElement in element.findall("Image"):
            image = ImageEntry(self)
            image.loadFromElement(imageElement)
    
    def saveToElement(self, element):
        for image in self.childItems():
            element.append(image.saveToElement())

class ImagesetTabbedEditor(tab.TabbedEditor, QGraphicsView):
    def __init__(self, filePath):
        tab.TabbedEditor.__init__(self, filePath)
        
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
            
        self.imagesetEntry = ImagesetEntry(self)
        self.imagesetEntry.loadFromElement(root)
        
        self.scene.addItem(self.imagesetEntry)
    
    def finalise(self):
        super(ImagesetTabbedEditor, self).finalise()
        
        self.tabWidget = None
        
    def hasChanges(self):
        return False
    
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
        else:
            self.lastMousePosition = event.pos()
      
    def mouseReleaseEvent(self, event):
        if event.buttons() != Qt.MiddleButton: 
            super(ImagesetTabbedEditor, self).mouseReleaseEvent(event)
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
        if event.delta() > 0:
            self.zoomIn()
        else:
            self.zoomOut()

class ImagesetTabbedEditorFactory(tab.TabbedEditorFactory):
    def canEditFile(self, filePath):
        extensions = [".imageset"]
        
        for extension in extensions:
            if filePath.endswith(extension):
                return True
            
        return False

    def create(self, filePath):
        return ImagesetTabbedEditor(filePath)
