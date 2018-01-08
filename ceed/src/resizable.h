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

#ifndef CEED_resizable_
#define CEED_resizable_

#include "CEEDBase.h"

#include "settings/settings_init.h"

#include <QGraphicsItem>
#include <QGraphicsView>

using CEED::settings::declaration::Entry;

namespace CEED {
namespace resizable {

class ResizableRectItem;

/*!
\brief GraphicsView

If you plan to use ResizableGraphicsRectItems, make sure you view them
    via a GraphicsView that is inherited from this exact class.

    The reason for that is that The ResizableRectItem needs to counter-scale
    resizing handles

    cegui.GraphicsView inherits from this class because you are likely to use
    resizables on top of CEGUI. If you don't need them, simply don't use them.
    The overhead is minimal.

*/
class GraphicsView : public QGraphicsView
{
public:
    bool m_wheelZoomEnabled;
    float m_zoomFactor;
    bool m_middleButtonDragScrollEnabled;
    QPoint m_lastDragScrollMousePosition;
    Entry* m_ctrlZoom;

   GraphicsView(QWidget* parent = nullptr);

    void setTransform(const QTransform& transform);

    void scaleChanged(qreal sx, qreal sy);

    void performZoom();

    void zoomOriginal();

    void zoomIn();

    void zoomOut();

    void wheelEvent(QWheelEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    /**When mouse is released in a resizable view, we have to
        go through all selected items and notify them of the release.

        This helps track undo movement and undo resize way easier
        */
    void mouseReleaseEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event);
};

/*!
\brief ResizingHandle

A rectangle that when moved resizes the parent resizable rect item.

    The reason to go with a child GraphicsRectItem instead of just overriding mousePressEvent et al
    is to easily support multi selection resizing (you can multi-select various edges in all imaginable
    combinations and resize many things at once).

*/
class ResizingHandle : public QGraphicsRectItem
{
public:
    ResizableRectItem* m_parentResizable;
    bool m_ignoreGeometryChanges;
    bool m_ignoreTransformChanges;
    bool m_mouseOver;

    ResizingHandle(ResizableRectItem* parent);

    virtual QPointF performResizing(const QPointF &value);

    void unselectAllSiblingHandles();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**Called when mouse is released whilst this was selected.
        This notifies us that resizing might have ended.
        */
    void mouseReleaseEventSelected(QMouseEvent* event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    virtual void scaleChanged(qreal sx, qreal sy);
};

/*!
\brief EdgeResizingHandle

Resizing handle positioned on one of the 4 edges

*/
class EdgeResizingHandle : public ResizingHandle
{
public:
    EdgeResizingHandle(ResizableRectItem* parent);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

class TopEdgeResizingHandle : public EdgeResizingHandle
{
public:
    TopEdgeResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;

    void scaleChanged(qreal sx, qreal sy) override;
};

class BottomEdgeResizingHandle : public EdgeResizingHandle
{
public:
    BottomEdgeResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;

    void scaleChanged(qreal sx, qreal sy) override;
};

class LeftEdgeResizingHandle : public EdgeResizingHandle
{
public:
    LeftEdgeResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;

    void scaleChanged(qreal sx, qreal sy) override;
};

class RightEdgeResizingHandle : public EdgeResizingHandle
{
public:
    RightEdgeResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;

    void scaleChanged(qreal sx, qreal sy) override;
};

/*!
\brief CornerResizingHandle

Resizing handle positioned in one of the 4 corners.

*/
class CornerResizingHandle : public ResizingHandle
{
public:
    CornerResizingHandle(ResizableRectItem* parent);

    void scaleChanged(qreal sx, qreal sy) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

class TopRightCornerResizingHandle : public CornerResizingHandle
{
public:
    TopRightCornerResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;
};

class BottomRightCornerResizingHandle : public CornerResizingHandle
{
public:
    BottomRightCornerResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;
};

class BottomLeftCornerResizingHandle : public CornerResizingHandle
{
public:
    BottomLeftCornerResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;
};

class TopLeftCornerResizingHandle : public CornerResizingHandle
{
public:
    TopLeftCornerResizingHandle(ResizableRectItem* parent);

    QPointF performResizing(const QPointF& value) override;
};

/*!
\brief ResizableRectItem

Rectangle that can be resized by dragging it's handles.

    Inherit from this class to gain resizing and moving capabilities.

    Depending on the size, the handles are shown outside the rectangle (if it's small)
    or inside (if it's large). All this is tweakable.

*/
class ResizableRectItem : public QGraphicsRectItem
{
public:
    bool m_mouseOver;

    EdgeResizingHandle* m_topEdgeHandle;
    EdgeResizingHandle* m_bottomEdgeHandle;
    EdgeResizingHandle* m_leftEdgeHandle;
    EdgeResizingHandle* m_rightEdgeHandle;

    CornerResizingHandle* m_topRightCornerHandle;
    CornerResizingHandle* m_bottomRightCornerHandle;
    CornerResizingHandle* m_bottomLeftCornerHandle;
    CornerResizingHandle* m_topLeftCornerHandle;

    bool m_handlesDirty;
    qreal m_currentScaleX;
    qreal m_currentScaleY;

    bool m_ignoreGeometryChanges;

    bool m_resizeInProgress;
    QPointF m_resizeOldPos;
    QRectF m_resizeOldRect;

    bool m_moveInProgress;
    QPointF m_moveOldPos;

    int m_outerHandleSize;
    int m_innerHandleSize;

    ResizableRectItem(QGraphicsItem* parentItem = nullptr);

    virtual QSizeF getMinSize()
    {
        QSizeF ret(1, 1);
        return ret;
    }

    virtual QSizeF getMaxSize()
    {
        return QSizeF(); // was None
    }

    virtual QPen getNormalPen()
    {
        QPen ret;
        ret.setColor(QColor(255, 255, 255, 150));
        ret.setStyle(Qt::DotLine);
        return ret;
    }

    virtual QPen getHoverPen()
    {
        QPen ret;
        ret.setColor(QColor(0, 255, 255, 255));
        return ret;
    }

    virtual QPen getPenWhileResizing()
    {
        QPen ret(QColor(255, 0, 255, 255));
        return ret;
    }

    virtual QPen getPenWhileMoving()
    {
        QPen ret(QColor(255, 0, 255, 255));
        return ret;
    }

    virtual QPen getEdgeResizingHandleHoverPen()
    {
        QPen ret;
        ret.setColor(QColor(0, 255, 255, 255));
        ret.setWidth(2);
        ret.setCosmetic(true);
        return ret;
    }

    virtual QPen getEdgeResizingHandleHiddenPen()
    {
        QPen ret;
        ret.setColor(Qt::transparent);
        return ret;
    }

    virtual QPen getCornerResizingHandleHoverPen()
    {
        QPen ret;
        ret.setColor(QColor(0, 255, 255, 255));
        ret.setWidth(2);
        ret.setCosmetic(true);
        return ret;
    }

    virtual QPen getCornerResizingHandleHiddenPen()
    {
        QPen ret;
        ret.setColor(Qt::transparent);
        return ret;
    }

    void setOuterHandleSize(int size)
    {
        m_outerHandleSize = size;
        m_handlesDirty = true;
    }

    void setInnerHandleSize(int size)
    {
        m_innerHandleSize = size;
        m_handlesDirty = true;
    }

    void setRect(const QRectF& rect)
    {
        QGraphicsRectItem::setRect(rect);

        m_handlesDirty = true;
        ensureHandlesUpdated();
    }

    virtual QPointF constrainMovePoint(const QPointF &point)
    {
        return point;
    }

    virtual QRectF constrainResizeRect(const QRectF& rect_, const QRectF& oldRect);

    /**Adjusts the rectangle and returns a 4-tuple of the actual used deltas
    (with restrictions accounted for)

    The default implementation doesn't use the handle parameter.
    */
    void performResizing(ResizingHandle* handle, qreal deltaX1, qreal deltaY1, qreal deltaX2, qreal deltaY2,
                         QPointF& topLeftDelta, QPointF& bottomRightDelta);

    /**Hides all handles. If a handle is given as the 'excluding' parameter, this handle is
    skipped over when hiding
    */
    void hideAllHandles(ResizingHandle* excluding = nullptr);

    /**Makes it possible to disable or enable resizing
    */
    void setResizingEnabled(bool enabled = true);

    /**Unselects all handles of this resizable*/
    void unselectAllHandles();

    /**A method meant to be overridden when you want to react when a handle is selected
    */
    virtual void notifyHandleSelected(ResizingHandle* handle)
    {
        Q_UNUSED(handle)
    }

    /**Checks whether any of the 8 handles is selected.
    note: At most 1 handle can be selected at a time!*/
    bool isAnyHandleSelected();

    /**Makes sure handles are updated (if possible).
    Updating handles while resizing would mess things up big times, so we just ignore the
    update in that circumstance
    */
    void ensureHandlesUpdated();

    qreal absoluteXToRelative(qreal value, const QTransform& transform);

    qreal absoluteYToRelative(qreal value, const QTransform& transform);

    /**Updates all the handles according to geometry*/
    void updateHandles();

    virtual void notifyResizeStarted()
    {
    }

    virtual void notifyResizeProgress(const QPointF& newPos, const QRectF& newRect)
    {
        Q_UNUSED(newPos)
        Q_UNUSED(newRect)
    }

    virtual void notifyResizeFinished(const QPointF& newPos, const QRectF& newRect)
    {
        m_ignoreGeometryChanges = true;
        setRect(newRect);
        setPos(newPos);
        m_ignoreGeometryChanges = false;
    }

    virtual void notifyMoveStarted()
    {
    }

    virtual void notifyMoveProgress(const QPointF& newPos)
    {
        Q_UNUSED(newPos);
    }

    virtual void notifyMoveFinished(const QPointF& newPos)
    {
        Q_UNUSED(newPos);
    }

    void scaleChanged(qreal sx, qreal sy);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    void mouseReleaseEventSelected(QMouseEvent* event);
};

} // namespace resizable
} // namespace CEED

#endif
