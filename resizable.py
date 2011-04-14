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
        self.setAcceptHoverEvents(True)
        
        self.ignoreGeometryChanges = False
        self.mouseOver = False

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
                self.parentItem().sizeBox.setPen(self.parentItem().sizeBoxPenWhileResizing())
                self.parentItem().sizeBox.setZValue(1)
                self.parentItem().hideAllHandles(excluding = self)
                self.parentItem().setPen(self.parentItem().penWhileResizing())
                
                self.parentItem().resizeOldPos = self.parentItem().pos()
                self.parentItem().resizeOldRect = self.parentItem().rect()                

        return super(ResizingHandle, self).itemChange(change, value)
    
    def mouseReleaseEvent(self, event):
        super(ResizingHandle, self).mouseReleaseEvent(event)
        
        if self.parentItem().resizeInProgress:
            # resize was in progress and just ended
            self.parentItem().resizeInProgress = False
            self.parentItem().sizeBox.setPen(self.parentItem().sizeBoxNormalPen())
            self.parentItem().sizeBox.setZValue(0)
            self.parentItem().showAllHandles(excluding = self)
            self.parentItem().setPen(self.parentItem().hoverPen() if self.parentItem().mouseOver else self.parentItem().normalPen())
            
            oldPos = self.parentItem().resizeOldPos
            oldRect = self.parentItem().resizeOldRect
            newPos = self.parentItem().pos() + self.parentItem().sizeBox.pos()
            newRect = self.parentItem().sizeBox.rect()
            
            if oldPos != newPos or oldRect != newRect:
                self.parentItem().notifyResizeFinished(oldPos, oldRect, newPos, newRect)

    def hoverEnterEvent(self, event):
        super(ResizingHandle, self).hoverEnterEvent(event)
        
        self.mouseOver = True
        
    def hoverLeaveEvent(self, event):
        self.mouseOver = False
        
        super(ResizingHandle, self).hoverLeaveEvent(event)

class EdgeResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(EdgeResizingHandle, self).__init__(parent)
        
        self.setPen(self.parentItem().edgeResizingHandleNormalPen())

    def hoverEnterEvent(self, event):
        super(EdgeResizingHandle, self).hoverEnterEvent(event)
        
        self.setPen(self.parentItem().edgeResizingHandleHoverPen())
        
    def hoverLeaveEvent(self, event):
        self.setPen(self.parentItem().edgeResizingHandleNormalPen())
        
        super(EdgeResizingHandle, self).hoverLeaveEvent(event)

class TopEdgeResizingHandle(EdgeResizingHandle):
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

class BottomEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(BottomEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(BottomEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, 0, delta))
            
            return QPointF(self.pos().x(), value.y())
            
        return ret

class LeftEdgeResizingHandle(EdgeResizingHandle):
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

class RightEdgeResizingHandle(EdgeResizingHandle):
    def __init__(self, parent):
        super(RightEdgeResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(RightEdgeResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta = value.x() - self.pos().x()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, delta, 0))
            
            return QPointF(value.x(), self.pos().y())
            
        return ret

class CornerResizingHandle(ResizingHandle):
    def __init__(self, parent):
        super(CornerResizingHandle, self).__init__(parent)
        
        self.setPen(self.parentItem().cornerResizingHandleNormalPen())

    def hoverEnterEvent(self, event):
        super(CornerResizingHandle, self).hoverEnterEvent(event)
        
        self.setPen(self.parentItem().cornerResizingHandleHoverPen())
        
    def hoverLeaveEvent(self, event):
        self.setPen(self.parentItem().cornerResizingHandleNormalPen())
        
        super(CornerResizingHandle, self).hoverLeaveEvent(event)

class TopRightCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(TopRightCornerResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(TopRightCornerResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta_x = value.x() - self.pos().x()
            delta_y = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, delta_x, -delta_y))
            self.parentItem().sizeBox.setPos(self.parentItem().sizeBox.pos() + QPointF(0, delta_y))
            
            return value
            
        return ret  

class BottomRightCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(BottomRightCornerResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(BottomRightCornerResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta_x = value.x() - self.pos().x()
            delta_y = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, delta_x, delta_y))
            
            return value
            
        return ret 

class BottomLeftCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(BottomLeftCornerResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(BottomLeftCornerResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta_x = value.x() - self.pos().x()
            delta_y = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, -delta_x, delta_y))
            self.parentItem().sizeBox.setPos(self.parentItem().sizeBox.pos() + QPointF(delta_x, 0))
            
            return value
            
        return ret

class TopLeftCornerResizingHandle(CornerResizingHandle):
    def __init__(self, parent):
        super(TopLeftCornerResizingHandle, self).__init__(parent)
        
    def itemChange(self, change, value):
        ret = super(TopLeftCornerResizingHandle, self).itemChange(change, value)

        if change == QGraphicsItem.ItemPositionChange and not self.ignoreGeometryChanges:
            delta_x = value.x() - self.pos().x()
            delta_y = value.y() - self.pos().y()

            self.parentItem().sizeBox.setRect(self.parentItem().sizeBox.rect().adjusted(0, 0, -delta_x, -delta_y))
            self.parentItem().sizeBox.setPos(self.parentItem().sizeBox.pos() + QPointF(delta_x, delta_y))
            
            return value
            
        return ret


class ResizableGraphicsRectItem(QGraphicsRectItem):
    def __init__(self):
        super(ResizableGraphicsRectItem, self).__init__()
        
        self.setFlags(QGraphicsItem.ItemSendsGeometryChanges)
        
        self.setAcceptsHoverEvents(True)
        self.mouseOver = False

        self.sizeBox = QGraphicsRectItem(self)
        self.sizeBox.setFlags(QGraphicsItem.ItemStacksBehindParent)
        self.sizeBox.setPen(self.sizeBoxNormalPen())
        
        self.topEdgeHandle = TopEdgeResizingHandle(self)
        self.bottomEdgeHandle = BottomEdgeResizingHandle(self)
        self.leftEdgeHandle = LeftEdgeResizingHandle(self)
        self.rightEdgeHandle = RightEdgeResizingHandle(self)
        
        self.topRightCornerHandle = TopRightCornerResizingHandle(self)
        self.bottomRightCornerHandle = BottomRightCornerResizingHandle(self)
        self.bottomLeftCornerHandle = BottomLeftCornerResizingHandle(self)
        self.topLeftCornerHandle = TopLeftCornerResizingHandle(self)
        
        self.handlesDirty = True
        self.handlesCachedFor = None
        self.resizeInProgress = False
        
        self.setHandleSize(20)
    
    def normalPen(self):
        ret = QPen()
        ret.setColor(QColor(Qt.transparent))
        
        return ret
    
    def hoverPen(self):
        ret = QPen()
        ret.setColor(QColor(0, 127, 0, 255))
        
        return ret
    
    def edgeResizingHandleNormalPen(self):
        ret = QPen()
        ret.setColor(QColor(0, 127, 0, 127))
        
        return ret
    
    def edgeResizingHandleHoverPen(self):
        ret = QPen()
        ret.setColor(QColor(0, 127, 0, 255))
        ret.setWidth(2)
        
        return ret
        
    def cornerResizingHandleNormalPen(self):
        ret = QPen()
        ret.setColor(QColor(Qt.transparent))
        
        return ret
    
    def cornerResizingHandleHoverPen(self):
        ret = QPen()
        ret.setColor(QColor(0, 127, 0, 255))
        ret.setWidth(2)
        
        return ret    
    
    def penWhileResizing(self):
        ret = QPen()
        ret.setColor(QColor(Qt.transparent))
        
        return ret
    
    def sizeBoxNormalPen(self):
        ret = QPen()
        ret.setColor(QColor(Qt.transparent))
        
        return ret
    
    def sizeBoxPenWhileResizing(self):
        ret = QPen(QColor(255, 0, 0, 255))
        
        return ret
    
    def setHandleSize(self, size):
        self.handleSize = size
        self.handlesDirty = True
        
    def setRect(self, rect):
        super(ResizableGraphicsRectItem, self).setRect(rect)

        self.sizeBox.setPos(QPointF(0, 0))
        self.sizeBox.setRect(rect)

        self.handlesDirty = True
        self.ensureHandlesUpdated()
        
    def hideAllHandles(self, excluding = None):
        for item in self.childItems():
            if isinstance(item, ResizingHandle) and item is not excluding:
                item.setVisible(False)
     
    def showAllHandles(self, excluding = None):
        for item in self.childItems():
            if isinstance(item, ResizingHandle) and item is not excluding:
                item.setVisible(True)
        
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
            
            self.topRightCornerHandle.ignoreGeometryChanges = True
            self.topRightCornerHandle.setPos(self.rect().width(), -self.absoluteYToRelative(self.handleSize, transform))
            self.topRightCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topRightCornerHandle.ignoreGeometryChanges = False

            self.bottomRightCornerHandle.ignoreGeometryChanges = True
            self.bottomRightCornerHandle.setPos(self.rect().width(), self.rect().height())
            self.bottomRightCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomRightCornerHandle.ignoreGeometryChanges = False
            
            self.bottomLeftCornerHandle.ignoreGeometryChanges = True
            self.bottomLeftCornerHandle.setPos(-self.absoluteXToRelative(self.handleSize, transform), self.rect().height())
            self.bottomLeftCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomLeftCornerHandle.ignoreGeometryChanges = False

            self.topLeftCornerHandle.ignoreGeometryChanges = True
            self.topLeftCornerHandle.setPos(-self.absoluteXToRelative(self.handleSize, transform), -self.absoluteYToRelative(self.handleSize, transform))
            self.topLeftCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topLeftCornerHandle.ignoreGeometryChanges = False
            
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
            
            self.topRightCornerHandle.ignoreGeometryChanges = True
            self.topRightCornerHandle.setPos(self.rect().width() - self.absoluteXToRelative(self.handleSize, transform), 0)
            self.topRightCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topRightCornerHandle.ignoreGeometryChanges = False
            
            self.bottomRightCornerHandle.ignoreGeometryChanges = True
            self.bottomRightCornerHandle.setPos(self.rect().width() - self.absoluteXToRelative(self.handleSize, transform), self.rect().height() - self.absoluteYToRelative(self.handleSize, transform))
            self.bottomRightCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomRightCornerHandle.ignoreGeometryChanges = False
            
            self.bottomLeftCornerHandle.ignoreGeometryChanges = True
            self.bottomLeftCornerHandle.setPos(0, self.rect().height() - self.absoluteYToRelative(self.handleSize, transform))
            self.bottomLeftCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.bottomLeftCornerHandle.ignoreGeometryChanges = False
            
            self.topLeftCornerHandle.ignoreGeometryChanges = True
            self.topLeftCornerHandle.setPos(0, 0)
            self.topLeftCornerHandle.setRect(0, 0,
                                       self.absoluteXToRelative(self.handleSize, transform),
                                       self.absoluteYToRelative(self.handleSize, transform))
            self.topLeftCornerHandle.ignoreGeometryChanges = False
        
        self.handlesDirty = False
        self.handlesCachedFor = transform
        
    def notifyResizeFinished(self, oldPos, oldRect, newPos, newRect):
        self.setRect(newRect)
        self.setPos(newPos)
        
    def hoverEnterEvent(self, event):
        super(ResizableGraphicsRectItem, self).hoverEnterEvent(event)
        
        self.setPen(self.hoverPen())
        self.showAllHandles()
        self.mouseOver = True
        
    def hoverLeaveEvent(self, event):
        self.mouseOver = False
        self.setPen(self.normalPen())
        self.hideAllHandles()
    
        super(ResizableGraphicsRectItem, self).hoverLeaveEvent(event)
        