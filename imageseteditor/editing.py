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
