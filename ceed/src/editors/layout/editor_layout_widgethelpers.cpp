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

#include "editor_layout_widgethelpers.h"

#include "action/action__init__.h"

#include "editors/layout/editor_layout_init.h"
#include "editors/layout/editor_layout_visual.h"

namespace CEED {
namespace editors {
namespace layout {
namespace widgethelpers {

QBrush Manipulator::snapGridBrush;
int Manipulator::snapGridX = -1, Manipulator::snapGridY = -1;
QColor Manipulator::snapGridPointColour;
QColor Manipulator::snapGridPointShadowColour;

QBrush Manipulator::getSnapGridBrush()
{
    int snapGridX = settings::getEntry("layout/visual/snap_grid_x")->m_value.toInt();
    int snapGridY = settings::getEntry("layout/visual/snap_grid_y")->m_value.toInt();
    QColor snapGridPointColour = settings::getEntry("layout/visual/snap_grid_point_colour")->m_value.value<QColor>();
    QColor snapGridPointShadowColour = settings::getEntry("layout/visual/snap_grid_point_shadow_colour")->m_value.value<QColor>();

    // if snap grid wasn't created yet or if it's parameters changed, create it anew!
    if ((Manipulator::snapGridX != snapGridX) || (Manipulator::snapGridY != snapGridY) ||
            (Manipulator::snapGridPointColour != snapGridPointColour) ||
            (Manipulator::snapGridPointShadowColour != snapGridPointShadowColour)) {

        Manipulator::snapGridX = snapGridX;
        Manipulator::snapGridY = snapGridY;
        Manipulator::snapGridPointColour = snapGridPointColour;
        Manipulator::snapGridPointShadowColour = snapGridPointShadowColour;

        QPixmap texture(snapGridX, snapGridY);
        texture.fill(QColor(Qt::transparent));

        QPainter painter(&texture);
        painter.setPen(QPen(snapGridPointColour));
        painter.drawPoint(0, 0);
        painter.setPen(QPen(snapGridPointShadowColour));
        painter.drawPoint(1, 0);
        painter.drawPoint(1, 1);
        painter.drawPoint(0, 1);
        painter.end();

        Manipulator::snapGridBrush.setTexture(texture);
    }

    return Manipulator::snapGridBrush;
}

Manipulator::Manipulator(visual::VisualEditing *visual, QGraphicsItem *parent, CEGUI::Window *widget, bool recursive, bool skipAutoWidgets)
    : super(parent, widget, recursive, skipAutoWidgets)
{
    m_visual = visual;
    m_showOutline = true;

    setAcceptDrops(true);

    m_drawSnapGrid = false;
    m_snapGridNonClientArea = false;
    m_ignoreSnapGrid = false;

    m_treeItem = nullptr;

    m_snapGridAction = action::getAction("layout/snap_grid");

    m_absoluteModeAction = action::getAction("layout/absolute_mode");
    m_absoluteModeActionConnection = QObject::connect(m_absoluteModeAction, &QAction::toggled, [=](bool checked){ slot_absoluteModeToggled(checked); });

    m_absoluteIntegersOnlyModeAction = action::getAction("layout/abs_integers_mode");

    setPen(getNormalPen()); // NOTE: virtual method in constructor
}

Manipulator::~Manipulator()
{
    QObject::disconnect(m_absoluteModeActionConnection);
}

void Manipulator::slot_absoluteModeToggled(bool checked)
{
    Q_UNUSED(checked)

    // immediately update if possible
    if (m_resizeInProgress) {
        notifyResizeProgress(m_lastResizeNewPos, m_lastResizeNewRect);
        update();
    }

    if (m_moveInProgress) {
        notifyMoveProgress(m_lastMoveNewPos);
        update();
    }
}

QPen Manipulator::getNormalPen()
{
    if (m_showOutline) {
        QPen pen = settings::getEntry("layout/visual/normal_outline")->m_value.value<QPen>();
        pen.setCosmetic(true);
        return pen;
    }
    return QColor(0, 0, 0, 0);
}

QPen Manipulator::getHoverPen()
{
    if (m_showOutline) {
        QPen pen = settings::getEntry("layout/visual/hover_outline")->m_value.value<QPen>();
        pen.setCosmetic(true);
        return pen;
    }
    return QColor(0, 0, 0, 0);
}

QPen Manipulator::getPenWhileResizing()
{
    QPen pen = settings::getEntry("layout/visual/resizing_outline")->m_value.value<QPen>();
    pen.setCosmetic(true);
    return pen;
}

QPen Manipulator::getPenWhileMoving()
{
    QPen pen = settings::getEntry("layout/visual/moving_outline")->m_value.value<QPen>();
    pen.setCosmetic(true);
    return pen;
}

QPen Manipulator::getDragAcceptableHintPen()
{
    QPen ret;
    ret.setColor(QColor(255, 255, 0));
    ret.setCosmetic(true);
    return ret;
}

QString Manipulator::getUniqueChildWidgetName(const QString &base)
{
    QString candidate = base;

    if (m_widget == nullptr)
        // we can't check for duplicates in this case
        return candidate;

    int i = 2;
    while (m_widget->isChild(FROM_QSTR(candidate))) {
        candidate = QString("%1%2").arg(base).arg(i);
        i += 1;
    }

    return candidate;
}

Manipulator *Manipulator::createChildManipulator(CEGUI::Window *childWidget, bool recursive, bool skipAutoWidgets)
{
    Manipulator* ret = new Manipulator(m_visual, this, childWidget, recursive, skipAutoWidgets);
    if (recursive)
        ret->createChildManipulators(true, skipAutoWidgets);
    ret->updateFromWidget();
    return ret;
}

void Manipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
    bool parentWidgetWasNone = m_widget->getParent() == nullptr;

    super::detach(detachWidget, destroyWidget, recursive);

    if (parentWidgetWasNone) {
        // if this was root we have to inform the scene accordingly!
        m_visual->setRootWidgetManipulator(nullptr);
    }
}

void Manipulator::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-ceed-widget-type")) {
        event->acceptProposedAction();

        setPen(getDragAcceptableHintPen());

    } else {
        event->ignore();
    }
}

void Manipulator::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
    setPen(getNormalPen());
}

void Manipulator::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    QByteArray data = event->mimeData()->data("application/x-ceed-widget-type");

    if (auto wtmd = dynamic_cast<const visual::WidgetTypeMimeData*>(event->mimeData())) {
        QString widgetType = wtmd->type;

        auto cmd = new undo::CreateCommand(m_visual, TO_QSTR(m_widget->getNamePath()), widgetType,
                                           getUniqueChildWidgetName(widgetType.section('/', -1)));
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);

        event->acceptProposedAction();

    } else {
        event->ignore();
    }
}

void Manipulator::impl_paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    super::impl_paint(painter, option, widget);

    if (m_drawSnapGrid && m_snapGridAction->isChecked()) {
        auto childRect = m_widget->getChildContentArea(m_snapGridNonClientArea).get();
        QRectF qChildRect(childRect.d_min.d_x, childRect.d_min.d_y, childRect.getWidth(), childRect.getHeight());
        qChildRect.translate(-scenePos());

        painter->save();
        painter->setBrushOrigin(qChildRect.topLeft());
        painter->fillRect(qChildRect, getSnapGridBrush());
        painter->restore();
    }
}

void Manipulator::triggerPropertyManagerCallback(const QSet<QString> &propertyNames)
{
    CEGUI::Window* widget = m_widget;

#if 1
    auto propertyManager = m_visual->m_propertiesDockWidget->m_inspector->m_propertyManager;
    propertyManager->triggerPropertyManagerCallback(widget, propertyNames);
#else
    // if the property manager has set callbacks on this widget
    if (hasattr(widget, "propertyManagerCallbacks")) {
        for (QString propertyName : propertyNames) {
            // if there's a callback for this property
            if (widget->propertyManagerCallbacks.contains(propertyName)) {
                // call it
                widget->propertyManagerCallbacks[propertyName]();
            }
        }
    }
#endif
}

void Manipulator::updateFromWidget(bool callUpdate, bool updateAncestorLCs)
{
    // we are updating the position and size from widget, we don't want any snapping
    m_ignoreSnapGrid = true;
    super::updateFromWidget(callUpdate, updateAncestorLCs);
    m_ignoreSnapGrid = false;

    m_showOutline = true;
#if 1
    bool locked = m_treeItem != nullptr && m_treeItem->data(Qt::CheckStateRole).toBool();
    if (locked) {
        setFlags(flags() & ~QGraphicsItem::ItemIsFocusable);
        setFlags(flags() & ~QGraphicsItem::ItemIsSelectable);
        setFlags(flags() & ~QGraphicsItem::ItemIsMovable);
    } else {
        setFlags(flags() | QGraphicsItem::ItemIsFocusable);
        setFlags(flags() | QGraphicsItem::ItemIsSelectable);
        setFlags(flags() | QGraphicsItem::ItemIsMovable);
    }
    setResizingEnabled(!locked);
#else
    setFlags(flags() | QGraphicsItem::ItemIsFocusable);
    setFlags(flags() | QGraphicsItem::ItemIsSelectable);
    setFlags(flags() | QGraphicsItem::ItemIsMovable);
    setFlags(flags() & ~QGraphicsItem::ItemHasNoContents);
    setResizingEnabled(true);
#endif
    if (m_widget->isAutoWindow()) {
        if (!settings::getEntry("layout/visual/auto_widgets_show_outline")->m_value.toBool()) {
            // don't show outlines unless instructed to do so
            m_showOutline = false;
        }

        if (!settings::getEntry("layout/visual/auto_widgets_selectable")->m_value.toBool()) {
            // make this widget not focusable, selectable, movable and resizable
            setFlags(flags() & ~QGraphicsItem::ItemIsFocusable);
            setFlags(flags() & ~QGraphicsItem::ItemIsSelectable);
            setFlags(flags() & ~QGraphicsItem::ItemIsMovable);
            setFlags(flags() |  QGraphicsItem::ItemHasNoContents);
            setResizingEnabled(false);
        }
    }

    if (dynamic_cast<CEGUI::LayoutContainer*>(m_widget)) {
        // LayoutContainers change their size to fit the widgets, it makes
        // no sense to show this size
        m_showOutline = false;
        // And it makes no sense to resize them, they will just snap back
        // when they relayout
        setResizingEnabled(false);
    }

    CEGUI::Window* parent = m_widget->getParent();
    if (parent != nullptr && dynamic_cast<CEGUI::LayoutContainer*>(m_widget)) {
        // if the widget is now parented inside a layout container we don't want
        // any drag moving to be possible
        setFlags(flags() & ~QGraphicsItem::ItemIsMovable);
    }
}

qreal Manipulator::snapXCoordToGrid(qreal x)
{
    // we have to take the child rect into account
    CEGUI::Rectf childRect = m_widget->getChildContentArea(m_snapGridNonClientArea).get();
    qreal xOffset = childRect.d_min.d_x - scenePos().x();

    // point is in local space
    int snapGridX = settings::getEntry("layout/visual/snap_grid_x")->m_value.toInt();
    return xOffset + round((x - xOffset) / snapGridX) * snapGridX;
}

qreal Manipulator::snapYCoordToGrid(qreal y)
{
    // we have to take the child rect into account
    CEGUI::Rectf childRect = m_widget->getChildContentArea(m_snapGridNonClientArea).get();
    qreal yOffset = childRect.d_min.d_y - scenePos().y();

    // point is in local space
    int snapGridY = settings::getEntry("layout/visual/snap_grid_y")->m_value.toInt();
    return yOffset + round((y - yOffset) / snapGridY) * snapGridY;
}

QPointF Manipulator::constrainMovePoint(const QPointF &point_)
{
    QPointF point = point_;
    if (!m_ignoreSnapGrid && m_snapGridAction->isChecked()) {
        QGraphicsItem* parent = parentItem();
        if (parent == nullptr)
            // ad hoc snapping for root widget, it snaps to itself
            parent = this;

        if (Manipulator* manip = dynamic_cast<Manipulator*>(parent)) {
            point = QPointF(manip->snapXCoordToGrid(point.x()), manip->snapYCoordToGrid(point.y()));
        }
    }

    point = super::constrainMovePoint(point);

    return point;
}

QRectF Manipulator::constrainResizeRect(const QRectF &rect_, const QRectF &oldRect)
{
    QRectF rect = rect_;

    // we constrain all 4 "corners" to the snap grid if needed
    if (!m_ignoreSnapGrid && m_snapGridAction->isChecked()) {
        QGraphicsItem* parent = parentItem();
        if (parent == nullptr)
            // ad hoc snapping for root widget, it snaps to itself
            parent = this;

        if (Manipulator* manip = dynamic_cast<Manipulator*>(parent)) {
            // we only snap the coordinates that have changed
            // because for example when you drag the left edge you don't want the right edge to snap!

            // we have to add the position coordinate as well to ensure the snap is precisely at the guide point
            // it is subtracted later on because the rect is relative to the item position

            if (rect.left() != oldRect.left())
                rect.setLeft(manip->snapXCoordToGrid(pos().x() + rect.left()) - pos().x());
            if (rect.top() != oldRect.top())
                rect.setTop(manip->snapYCoordToGrid(pos().y() + rect.top()) - pos().y());

            if (rect.right() != oldRect.right())
                rect.setRight(manip->snapXCoordToGrid(pos().x() + rect.right()) - pos().x());
            if (rect.bottom() != oldRect.bottom())
                rect.setBottom(manip->snapYCoordToGrid(pos().y() + rect.bottom()) - pos().y());
        }
    }

    rect = super::constrainResizeRect(rect, oldRect);

    return rect;
}

/////

SerialisationData::SerialisationData(visual::VisualEditing *visual, CEGUI::Window *widget, bool serialiseChildren)
    : cegui::widgethelpers::SerialisationData(widget, serialiseChildren)
    , m_visual(visual)
{
}

SerialisationData *SerialisationData::createChildData(CEGUI::Window *widget, bool serialiseChildren)
{
    auto data = new SerialisationData(m_visual, widget, serialiseChildren);
    if (serialiseChildren)
        data->serialiseChildren(widget);
    return data;
}

cegui::widgethelpers::Manipulator *SerialisationData::createManipulator(cegui::widgethelpers::Manipulator *parentManipulator,
                                                                        CEGUI::Window *widget, bool recursive, bool skipAutoWidgets)
{
    auto* ret = new Manipulator(m_visual, parentManipulator, widget, recursive, skipAutoWidgets);
    if (recursive)
        ret->createChildManipulators(true, skipAutoWidgets);
    ret->updateFromWidget();
    return ret;
}

void SerialisationData::setVisual(visual::VisualEditing *visual)
{
    m_visual = visual;

    for (auto* child : m_children) {
        dynamic_cast<SerialisationData*>(child)->setVisual(visual);
    }
}

Manipulator *SerialisationData::reconstruct(Manipulator *rootManipulator)
{
    cegui::widgethelpers::Manipulator* ret = cegui::widgethelpers::SerialisationData::reconstruct(rootManipulator);

    if (ret->parentItem() == nullptr) {
        // this is a root widget being reconstructed, handle this accordingly
        m_visual->setRootWidgetManipulator(dynamic_cast<widgethelpers::Manipulator*>(ret));
    }

    return dynamic_cast<widgethelpers::Manipulator*>(ret);
}


} // namespae widgethelpers
} // namespace layout
} // namespace editors
} // namespace CEED
