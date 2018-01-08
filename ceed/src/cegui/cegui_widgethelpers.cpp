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

#include "cegui_widgethelpers.h"

namespace CEED {
namespace cegui {
namespace widgethelpers {

Manipulator::Manipulator(QGraphicsItem *parent, CEGUI::Window *widget, bool recursive, bool skipAutoWidgets)
    : super(parent)
{
    setFlags(QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);

    m_widget = widget;

#if 0 // Can't call virtual function from a constructor
    if (recursive)
        createChildManipulators(true, skipAutoWidgets);
#endif

#if 0
    m_preResizePos = None;
    m_preResizeSize = None;
    m_lastResizeNewPos = None;
    m_lastResizeNewRect = None;

    m_preMovePos = None;
    m_lastMoveNewPos = None;
#endif

    setPen(getNormalPen()); // NOTE: virtual method in constructor
}

Manipulator *Manipulator::createChildManipulator(CEGUI::Window *childWidget, bool recursive, bool skipAutoWidgets)
{
    auto* ret = new Manipulator(this, childWidget, recursive, skipAutoWidgets);
    if (recursive)
        ret->createChildManipulators(true, skipAutoWidgets);
    ret->updateFromWidget();
    return ret;
}

void Manipulator::getFunctionsChildCountAndChildGet(std::function<size_t()>& countGetter, std::function<CEGUI::Window*(size_t)> &childGetter)
{
    using namespace std::placeholders;
    if (auto dc = dynamic_cast<CEGUI::TabControl*>(m_widget)) {
        countGetter = std::bind(&CEGUI::TabControl::getTabCount, dc);
        childGetter = std::bind(&CEGUI::TabControl::getTabContentsAtIndex, dc, _1);
    } else if (auto dc = dynamic_cast<CEGUI::ScrollablePane*>(m_widget)) {
        countGetter = std::bind(&CEGUI::ScrolledContainer::getChildCount, dc->getContentPane());
        childGetter = std::bind(&CEGUI::ScrolledContainer::getChildAtIdx, dc->getContentPane(), _1);
    } else {
        countGetter = std::bind(&CEGUI::Window::getChildCount, m_widget);
        childGetter = std::bind(&CEGUI::Window::getChildAtIdx, m_widget, _1);
    }
}

void Manipulator::createChildManipulators(bool recursive, bool skipAutoWidgets)
{
    std::function<size_t()> countGetter;
    std::function<CEGUI::Window*(size_t)> childGetter;
    getFunctionsChildCountAndChildGet(countGetter, childGetter);

    for (int idx = 0; idx < countGetter(); idx++) {
        CEGUI::Window* childWidget = childGetter(idx);

        if (!skipAutoWidgets || !childWidget->isAutoWindow()) {
            // note: we don't have to assign or attach the child manipulator here
            //       just passing parent to the constructor is enough
            createChildManipulator(childWidget, recursive, skipAutoWidgets);
        }
    }
}

void Manipulator::createMissingChildManipulators(bool recursive, bool skipAutoWidgets)
{
    std::function<size_t()> countGetter;
    std::function<CEGUI::Window*(size_t)> childGetter;
    getFunctionsChildCountAndChildGet(countGetter, childGetter);

    for (int idx = 0; idx < countGetter(); idx++) {
        CEGUI::Window* childWidget = childGetter(idx);

        try {
            // try to find a manipulator for currently examined child widget
            getManipulatorByPath(TO_QSTR(childWidget->getName()));

        } catch (LookupError e) {
            if (!skipAutoWidgets || !childWidget->isAutoWindow()) {
                // note: we don't have to assign or attach the child manipulator here
                //       just passing parent to the constructor is enough
                Manipulator* childManipulator = createChildManipulator(childWidget, recursive, skipAutoWidgets);
                if (recursive)
                    childManipulator->createMissingChildManipulators(true, skipAutoWidgets);
            }
        }
    }
}

void Manipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
    // descend if recursive
    if (recursive)
        detachChildManipulators(detachWidget, destroyWidget, true);

    // detach from the GUI hierarchy
    if (detachWidget) {
        CEGUI::Window* parentWidget = m_widget->getParent();
        if (parentWidget != nullptr)
            parentWidget->removeChild(m_widget);
    }

    // detach from the parent manipulator
    scene()->removeItem(this);

    if (detachWidget && destroyWidget) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_widget);
        m_widget = nullptr;
    }
}

void Manipulator::detachChildManipulators(bool detachWidget, bool destroyWidget, bool recursive)
{
    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_))
            child->detach(detachWidget, destroyWidget, recursive);
    }
}

Manipulator *Manipulator::getManipulatorByPath(const QString &widgetPath)
{
    if (dynamic_cast<CEGUI::TabControl*>(m_widget) || dynamic_cast<CEGUI::ScrollablePane*>(m_widget)) {
        Manipulator* manipulator = getManipulatorFromChildContainerByPath(widgetPath);
        if (manipulator != nullptr)
            return manipulator;
    }

    QString path0 = widgetPath.section('/', 0, 0);
    QString path1 = widgetPath.section('/', 1);
    Q_ASSERT(!path0.isEmpty());

    QString baseName = path0;
    QString remainder = path1;

    for (QGraphicsItem* item_ : childItems()) {
        if (Manipulator* item = dynamic_cast<Manipulator*>(item_)) {
            if (TO_QSTR(item->m_widget->getName()) == baseName) {
                if (remainder == "")
                    return item;
                else
                    return item->getManipulatorByPath(remainder);
            }
        }
    }

    throw LookupError("Can't find widget manipulator of path '" + widgetPath + "'");
}

Manipulator *Manipulator::getManipulatorFromChildContainerByPath(const QString &widgetPath)
{
    QString contentPaneChildPath = widgetPath.section('/', 0, 0);
    Q_ASSERT(!contentPaneChildPath.isEmpty());
    QString directChildPath = widgetPath.section('/', 1);

    for (QGraphicsItem* item_ : childItems()) {
        if (Manipulator* item = dynamic_cast<Manipulator*>(item_)) {
            if (item->m_widget->getName() == FROM_QSTR(directChildPath))
                return item;
        }
    }

    return nullptr;
}

QList<Manipulator *> Manipulator::getChildManipulators()
{
    QList<Manipulator*> ret;

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            ret.append(child);
        }
    }

    return ret;
}

QList<Manipulator *> Manipulator::getAllDescendantManipulators()
{
    QList<Manipulator*> ret;

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            ret += child;
            ret += child->getAllDescendantManipulators();
        }
    }

    return ret;
}

void Manipulator::updateFromWidget(bool callUpdate, bool updateAncestorLCs)
{
    Q_ASSERT(m_widget != nullptr);

    if (callUpdate)
        m_widget->update(0.0f);

    if (updateAncestorLCs) {
        // We are trying to find a topmost LC (in case of nested LCs) and
        // recursively update it

        QGraphicsItem* item = parentItem();
        Manipulator* parentManip = dynamic_cast<Manipulator*>(item);
        Manipulator* topmostLC = nullptr;
        while ((parentManip != nullptr) && dynamic_cast<CEGUI::LayoutContainer*>(parentManip->m_widget)) {
            topmostLC = parentManip;
            item = item->parentItem();
            parentManip = dynamic_cast<Manipulator*>(item);
        }

        if (topmostLC != nullptr) {
            topmostLC->updateFromWidget(true, false);

            // No need to continue, this method will get called again with
            // updateAncestorLCs = false
            return;
        }
    }

    CEGUI::Rectf unclippedOuterRect = m_widget->getUnclippedOuterRect().getFresh(true);
    CEGUI::Vector2f pos = unclippedOuterRect.getPosition();
    CEGUI::Sizef size = unclippedOuterRect.getSize();

    CEGUI::Window* parentWidget = m_widget->getParent();
    if (parentWidget != nullptr) {
        CEGUI::Rectf parentUnclippedOuterRect = parentWidget->getUnclippedOuterRect().get();
        pos -= parentUnclippedOuterRect.getPosition();
    }

    m_ignoreGeometryChanges = true;
    setPos(QPointF(pos.d_x, pos.d_y));
    setRect(QRectF(0, 0, size.d_width, size.d_height));
    m_ignoreGeometryChanges = false;

    for (QGraphicsItem* child_ : childItems()) {
        Manipulator* child = dynamic_cast<Manipulator*>(child_);
        if (child == nullptr) {
            continue;
        }

        // if we are updating top to bottom we don't need to update ancestor
        // layout containers, they will already be updated
        child->updateFromWidget(callUpdate, false);
    }
}

void Manipulator::moveToFront()
{
    m_widget->moveToFront();

    QGraphicsItem* parentItem = this->parentItem();
    if (parentItem != nullptr) {
        for (QGraphicsItem* item : parentItem->childItems()) {
            if (item == this)
                continue;

            // For some reason this is the opposite of what (IMO) it should be
            // which is stackBefore(item)
            //
            // Is Qt documentation flawed or something?!
            item->stackBefore(this);
        }

        dynamic_cast<Manipulator*>(parentItem)->moveToFront();
    }
}

QVariant Manipulator::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        if (value.toBool())
            moveToFront();
    }

    return super::itemChange(change, value);
}

void Manipulator::notifyHandleSelected(resizable::ResizingHandle *handle)
{
    super::notifyHandleSelected(handle);

    moveToFront();
}

QSizeF Manipulator::getMinSize()
{
    if (m_widget) {
        CEGUI::Sizef minPixelSize = CEGUI::CoordConverter::asAbsolute(m_widget->getMinSize(),
                                                                      CEGUI::System::getSingleton().getRenderer()->getDisplaySize());

        return QSizeF(minPixelSize.d_width, minPixelSize.d_height);
    }
    return QSizeF();
}

QSizeF Manipulator::getMaxSize()
{
    if (m_widget) {
        CEGUI::Sizef maxPixelSize = CEGUI::CoordConverter::asAbsolute(m_widget->getMaxSize(),
                                                         CEGUI::System::getSingleton().getRenderer()->getDisplaySize());

        return QSizeF(maxPixelSize.d_width, maxPixelSize.d_height);
    }
    return QSizeF();
}

CEGUI::Sizef Manipulator::getBaseSize()
{
    if (m_widget->getParent() != nullptr && !m_widget->isNonClient()) {
        return m_widget->getParent()->getUnclippedInnerRect().get().getSize();
    } else {
        return m_widget->getParentPixelSize();
    }
}

void Manipulator::notifyResizeStarted()
{
    super::notifyResizeStarted();

    m_preResizePos = m_widget->getPosition();
    m_preResizeSize = m_widget->getSize();

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            child->setVisible(false);
        }
    }

    CEGUI::Window* parent = m_widget->getParent();
    if (parent != nullptr && dynamic_cast<CEGUI::LayoutContainer*>(parent)) {
        // hide siblings in the same layout container
        for (QGraphicsItem* item : parentItem()->childItems()) {
            if ((item != this) && dynamic_cast<Manipulator*>(item)) {
                item->setVisible(false);
            }
        }
    }
}

void Manipulator::notifyResizeProgress(const QPointF &newPos, const QRectF &newRect)
{
    super::notifyResizeProgress(newPos, newRect);

    // absolute pixel deltas
    QPointF pixelDeltaPos = newPos - m_resizeOldPos;
    QSizeF pixelDeltaSize = newRect.size() - m_resizeOldRect.size();

    CEGUI::UVector2 deltaPos;
    CEGUI::USize deltaSize;

    if (useAbsoluteCoordsForResize()) {
        if (useIntegersForAbsoluteResize()) {
            deltaPos = CEGUI::UVector2(CEGUI::UDim(0, std::floor(pixelDeltaPos.x())), CEGUI::UDim(0, std::floor(pixelDeltaPos.y())));
            deltaSize = CEGUI::USize(CEGUI::UDim(0, std::floor(pixelDeltaSize.width())), CEGUI::UDim(0, std::floor(pixelDeltaSize.height())));
        } else {
            deltaPos = CEGUI::UVector2(CEGUI::UDim(0, pixelDeltaPos.x()), CEGUI::UDim(0, pixelDeltaPos.y()));
            deltaSize = CEGUI::USize(CEGUI::UDim(0, pixelDeltaSize.width()), CEGUI::UDim(0, pixelDeltaSize.height()));
        }
    } else {
        CEGUI::Sizef baseSize = getBaseSize();

        deltaPos = CEGUI::UVector2(CEGUI::UDim(pixelDeltaPos.x() / baseSize.d_width, 0), CEGUI::UDim(pixelDeltaPos.y() / baseSize.d_height, 0));
        deltaSize = CEGUI::USize(CEGUI::UDim(pixelDeltaSize.width() / baseSize.d_width, 0), CEGUI::UDim(pixelDeltaSize.height() / baseSize.d_height, 0));
    }

    // because the Qt manipulator is always top left aligned in the CEGUI sense,
    // we have to process the size to factor in alignments if they differ
    CEGUI::UVector2 processedDeltaPos;

    CEGUI::HorizontalAlignment hAlignment = m_widget->getHorizontalAlignment();
    if (hAlignment == CEGUI::HorizontalAlignment::HA_LEFT)
        processedDeltaPos.d_x = deltaPos.d_x;
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_CENTRE)
        processedDeltaPos.d_x = deltaPos.d_x + CEGUI::UDim(0.5, 0.5) * deltaSize.d_width;
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_RIGHT)
        processedDeltaPos.d_x = deltaPos.d_x + deltaSize.d_width;
    else
        Q_ASSERT(false);

    CEGUI::VerticalAlignment vAlignment = m_widget->getVerticalAlignment();
    if (vAlignment == CEGUI::VerticalAlignment::VA_TOP)
        processedDeltaPos.d_y = deltaPos.d_y;
    else if (vAlignment == CEGUI::VerticalAlignment::VA_CENTRE)
        processedDeltaPos.d_y = deltaPos.d_y + CEGUI::UDim(0.5, 0.5) * deltaSize.d_height;
    else if (vAlignment == CEGUI::VerticalAlignment::VA_BOTTOM)
        processedDeltaPos.d_y = deltaPos.d_y + deltaSize.d_height;
    else
        Q_ASSERT(false);

    m_widget->setPosition(*m_preResizePos + processedDeltaPos);
    m_widget->setSize(*m_preResizeSize + deltaSize);

    m_lastResizeNewPos = newPos;
    m_lastResizeNewRect = newRect;
}

void Manipulator::notifyResizeFinished(const QPointF &newPos, const QRectF &newRect)
{
    super::notifyResizeFinished(newPos, newRect);

    updateFromWidget();

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            child->updateFromWidget();
            child->setVisible(true);
        }
    }

    CEGUI::Window* parent = m_widget->getParent();
    if (parent != nullptr && dynamic_cast<CEGUI::LayoutContainer*>(parent)) {
        // show siblings in the same layout container
        for (QGraphicsItem* child : childItems()) {
            if (child != this && dynamic_cast<Manipulator*>(child)) {
                child->setVisible(true);
            }
        }

        dynamic_cast<Manipulator*>(parentItem())->updateFromWidget(true);
    }

    m_lastResizeNewPos = QPointF();
    m_lastResizeNewRect = QRectF();
}

void Manipulator::notifyMoveStarted()
{
    super::notifyMoveStarted();

    m_preMovePos = m_widget->getPosition();

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            child->setVisible(false);
        }
    }
}

void Manipulator::notifyMoveProgress(const QPointF &newPos)
{
    super::notifyMoveProgress(newPos);

    // absolute pixel deltas
    QPointF pixelDeltaPos = newPos - m_moveOldPos;

    CEGUI::UVector2 deltaPos;
    if (useAbsoluteCoordsForMove()) {
        if (useIntegersForAbsoluteMove())
            deltaPos = CEGUI::UVector2(CEGUI::UDim(0, std::floor(pixelDeltaPos.x())), CEGUI::UDim(0, std::floor(pixelDeltaPos.y())));
        else
            deltaPos = CEGUI::UVector2(CEGUI::UDim(0, pixelDeltaPos.x()), CEGUI::UDim(0, pixelDeltaPos.y()));
    } else {
        CEGUI::Sizef baseSize = getBaseSize();

        deltaPos = CEGUI::UVector2(CEGUI::UDim(pixelDeltaPos.x() / baseSize.d_width, 0), CEGUI::UDim(pixelDeltaPos.y() / baseSize.d_height, 0));
    }

    m_widget->setPosition(*m_preMovePos + deltaPos);

    m_lastMoveNewPos = newPos;
}

void Manipulator::notifyMoveFinished(const QPointF &newPos)
{
    super::notifyMoveFinished(newPos);

    updateFromWidget();

    for (QGraphicsItem* child_ : childItems()) {
        if (Manipulator* child = dynamic_cast<Manipulator*>(child_)) {
            child->updateFromWidget();
            child->setVisible(true);
        }
    }

    m_lastMoveNewPos = QPointF(); // was None
}

QPainterPath Manipulator::boundingClipPath()
{
    QPainterPath ret;
    ret.addRect(boundingRect());

    return ret;
}

bool Manipulator::isAboveItem(QGraphicsItem *item)
{
    // undecidable otherwise
    Q_ASSERT(item->scene() == scene());

    // FIXME: nasty nasty way to do this
    for (QGraphicsItem* i : scene()->items()) {
        if (i == this)
            return true;
        if (i == item)
            return false;
    }

    Q_ASSERT(false);
    return false;
}

void Manipulator::paintHorizontalGuides(CEGUI::Sizef baseSize, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    CEGUI::UVector2 widgetPosition = m_widget->getPosition();
    CEGUI::USize widgetSize = m_widget->getSize();

    // x coordinate
    float scaleXInPixels = CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetPosition.d_x.d_scale, 0), baseSize.d_width);
    float offsetXInPixels = widgetPosition.d_x.d_offset;

    // width
    float scaleWidthInPixels = CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetSize.d_width.d_scale, 0), baseSize.d_width);
    float offsetWidthInPixels = widgetSize.d_width.d_offset;

    CEGUI::HorizontalAlignment hAlignment = m_widget->getHorizontalAlignment();
    QPointF startXPoint;
    if (hAlignment == CEGUI::HorizontalAlignment::HA_LEFT)
        startXPoint = (rect().topLeft() + rect().bottomLeft()) / 2;
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_CENTRE)
        startXPoint = rect().center();
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_RIGHT)
        startXPoint = (rect().topRight() + rect().bottomRight()) / 2;
    else
        Q_ASSERT(false);

    QPointF midXPoint = startXPoint - QPointF(offsetXInPixels, 0);
    QPointF endXPoint = midXPoint - QPointF(scaleXInPixels, 0);
    QPointF xOffset = (scaleXInPixels * offsetXInPixels < 0) ? QPointF(0, 1) : QPointF(0, 0);

    QPen pen;
    // 0 means 1px size no matter the transformation
    pen.setWidth(0);
    pen.setColor(QColor(0, 255, 0, 255));
    painter->setPen(pen);
    painter->drawLine(startXPoint, midXPoint);
    pen.setColor(QColor(255, 0, 0, 255));
    painter->setPen(pen);
    painter->drawLine(midXPoint + xOffset, endXPoint + xOffset);

    CEGUI::VerticalAlignment vAlignment = m_widget->getVerticalAlignment();
    QPointF startWPoint;
    if (vAlignment == CEGUI::VerticalAlignment::VA_TOP)
        startWPoint = rect().bottomLeft();
    else if (vAlignment == CEGUI::VerticalAlignment::VA_CENTRE)
        startWPoint = rect().bottomLeft();
    else if (vAlignment == CEGUI::VerticalAlignment::VA_BOTTOM)
        startWPoint = rect().topLeft();
    else
        Q_ASSERT(false);

    QPointF midWPoint = startWPoint + QPointF(scaleWidthInPixels, 0);
    QPointF endWPoint = midWPoint + QPointF(offsetWidthInPixels, 0);
    QPointF wOffset;
    // FIXME: epicly unreadable
    if (scaleWidthInPixels * offsetWidthInPixels < 0) {
        if (vAlignment == CEGUI::VerticalAlignment::VA_BOTTOM)
            wOffset = QPointF(0, -1);
        else
            wOffset = QPointF(0, 1);
    } else {
        wOffset = QPointF(0, 0);
    }

    // 0 means 1px size no matter the transformation
    pen.setWidth(0);
    pen.setColor(QColor(255, 0, 0, 255));
    painter->setPen(pen);
    painter->drawLine(startWPoint, midWPoint);
    pen.setColor(QColor(0, 255, 0, 255));
    painter->setPen(pen);
    painter->drawLine(midWPoint + wOffset, endWPoint + wOffset);
}

void Manipulator::paintVerticalGuides(CEGUI::Sizef baseSize, QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    CEGUI::UVector2 widgetPosition = m_widget->getPosition();
    CEGUI::USize widgetSize = m_widget->getSize();

    // y coordinate
    float scaleYInPixels = CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetPosition.d_y.d_scale, 0), baseSize.d_height);
    float offsetYInPixels = widgetPosition.d_y.d_offset;

    // height
    float scaleHeightInPixels = CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetSize.d_height.d_scale, 0), baseSize.d_height);
    float offsetHeightInPixels = widgetSize.d_height.d_offset;

    CEGUI::VerticalAlignment vAlignment = m_widget->getVerticalAlignment();
    QPointF startYPoint;
    if (vAlignment == CEGUI::VerticalAlignment::VA_TOP)
        startYPoint = (rect().topLeft() + rect().topRight()) / 2;
    else if (vAlignment == CEGUI::VerticalAlignment::VA_CENTRE)
        startYPoint = rect().center();
    else if (vAlignment == CEGUI::VerticalAlignment::VA_BOTTOM)
        startYPoint = (rect().bottomLeft() + rect().bottomRight()) / 2;
    else
        Q_ASSERT(false);

    QPointF midYPoint = startYPoint - QPointF(0, offsetYInPixels);
    QPointF endYPoint = midYPoint - QPointF(0, scaleYInPixels);
    QPointF yOffset;
    if (scaleYInPixels * offsetYInPixels < 0)
        yOffset = QPointF(1, 0);
    else
        yOffset = QPointF(0, 0);

    QPen pen;
    // 0 means 1px size no matter the transformation
    pen.setWidth(0);
    pen.setColor(QColor(0, 255, 0, 255));
    painter->setPen(pen);
    painter->drawLine(startYPoint, midYPoint);
    pen.setColor(QColor(255, 0, 0, 255));
    painter->setPen(pen);
    painter->drawLine(midYPoint + yOffset, endYPoint + yOffset);

    CEGUI::HorizontalAlignment hAlignment = m_widget->getHorizontalAlignment();
    QPointF startHPoint;
    if (hAlignment == CEGUI::HorizontalAlignment::HA_LEFT)
        startHPoint = rect().topRight();
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_CENTRE)
        startHPoint = rect().topRight();
    else if (hAlignment == CEGUI::HorizontalAlignment::HA_RIGHT)
        startHPoint = rect().topLeft();
    else
        Q_ASSERT(false);

    QPointF midHPoint = startHPoint + QPointF(0, scaleHeightInPixels);
    QPointF endHPoint = midHPoint + QPointF(0, offsetHeightInPixels);
    QPointF hOffset;
    // FIXME: epicly unreadable
    if (scaleHeightInPixels * offsetHeightInPixels < 0) {
        if (hAlignment == CEGUI::HorizontalAlignment::HA_RIGHT)
            hOffset = QPointF(-1, 0);
        else
            hOffset = QPointF(1, 0);
    } else {
        hOffset = QPointF(0, 0);
    }

    // 0 means 1px size no matter the transformation
    pen.setWidth(0);
    pen.setColor(QColor(255, 0, 0, 255));
    painter->setPen(pen);
    painter->drawLine(startHPoint, midHPoint);
    pen.setColor(QColor(0, 255, 0, 255));
    painter->setPen(pen);
    painter->drawLine(midHPoint + hOffset, endHPoint + hOffset);
}

void Manipulator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    if (getPreventManipulatorOverlap()) {
        // NOTE: This is just an option because it's very performance intensive, most people editing big layouts will
        //       want this disabled. But it makes editing nicer and fancier :-)

        // We are drawing the outlines after CEGUI has already been rendered so he have to clip overlapping parts
        // we basically query all items colliding with ourselves and if that's a manipulator and is over us we subtract
        // that from the clipped path.
        QPainterPath clipPath;
        clipPath.addRect(QRectF(-scenePos().x(), -scenePos().y(), scene()->sceneRect().width(), scene()->sceneRect().height()));
        // FIXME: I used self.collidingItems() but that seems way way slower than just going over everything on the scene
        //        in reality we need siblings of ancestors recursively up to the top
        //
        //        this just begs for optimisation in the future
        auto collidingItems = scene()->items();
        for (QGraphicsItem* item_ : collidingItems) {
            Manipulator* item;
            if (item_->isVisible() && (item_ != this) && (item = dynamic_cast<Manipulator*>(item_))) {
                if (item->isAboveItem(this))
                    clipPath = clipPath.subtracted(item->boundingClipPath().translated(item->scenePos() - scenePos()));
            }
        }

        // we clip using stencil buffers to prevent overlapping outlines appearing
        // FIXME: This could potentially get very slow for huge layouts
        painter->setClipPath(clipPath);
    }

    impl_paint(painter, option, widget);

    painter->restore();
}

void Manipulator::impl_paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    super::paint(painter, option, widget);

    if (isSelected() || m_resizeInProgress || isAnyHandleSelected()) {
        CEGUI::Sizef baseSize = getBaseSize();
        paintHorizontalGuides(baseSize, painter, option, widget);
        paintVerticalGuides(baseSize, painter, option, widget);
    }
}

void Manipulator::triggerPropertyManagerCallback(const QSet<QString> &propertyNames)
{
#if 0
    CEGUI::Window* widget = m_widget;

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

bool Manipulator::impl_hasNonAutoWidgetDescendants(CEGUI::Window *widget)
{
    if (!widget->isAutoWindow())
        return true;

    for (int i = 0; i < widget->getChildCount(); i++) {
        CEGUI::Window* child = widget->getChildAtIdx(i);

        if (impl_hasNonAutoWidgetDescendants(child))
            return true;
    }

    return false;
}

/////

SerialisationData::SerialisationData(CEGUI::Window *widget, bool serialiseChildren)
{
    m_name = "";
    m_type = "";
    // full parent path at the time of serialisation
    m_parentPath = "";
    // if true, the widget was an auto-widget, therefore it will not be
    // created directly when reconstructing but instead just retrieved
    m_autoWidget = false;
    //        m_properties = {}
    //        m_children = []

    if (widget != nullptr) {
        m_name = widget->getName();
        m_type = widget->getType();
        m_autoWidget = widget->isAutoWindow();

        CEGUI::Window* parent = widget->getParent();
        if (parent != nullptr)
            m_parentPath = parent->getNamePath();

        serialiseProperties(widget);

#if 0 // can't do this here (virtual methods)
        if (serialiseChildren)
            this->serialiseChildren(widget);
#endif
    }
}

void SerialisationData::setParentPath(const QString &parentPath)
{
    m_parentPath = FROM_QSTR(parentPath);

    for (SerialisationData* child : m_children) {
        child->setParentPath(TO_QSTR(m_parentPath + "/" + m_name));
    }
}

SerialisationData *SerialisationData::createChildData(CEGUI::Window *widget, bool serialiseChildren)
{
    return new SerialisationData(widget, serialiseChildren);
}

Manipulator *SerialisationData::createManipulator(Manipulator *parentManipulator, CEGUI::Window *widget, bool recursive, bool skipAutoWidgets)
{
    auto* ret = new Manipulator(parentManipulator, widget, recursive, skipAutoWidgets);
    if (recursive)
        ret->createChildManipulators(true, skipAutoWidgets);
    ret->updateFromWidget();
    return ret;
}

void SerialisationData::serialiseProperties(CEGUI::Window *widget)
{
    auto it = widget->getPropertyIterator();
    while (!it.isAtEnd()) {
        CEGUI::String propertyName = it.getCurrentKey();

        if (!widget->isPropertyBannedFromXML(propertyName) && !widget->isPropertyDefault(propertyName)) {
            m_properties[propertyName] = widget->getProperty(propertyName);
        }

        it++;
    }
}

void SerialisationData::serialiseChildren(CEGUI::Window *widget, bool skipAutoWidgets)
{
    int i = 0;

    while (i < widget->getChildCount()) {
        auto* child = widget->getChildAtIdx(i);
        if (!skipAutoWidgets || !child->isAutoWindow()) {
            m_children += createChildData(child, true);
        }
        i += 1;
    }
}

Manipulator *SerialisationData::reconstruct(Manipulator *rootManipulator)
{
    CEGUI::Window* widget = nullptr;
    Manipulator* ret = nullptr;

    if (rootManipulator == nullptr) {
        if (m_autoWidget)
            throw RuntimeError("Root widget can't be an auto widget!");

        widget = CEGUI::WindowManager::getSingleton().createWindow(m_type, m_name);
        ret = createManipulator(nullptr, widget);
        rootManipulator = ret;

    } else {
        Manipulator* parentManipulator = nullptr;
        QString parentPathSplit0 = TO_QSTR(m_parentPath).section('/', 0, 0);
        QString parentPathSplit1 = TO_QSTR(m_parentPath).section('/', 1);
        Q_ASSERT(!parentPathSplit0.isEmpty());

        if (parentPathSplit1.isEmpty())
            parentManipulator = rootManipulator;
        else
            parentManipulator = rootManipulator->getManipulatorByPath(parentPathSplit1);

        if (m_autoWidget) {
            widget = parentManipulator->m_widget->getChild(m_name);
            if (widget->getType() != m_type)
                throw RuntimeError("Skipping widget construction because it's an auto widget, the types don't match though!");

        } else {
            widget = CEGUI::WindowManager::getSingleton().createWindow(m_type, m_name);
            parentManipulator->m_widget->addChild(widget);
        }

        // Extra code because of auto windows
        // NOTE: We don't have to do rootManipulator.createMissingChildManipulators
        //       because auto windows never get created outside their parent
        parentManipulator->createMissingChildManipulators(true, false);

        QString realPathSplit1 = TO_QSTR(widget->getNamePath()).section('/', 1);
        ret = rootManipulator->getManipulatorByPath(realPathSplit1);
    }

    for (auto it = m_properties.begin(); it != m_properties.end(); it++) {
        CEGUI::String name = it.key();
        CEGUI::String value = it.value();
        widget->setProperty(name, value);
    }

    for (SerialisationData* child : m_children) {
        CEGUI::Window* childWidget = child->reconstruct(rootManipulator)->m_widget;
        if (!child->m_autoWidget) {
            widget->addChild(childWidget);
        }
    }

    // refresh the manipulator using newly set properties
    ret->updateFromWidget(false, true);
    return ret;
}


} // namespace widgethelpers
} // namespace cegui
} // namespace CEED
