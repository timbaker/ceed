/*
   CEED - Unified CEGUI asset editor

   Copyright (C) 2011-2017   Martin Preisler <martin@preisler.me>
                             and contributing authors (see AUTHORS file)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "resizable.h"

#include <QtEvents>
#include <QScrollBar>

namespace CEED {
namespace resizable {

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    // disabled by default
    m_wheelZoomEnabled = false;
    m_zoomFactor = 1.0f;

    m_middleButtonDragScrollEnabled = false;
    m_lastDragScrollMousePosition = QPoint();

    m_ctrlZoom = settings::getEntry("global/navigation/ctrl_zoom");
}

void GraphicsView::setTransform(const QTransform &transform)
{
    QGraphicsView::setTransform(transform);

    qreal sx = transform.m11();
    qreal sy = transform.m22();

    scaleChanged(sx, sy);
}

void GraphicsView::scaleChanged(qreal sx, qreal sy)
{
    for (QGraphicsItem* item : scene()->items()) {
        if (auto* dc = dynamic_cast<ResizableRectItem*>(item)) {
            dc->scaleChanged(sx, sy);
        }
    }
}

void GraphicsView::performZoom()
{
    QTransform transform;
    transform.scale(m_zoomFactor, m_zoomFactor);
    setTransform(transform);
}

void GraphicsView::zoomOriginal()
{
    m_zoomFactor = 1;
    performZoom();
}

void GraphicsView::zoomIn()
{
    m_zoomFactor *= 2;

    if (m_zoomFactor > 256)
        m_zoomFactor = 256;

    performZoom();
}

void GraphicsView::zoomOut()
{
    m_zoomFactor /= 2;

    if (m_zoomFactor < 0.5)
        m_zoomFactor = 0.5;

    performZoom();
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    if (m_wheelZoomEnabled && (!m_ctrlZoom->m_value.toBool() || (event->modifiers() & Qt::ControlModifier))) {
        if (event->delta() == 0)
            return;

        if (event->delta() > 0)
            zoomIn();
        else
            zoomOut();
    }

    else
        QGraphicsView::wheelEvent(event);
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (m_middleButtonDragScrollEnabled && (event->buttons() == Qt::MiddleButton))
        m_lastDragScrollMousePosition = event->pos();

    else
        QGraphicsView::mousePressEvent(event);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    for (QGraphicsItem* selectedItem : scene()->selectedItems()) {
        if (auto resizingHandle = dynamic_cast<ResizingHandle*>(selectedItem))
            resizingHandle->mouseReleaseEventSelected(event);
        else if (auto resizableRectItem = dynamic_cast<ResizableRectItem*>(selectedItem))
            resizableRectItem->mouseReleaseEventSelected(event);
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_middleButtonDragScrollEnabled && (event->buttons() == Qt::MiddleButton)) {
        QScrollBar* horizontal = horizontalScrollBar();
        horizontal->setSliderPosition(horizontal->sliderPosition() - (event->pos().x() - m_lastDragScrollMousePosition.x()));
        QScrollBar* vertical = verticalScrollBar();
        vertical->setSliderPosition(vertical->sliderPosition() - (event->pos().y() - m_lastDragScrollMousePosition.y()));
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }

    m_lastDragScrollMousePosition = event->pos();
}

/////

ResizingHandle::ResizingHandle(ResizableRectItem *parent)
    : QGraphicsRectItem(parent)
    , m_parentResizable(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setAcceptHoverEvents(true);

    m_ignoreGeometryChanges = false;
    m_ignoreTransformChanges = false;
    m_mouseOver = false;
    //        m_currentView = None;
}

QPointF ResizingHandle::performResizing(const QPointF &value)
{
    /**Adjusts the parent rectangle and returns a position to use for this handle
        (with restrictions accounted for)
        */

    return value;
}

void ResizingHandle::unselectAllSiblingHandles()
{
    /**Makes sure all siblings of this handle are unselected.*/

    Q_ASSERT(m_parentResizable);

    for (QGraphicsItem* item : m_parentResizable->childItems()) {
        if (dynamic_cast<ResizingHandle*>(item) && (item != this))
            item->setSelected(false);
    }
}

QVariant ResizingHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    /**This overriden method does most of the resize work
        */

    if (change == QGraphicsItem::ItemSelectedChange) {
        if (m_parentResizable->isSelected()) {
            // we disallow multi-selecting a resizable item and one of it's handles,
            return false;
        }
    }

    else if (change == QGraphicsItem::ItemSelectedHasChanged) {
        // if we have indeed been selected, make sure all our sibling handles are unselected
        // we allow multi-selecting multiple handles but only one handle per resizable is allowed

        unselectAllSiblingHandles();
        m_parentResizable->notifyHandleSelected(this);
    }

    else if (change == QGraphicsItem::ItemPositionChange) {
        // this is the money code
        // changing position of the handle resizes the whole resizable
        if (!m_parentResizable->m_resizeInProgress && !m_ignoreGeometryChanges) {
            m_parentResizable->m_resizeInProgress = true;
            m_parentResizable->m_resizeOldPos = m_parentResizable->pos();
            m_parentResizable->m_resizeOldRect = m_parentResizable->rect();

            m_parentResizable->setPen(m_parentResizable->getPenWhileResizing());
            m_parentResizable->hideAllHandles(this);

            m_parentResizable->notifyResizeStarted();
        }

        if (m_parentResizable->m_resizeInProgress) {
            QPointF ret = performResizing(value.toPointF());

            QPointF newPos = m_parentResizable->pos() + m_parentResizable->rect().topLeft();
            QRectF newRect = QRectF(0, 0, m_parentResizable->rect().width(), m_parentResizable->rect().height());

            m_parentResizable->notifyResizeProgress(newPos, newRect);

            return ret;
        }
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void ResizingHandle::mouseReleaseEventSelected(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_parentResizable->m_resizeInProgress) {
        // resize was in progress and just ended
        m_parentResizable->m_resizeInProgress = false;
        m_parentResizable->setPen(m_parentResizable->m_mouseOver ? m_parentResizable->getHoverPen() : m_parentResizable->getNormalPen());

        QPointF newPos = m_parentResizable->pos() + m_parentResizable->rect().topLeft();
        QRectF newRect = QRectF(0, 0, m_parentResizable->rect().width(), m_parentResizable->rect().height());

        m_parentResizable->notifyResizeFinished(newPos, newRect);
    }
}

void ResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    m_mouseOver = true;
}

void ResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseOver = false;
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void ResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    Q_UNUSED(sx)
    Q_UNUSED(sy)
}

/////

EdgeResizingHandle::EdgeResizingHandle(ResizableRectItem *parent)
    : ResizingHandle(parent)
{
    setPen(m_parentResizable->getEdgeResizingHandleHiddenPen());
}

void EdgeResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    ResizingHandle::hoverEnterEvent(event);
    setPen(m_parentResizable->getEdgeResizingHandleHoverPen());
}

void EdgeResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setPen(m_parentResizable->getEdgeResizingHandleHiddenPen());
    ResizingHandle::hoverLeaveEvent(event);
}

/////

TopEdgeResizingHandle::TopEdgeResizingHandle(ResizableRectItem *parent)
    : EdgeResizingHandle(parent)
{
    setCursor(Qt::SizeVerCursor);
}

QPointF TopEdgeResizingHandle::performResizing(const QPointF &value)
{
    qreal delta = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, 0, delta, 0, 0, topLeft, bottomRight);

    return QPointF(pos().x(), topLeft.y() + pos().y());
}

void TopEdgeResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    EdgeResizingHandle::scaleChanged(sx, sy);

    QTransform transform = this->transform();
    transform = QTransform(1.0, transform.m12(), transform.m13(),
                           transform.m21(), 1.0 / sy, transform.m23(),
                           transform.m31(), transform.m32(), transform.m33());
    setTransform(transform);
}

/////

BottomEdgeResizingHandle::BottomEdgeResizingHandle(ResizableRectItem *parent)
    : EdgeResizingHandle(parent)
{
    setCursor(Qt::SizeVerCursor);
}

QPointF BottomEdgeResizingHandle::performResizing(const QPointF &value)
{
    qreal delta = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, 0, 0, 0, delta, topLeft, bottomRight);

    return QPointF(pos().x(), bottomRight.y() + pos().y());
}

void BottomEdgeResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    EdgeResizingHandle::scaleChanged(sx, sy);

    QTransform transform = this->transform();
    transform = QTransform(1.0, transform.m12(), transform.m13(),
                           transform.m21(), 1.0 / sy, transform.m23(),
                           transform.m31(), transform.m32(), transform.m33());
    setTransform(transform);
}

/////

LeftEdgeResizingHandle::LeftEdgeResizingHandle(ResizableRectItem *parent)
    : EdgeResizingHandle(parent)
{
    setCursor(Qt::SizeHorCursor);
}

QPointF LeftEdgeResizingHandle::performResizing(const QPointF &value)
{
    qreal delta = value.x() - pos().x();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, delta, 0, 0, 0, topLeft, bottomRight);

    return QPointF(topLeft.x() + pos().x(), pos().y());
}

void LeftEdgeResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    EdgeResizingHandle::scaleChanged(sx, sy);

    QTransform transform = this->transform();
    transform = QTransform(1.0 / sx, transform.m12(), transform.m13(),
                           transform.m21(), 1.0, transform.m23(),
                           transform.m31(), transform.m32(), transform.m33());
    setTransform(transform);
}

/////

RightEdgeResizingHandle::RightEdgeResizingHandle(ResizableRectItem *parent)
    : EdgeResizingHandle(parent)
{
    setCursor(Qt::SizeHorCursor);
}

QPointF RightEdgeResizingHandle::performResizing(const QPointF &value)
{
    qreal delta = value.x() - pos().x();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, 0, 0, delta, 0, topLeft, bottomRight);

    return QPointF(bottomRight.x() + pos().x(), pos().y());
}

void RightEdgeResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    EdgeResizingHandle::scaleChanged(sx, sy);

    QTransform transform = this->transform();
    transform = QTransform(1.0 / sx, transform.m12(), transform.m13(),
                           transform.m21(), 1.0, transform.m23(),
                           transform.m31(), transform.m32(), transform.m33());
    setTransform(transform);
}

/////

CornerResizingHandle::CornerResizingHandle(ResizableRectItem *parent)
    : ResizingHandle(parent)
{
    setPen(m_parentResizable->getCornerResizingHandleHiddenPen());
    setFlags(flags());

    setZValue(1);
}

void CornerResizingHandle::scaleChanged(qreal sx, qreal sy)
{
    ResizingHandle::scaleChanged(sx, sy);

    QTransform transform = this->transform();
    transform = QTransform(1.0 / sx, transform.m12(), transform.m13(),
                           transform.m21(), 1.0 / sy, transform.m23(),
                           transform.m31(), transform.m32(), transform.m33());
    setTransform(transform);
}

void CornerResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    ResizingHandle::hoverEnterEvent(event);
    setPen(m_parentResizable->getCornerResizingHandleHoverPen());
}

void CornerResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setPen(m_parentResizable->getCornerResizingHandleHiddenPen());
    ResizingHandle::hoverLeaveEvent(event);
}

/////

TopRightCornerResizingHandle::TopRightCornerResizingHandle(ResizableRectItem *parent)
    : CornerResizingHandle(parent)
{
    setCursor(Qt::SizeBDiagCursor);
}

QPointF TopRightCornerResizingHandle::performResizing(const QPointF &value)
{
    qreal deltaX = value.x() - pos().x();
    qreal deltaY = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, 0, deltaY, deltaX, 0, topLeft, bottomRight);

    return QPointF(bottomRight.x() + pos().x(), topLeft.y() + pos().y());
}

/////

BottomRightCornerResizingHandle::BottomRightCornerResizingHandle(ResizableRectItem *parent)
    : CornerResizingHandle(parent)
{
    setCursor(Qt::SizeFDiagCursor);
}

QPointF BottomRightCornerResizingHandle::performResizing(const QPointF &value)
{
    qreal deltaX = value.x() - pos().x();
    qreal deltaY = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, 0, 0, deltaX, deltaY, topLeft, bottomRight);

    return QPointF(bottomRight.x() + pos().x(), bottomRight.y() + pos().y());
}

/////

BottomLeftCornerResizingHandle::BottomLeftCornerResizingHandle(ResizableRectItem *parent)
    : CornerResizingHandle(parent)
{
    setCursor(Qt::SizeBDiagCursor);
}

QPointF BottomLeftCornerResizingHandle::performResizing(const QPointF &value)
{
    qreal deltaX = value.x() - pos().x();
    qreal deltaY = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, deltaX, 0, 0, deltaY, topLeft, bottomRight);

    return QPointF(topLeft.x() + pos().x(), bottomRight.y() + pos().y());
}

/////

TopLeftCornerResizingHandle::TopLeftCornerResizingHandle(ResizableRectItem *parent)
    : CornerResizingHandle(parent)
{
    setCursor(Qt::SizeFDiagCursor);
}

QPointF TopLeftCornerResizingHandle::performResizing(const QPointF &value)
{
    qreal deltaX = value.x() - pos().x();
    qreal deltaY = value.y() - pos().y();
    QPointF topLeft, bottomRight;
    m_parentResizable->performResizing(this, deltaX, deltaY, 0, 0, topLeft, bottomRight);

    return QPointF(topLeft.x() + pos().x(), topLeft.y() + pos().y());
}

/////

ResizableRectItem::ResizableRectItem(QGraphicsItem *parentItem)
    : QGraphicsRectItem(parentItem)
{
    setFlags(QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemIsMovable);

    setAcceptHoverEvents(true);
    m_mouseOver = false;

    m_topEdgeHandle = new TopEdgeResizingHandle(this);
    m_bottomEdgeHandle = new BottomEdgeResizingHandle(this);
    m_leftEdgeHandle = new LeftEdgeResizingHandle(this);
    m_rightEdgeHandle = new RightEdgeResizingHandle(this);

    m_topRightCornerHandle = new TopRightCornerResizingHandle(this);
    m_bottomRightCornerHandle = new BottomRightCornerResizingHandle(this);
    m_bottomLeftCornerHandle = new BottomLeftCornerResizingHandle(this);
    m_topLeftCornerHandle = new TopLeftCornerResizingHandle(this);

    m_handlesDirty = true;
    m_currentScaleX = 1;
    m_currentScaleY = 1;

    m_ignoreGeometryChanges = false;

    m_resizeInProgress = false;
    m_resizeOldPos = QPointF(); // optional<QPointF()>
    m_resizeOldRect = QRectF(); // optional<QRectF()>
    m_moveInProgress = false;
    m_moveOldPos = QPointF(); // optional<QPointF()>

    hideAllHandles();

    m_outerHandleSize = 0;
    m_innerHandleSize = 0;
    setOuterHandleSize(15);
    setInnerHandleSize(10);

    setCursor(Qt::OpenHandCursor);

    setPen(getNormalPen()); // NOTE: virtual method in constructor
}

QRectF ResizableRectItem::constrainResizeRect(const QRectF &rect_, const QRectF &oldRect)
{
    Q_UNUSED(oldRect)

    QSizeF minSize = getMinSize();
    QSizeF maxSize = getMaxSize();

    QRectF rect = rect_;

    if (!minSize.isEmpty()) {
        QRectF minRect(rect.center() - QPointF(0.5 * minSize.width(), 0.5 * minSize.height()), minSize);
        rect = rect.united(minRect);
    }
    if (!maxSize.isEmpty()) {
        QRectF maxRect(rect.center() - QPointF(0.5 * maxSize.width(), 0.5 * maxSize.height()), maxSize);
        rect = rect.intersected(maxRect);
    }

    return rect;
}

void ResizableRectItem::performResizing(ResizingHandle *handle, qreal deltaX1, qreal deltaY1, qreal deltaX2, qreal deltaY2, QPointF &topLeftDelta, QPointF &bottomRightDelta)
{
    Q_UNUSED(handle)

    QRectF newRect = rect().adjusted(deltaX1, deltaY1, deltaX2, deltaY2);
    newRect = constrainResizeRect(newRect, rect());

    // TODO: the rect moves as a whole when it can't be sized any less
    //       this is probably not the behavior we want!

    topLeftDelta = newRect.topLeft() - rect().topLeft();
    bottomRightDelta = newRect.bottomRight() - rect().bottomRight();

    setRect(newRect);
}

void ResizableRectItem::hideAllHandles(ResizingHandle *excluding)
{
    for (QGraphicsItem* item : childItems()) {
        if (dynamic_cast<ResizingHandle*>(item) && item != excluding) {
            if (auto rh = dynamic_cast<EdgeResizingHandle*>(item)) {
                rh->setPen(getEdgeResizingHandleHiddenPen());
            } else if (auto rh = dynamic_cast<CornerResizingHandle*>(item)) {
                rh->setPen(getCornerResizingHandleHiddenPen());
            }
        }
    }
}

void ResizableRectItem::setResizingEnabled(bool enabled)
{
    for (QGraphicsItem* item : childItems()) {
        if (dynamic_cast<ResizingHandle*>(item)) {
            item->setVisible(enabled);
        }
    }
}

void ResizableRectItem::unselectAllHandles()
{
    for (QGraphicsItem* item : childItems()) {
        if (dynamic_cast<ResizingHandle*>(item)) {
            item->setSelected(false);
        }
    }
}

bool ResizableRectItem::isAnyHandleSelected()
{
    for (QGraphicsItem* item : childItems()) {
        if (dynamic_cast<ResizingHandle*>(item)) {
            if (item->isSelected())
                return true;
        }
    }

    return false;
}

void ResizableRectItem::ensureHandlesUpdated()
{
    if (m_handlesDirty && !m_resizeInProgress)
        updateHandles();
}

qreal ResizableRectItem::absoluteXToRelative(qreal value, const QTransform &transform)
{
    qreal xScale = transform.m11();

    // this works in this special case, not in generic case!
    // I would have to undo rotation for this to work generically
    if (xScale == 0)
        return 1;
    return value / xScale;
}

qreal ResizableRectItem::absoluteYToRelative(qreal value, const QTransform &transform)
{
    qreal yScale = transform.m22();

    // this works in this special case, not in generic case!
    // I would have to undo rotation for this to work generically
    if (yScale == 0)
        return 1;
    return value / yScale;
}

void ResizableRectItem::updateHandles()
{
    qreal absoluteWidth = m_currentScaleX * rect().width();
    qreal absoluteHeight = m_currentScaleY * rect().height();

    if (absoluteWidth < 4 * m_outerHandleSize || absoluteHeight < 4 * m_outerHandleSize) {
        m_topEdgeHandle->m_ignoreGeometryChanges = true;
        m_topEdgeHandle->setPos(0, 0);
        m_topEdgeHandle->setRect(0, -m_innerHandleSize,
                                 rect().width(),
                                 m_innerHandleSize);
        m_topEdgeHandle->m_ignoreGeometryChanges = false;

        m_bottomEdgeHandle->m_ignoreGeometryChanges = true;
        m_bottomEdgeHandle->setPos(0, rect().height());
        m_bottomEdgeHandle->setRect(0, 0,
                                    rect().width(),
                                    m_innerHandleSize);
        m_bottomEdgeHandle->m_ignoreGeometryChanges = false;

        m_leftEdgeHandle->m_ignoreGeometryChanges = true;
        m_leftEdgeHandle->setPos(0, 0);
        m_leftEdgeHandle->setRect(-m_innerHandleSize, 0,
                                  m_innerHandleSize,
                                  rect().height());
        m_leftEdgeHandle->m_ignoreGeometryChanges = false;

        m_rightEdgeHandle->m_ignoreGeometryChanges = true;
        m_rightEdgeHandle->setPos(QPointF(rect().width(), 0));
        m_rightEdgeHandle->setRect(0, 0,
                                   m_innerHandleSize,
                                   rect().height());
        m_rightEdgeHandle->m_ignoreGeometryChanges = false;

        m_topRightCornerHandle->m_ignoreGeometryChanges = true;
        m_topRightCornerHandle->setPos(rect().width(), 0);
        m_topRightCornerHandle->setRect(0, -m_innerHandleSize,
                                        m_innerHandleSize,
                                        m_innerHandleSize);
        m_topRightCornerHandle->m_ignoreGeometryChanges = false;

        m_bottomRightCornerHandle->m_ignoreGeometryChanges = true;
        m_bottomRightCornerHandle->setPos(rect().width(), rect().height());
        m_bottomRightCornerHandle->setRect(0, 0,
                                           m_innerHandleSize,
                                           m_innerHandleSize);
        m_bottomRightCornerHandle->m_ignoreGeometryChanges = false;

        m_bottomLeftCornerHandle->m_ignoreGeometryChanges = true;
        m_bottomLeftCornerHandle->setPos(0, rect().height());
        m_bottomLeftCornerHandle->setRect(-m_innerHandleSize, 0,
                                          m_innerHandleSize,
                                          m_innerHandleSize);
        m_bottomLeftCornerHandle->m_ignoreGeometryChanges = false;

        m_topLeftCornerHandle->m_ignoreGeometryChanges = true;
        m_topLeftCornerHandle->setPos(0, 0);
        m_topLeftCornerHandle->setRect(-m_innerHandleSize, -m_innerHandleSize,
                                       m_innerHandleSize,
                                       m_innerHandleSize);
        m_topLeftCornerHandle->m_ignoreGeometryChanges = false;

    } else {
        m_topEdgeHandle->m_ignoreGeometryChanges = true;
        m_topEdgeHandle->setPos(0, 0);
        m_topEdgeHandle->setRect(0, 0,
                                 rect().width(),
                                 m_outerHandleSize);
        m_topEdgeHandle->m_ignoreGeometryChanges = false;

        m_bottomEdgeHandle->m_ignoreGeometryChanges = true;
        m_bottomEdgeHandle->setPos(0, rect().height());
        m_bottomEdgeHandle->setRect(0, -m_outerHandleSize,
                                    rect().width(),
                                    m_outerHandleSize);
        m_bottomEdgeHandle->m_ignoreGeometryChanges = false;

        m_leftEdgeHandle->m_ignoreGeometryChanges = true;
        m_leftEdgeHandle->setPos(QPointF(0, 0));
        m_leftEdgeHandle->setRect(0, 0,
                                  m_outerHandleSize,
                                  rect().height());
        m_leftEdgeHandle->m_ignoreGeometryChanges = false;

        m_rightEdgeHandle->m_ignoreGeometryChanges = true;
        m_rightEdgeHandle->setPos(QPointF(rect().width(), 0));
        m_rightEdgeHandle->setRect(-m_outerHandleSize, 0,
                                   m_outerHandleSize,
                                   rect().height());
        m_rightEdgeHandle->m_ignoreGeometryChanges = false;

        m_topRightCornerHandle->m_ignoreGeometryChanges = true;
        m_topRightCornerHandle->setPos(rect().width(), 0);
        m_topRightCornerHandle->setRect(-m_outerHandleSize, 0,
                                        m_outerHandleSize,
                                        m_outerHandleSize);
        m_topRightCornerHandle->m_ignoreGeometryChanges = false;

        m_bottomRightCornerHandle->m_ignoreGeometryChanges = true;
        m_bottomRightCornerHandle->setPos(rect().width(), rect().height());
        m_bottomRightCornerHandle->setRect(-m_outerHandleSize, -m_outerHandleSize,
                                           m_outerHandleSize,
                                           m_outerHandleSize);
        m_bottomRightCornerHandle->m_ignoreGeometryChanges = false;

        m_bottomLeftCornerHandle->m_ignoreGeometryChanges = true;
        m_bottomLeftCornerHandle->setPos(0, rect().height());
        m_bottomLeftCornerHandle->setRect(0, -m_outerHandleSize,
                                          m_outerHandleSize,
                                          m_outerHandleSize);
        m_bottomLeftCornerHandle->m_ignoreGeometryChanges = false;

        m_topLeftCornerHandle->m_ignoreGeometryChanges = true;
        m_topLeftCornerHandle->setPos(0, 0);
        m_topLeftCornerHandle->setRect(0, 0,
                                       m_outerHandleSize,
                                       m_outerHandleSize);
        m_topLeftCornerHandle->m_ignoreGeometryChanges = false;
    }
    m_handlesDirty = false;
}

void ResizableRectItem::scaleChanged(qreal sx, qreal sy)
{
    m_currentScaleX = sx;
    m_currentScaleY = sy;

    for (QGraphicsItem* childItem : childItems()) {
        if (auto* dc = dynamic_cast<ResizingHandle*>(childItem)) {
            dc->scaleChanged(sx, sy);
        }

        if (auto* dc = dynamic_cast<ResizableRectItem*>(childItem)) {
            dc->scaleChanged(sx, sy);
        }
    }

    m_handlesDirty = true;
    ensureHandlesUpdated();
}

QVariant ResizableRectItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        if (value.toBool())
            unselectAllHandles();
        else
            hideAllHandles();
    }

    else if (change == QGraphicsItem::ItemPositionChange) {
        QPointF newPos = constrainMovePoint(value.toPointF());

        if (!m_moveInProgress && !m_ignoreGeometryChanges) {
            m_moveInProgress = true;
            m_moveOldPos = pos();

            setPen(getPenWhileMoving());
            hideAllHandles();

            notifyMoveStarted();
        }

        if (m_moveInProgress) {
            // value is the new position, pos() is the old position
            // we use value to avoid the 1 pixel lag
            notifyMoveProgress(newPos);
        }
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void ResizableRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsRectItem::hoverEnterEvent(event);

    setPen(getHoverPen());
    m_mouseOver = true;
}

void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_mouseOver = false;
    setPen(getNormalPen());

    QGraphicsRectItem::hoverLeaveEvent(event);
}

void ResizableRectItem::mouseReleaseEventSelected(QMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_moveInProgress) {
        m_moveInProgress = false;
        QPointF newPos = pos();
        notifyMoveFinished(newPos);
    }
}


} // namespace resizable
} // namespace CEED
