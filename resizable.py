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

class ResizingHandle(QGraphicsRectItem):
    def __init__(self, parent):
        super(ResizingHandle, self).__init__()
        
        self.setFlags(QGraphicsItem.ItemIsSelectable |
                      QGraphicsItem.ItemIsMovable |
                      QGraphicsItem.ItemSendsGeometryChanges)
        
        self.setParentItem(parent)
        
        self.ignoreGeometryChanges = False

    def unselectAllSiblingHandles(self):
        assert(self.parentItem())
        
        for item in self.parentItem().childItems():
            if isinstance(item, ResizingHandle) and not self is item:
                item.setSelected(False)

    def itemChange(self, change, value):
        if change == QGraphicsItem.ItemSelectedHasChanged:
            self.unselectAllSiblingHandles()
            
        if change == QGraphicsItem.ItemPositionChange:
            if not self.parentItem().resizeInProgress and not self.ignoreGeometryChanges:
                self.parentItem().resizeInProgress = True
                
                self.parentItem().resizeOldPos = self.parentItem().pos()
                self.parentItem().resizeOldRect = self.parentItem().rect()

        return super(ResizingHandle, self).itemChange(change, value)
    
    def mouseReleaseEvent(self, event):
        super(ResizingHandle, self).mouseReleaseEvent(event)
        
        if self.parentItem().resizeInProgress:
            # resize was in progress and just ended
            self.parentItem().resizeInProgress = False
            
            oldPos = self.parentItem().resizeOldPos
            oldRect = self.parentItem().resizeOldRect
            newPos = self.parentItem().pos() + self.parentItem().sizeBox.pos()
            newRect = self.parentItem().sizeBox.rect()
            
            if oldPos != newPos or oldRect != newRect:
                self.parentItem().notifyResizeFinished(oldPos, oldRect, newPos, newRect)

class TopEdgeResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(TopEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(TopEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, 0, -delta))
            self.parentItem().sizeBox.setPos(self.parentItem().sizeBox.pos() + QPointF(0, delta))
            
            return QPointF(self.pos().x(), value.y())
            
        return ret  

class BottomEdgeResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(BottomEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(BottomEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, 0, delta))
            
            return QPointF(self.pos().x(), value.y())
            
        return ret

class LeftEdgeResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(LeftEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(LeftEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.x() - self.pos().x()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, -delta, 0))
            self.parentItem().sizeBox.setPos(self.parentItem().sizeBox.pos() + QPointF(delta, 0))
            
            return QPointF(value.x(), self.pos().y())
            
        return ret

class RightEdgeResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(RightEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(RightEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.x() - self.pos().x()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, delta, 0))
            
            return QPointF(value.x(), self.pos().y())
            
        return ret

class ResizableGraphicsRectItem(QGraphicsRectItem):
    def __init__(self):
        super(ResizableGraphicsRectItem, self).__init__()
        
        self.setFlags(QGraphicsItem.ItemSendsGeometryChanges)
        
        #self.setAcceptsHoverEvents(True)
        
        self.sizeBox = QGraphicsRectItem(self)
        self.sizeBox.setPen(QPen(QColor(Qt.GlobalColor.red)))
        self.sizeBox.setFlags(QGraphicsItem.ItemStacksBehindParent)
        
        self.topEdgeHandle = TopEdgeResizingHandle(self)
        self.bottomEdgeHandle = BottomEdgeResizingHandle(self)
        self.leftEdgeHandle = LeftEdgeResizingHandle(self)
        self.rightEdgeHandle = RightEdgeResizingHandle(self)
        
        #self.topRightCornerHandle = QGraphicsRectItem(self)
        #self.bottomRightCornerHandle = QGraphicsRectItem(self)
        #self.bottomLeftCornerHandle = QGraphicsRectItem(self)
        #self.topLeftCornerHandle = QGraphicsRectItem(self)
        
        self.handlesDirty = True
        self.handlesCachedFor = None
        self.resizeInProgress = False
        
        self.setHandleSize(20)
    
    def setHandleSize(self, size):
        self.handleSize = size
        self.handlesDirty = True
        
    def setRect(self, rect):
        super(ResizableGraphicsRectItem, self).setRect(rect)

        self.sizeBox.setPos(QPointF(0, 0))
        self.sizeBox.setRect(rect)

        self.handlesDirty = True
        self.ensureHandlesUpdated()
        
    def setPen(self, pen):
        super(ResizableGraphicsRectItem, self).setPen(pen)
        
        self.sizeBox.setPen(pen)
        
        self.topEdgeHandle.setPen(pen)
        self.bottomEdgeHandle.setPen(pen)
        self.leftEdgeHandle.setPen(pen)
        self.rightEdgeHandle.setPen(pen)
        
    def ensureHandlesUpdated(self):
        transform = self.transform()
        
        if (self.handlesDirty or self.handlesCachedFor != transform) and not self.resizeInProgress:
            self.updateHandles(transform)
        
    def absoluteXToRelative(self, value, transform):
            x_scale = transform.m11()

            # this works in this special case, not in generic case!
            # I would have to undo rotation for this to work generically            
            return value / x_scale if x_scale != 0 else 1
    
    def absoluteYToRelative(self, value, transform):
            y_scale = transform.m22()
            
            # this works in this special case, not in generic case!
            # I would have to undo rotation for this to work generically   
            return value / y_scale if y_scale != 0 else 1
    
    def updateHandles(self, transform):
        """Updates all the handles according to geometry"""
        
        if self.rect().width() < 4 * self.absoluteXToRelative(self.handleSize, transform) or self.rect().height() < 4 * self.absoluteYToRelative(self.handleSize, transform):
            self.topEdgeHandle.ignoreGeometryChanges = True
            self.topEdgeHandle.setPos(0, -self.absoluteYToRelative(self.handleSize, transform))
            self.topEdgeHandle.setRect(0, 0,
                                       self.rect().width(),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topEdgeHandle.ignoreGeometryChanges = False
            
            self.bottomEdgeHandle.ignoreGeometryChanges = True
            self.bottomEdgeHandle.setPos(0, self.rect().height())
            self.bottomEdgeHandle.setRect(0, 0,
                                       self.rect().width(),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomEdgeHandle.ignoreGeometryChanges = False
            
            self.leftEdgeHandle.ignoreGeometryChanges = True
            self.leftEdgeHandle.setPos(QPointF(-self.absoluteXToRelative(self.handleSize, transform), 0))
            self.leftEdgeHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.rect().height())
            self.leftEdgeHandle.ignoreGeometryChanges = False
            
            self.rightEdgeHandle.ignoreGeometryChanges = True
            self.rightEdgeHandle.setPos(QPointF(self.rect().width(), 0))
            self.rightEdgeHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.rect().height())
            self.rightEdgeHandle.ignoreGeometryChanges = False
            
        else:
            self.topEdgeHandle.ignoreGeometryChanges = True
            self.topEdgeHandle.setPos(QPointF(self.absoluteXToRelative(self.handleSize, transform), 0))
            self.topEdgeHandle.setRect(0, 0,
                                       self.rect().width() - 2 * self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topEdgeHandle.ignoreGeometryChanges = False
            
            self.bottomEdgeHandle.ignoreGeometryChanges = True
            self.bottomEdgeHandle.setPos(QPointF(self.absoluteXToRelative(self.handleSize, transform), self.rect().height() - self.absoluteYToRelative(self.handleSize, transform)))
            self.bottomEdgeHandle.setRect(0, 0,
                                       self.rect().width() - 2 * self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomEdgeHandle.ignoreGeometryChanges = False
            
            self.leftEdgeHandle.ignoreGeometryChanges = True
            self.leftEdgeHandle.setPos(QPointF(0, self.absoluteYToRelative(self.handleSize, transform)))
            self.leftEdgeHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.rect().height() - 2 * self.absoluteYToRelative(self.handleSize, transform))
            self.leftEdgeHandle.ignoreGeometryChanges = False
            
            self.rightEdgeHandle.ignoreGeometryChanges = True
            self.rightEdgeHandle.setPos(QPointF(self.rect().width() - self.absoluteXToRelative(self.handleSize, transform), self.absoluteYToRelative(self.handleSize, transform)))
            self.rightEdgeHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.rect().height() - 2 * self.absoluteYToRelative(self.handleSize, transform))
            self.rightEdgeHandle.ignoreGeometryChanges = False
        
        self.handlesDirty = False
        self.handlesCachedFor = transform
        
    def notifyResizeFinished(self, oldPos, oldRect, newPos, newRect):
        self.setRect(newRect)
        self.setPos(newPos)
        
    #def hoverEnterEvent(self, event):
    #    super(ResizableGraphicsRectItem, self).hoverEnterEvent(event)
    #    
    #    self.topEdgeHandle.setVisible(True)
        
    #def hoverLeaveEvent(self, event):
    #    self.topEdgeHandle.setVisible(False)
    #
    #    super(ResizableGraphicsRectItem, self).hoverLeaveEvent(event)
        