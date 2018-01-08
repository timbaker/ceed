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

#ifndef CEED_editors_layout_widgethelpers_
#define CEED_editors_layout_widgethelpers_

#include "CEEDBase.h"

#include "action/declaration.h"

#include "editors/layout/editor_layout_undo.h"
#include "editors/layout/editor_layout_visual.h"

#include "cegui/cegui_widgethelpers.h"

#include <QGraphicsSceneEvent>

namespace CEED {
namespace editors {
namespace layout {
namespace widgethelpers {

/*!
\brief Manipulator

Layout editing specific widget manipulator
*/
class Manipulator : public cegui::widgethelpers::Manipulator
{
    typedef cegui::widgethelpers::Manipulator super;
public:
    static QBrush snapGridBrush;
    static int snapGridX, snapGridY;
    static QColor snapGridPointColour;
    static QColor snapGridPointShadowColour;

    /**Retrieves a (cached) snap grid brush
        */
    static QBrush getSnapGridBrush();

    /**Returns a valid CEGUI widget name out of the supplied name, if possible.
    Returns None if the supplied name is invalid and can't be converted to a valid name
    (an empty string for example).
    */
    static QString getValidWidgetName(const QString& name_)
    {
        if (name_.isEmpty())
            return "";
        QString name = name_.trimmed();
        if (name.isEmpty())
            return "";
        return name.replace("/", "_");
    }

    visual::VisualEditing* m_visual;
    bool m_showOutline;
    bool m_drawSnapGrid;
    bool m_snapGridNonClientArea;
    bool m_ignoreSnapGrid;
    using Action = action::declaration::Action;
    Action* m_snapGridAction;
    Action* m_absoluteModeAction;
    Action* m_absoluteIntegersOnlyModeAction;
    visual::WidgetHierarchyItem* m_treeItem;
    QMetaObject::Connection m_absoluteModeActionConnection;

    Manipulator(visual::VisualEditing* visual, QGraphicsItem* parent, CEGUI::Window* widget, bool recursive = true, bool skipAutoWidgets = false);

    ~Manipulator();

    void slot_absoluteModeToggled(bool checked);

    QPen getNormalPen() override;

    QPen getHoverPen() override;

    QPen getPenWhileResizing() override;

    QPen getPenWhileMoving() override;

    QPen getDragAcceptableHintPen();

    /**Finds a unique name for a child widget of the manipulated widget

        The resulting name's format is the base with a number appended
        */
    QString getUniqueChildWidgetName(const QString& base = "Widget");

    Manipulator* createChildManipulator(CEGUI::Window* childWidget, bool recursive = true, bool skipAutoWidgets = false);

    void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;

    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;

    /**Takes care of creating new widgets when user drops the right mime type here
    (dragging from the CreateWidgetDockWidget)
    */
    void dropEvent(QGraphicsSceneDragDropEvent *event) override;

    bool useAbsoluteCoordsForMove()
    {
        return m_absoluteModeAction->isChecked();
    }

    bool useAbsoluteCoordsForResize()
    {
        return m_absoluteModeAction->isChecked();
    }

    bool useIntegersForAbsoluteMove()
    {
        return m_absoluteIntegersOnlyModeAction->isChecked();
    }

    bool useIntegersForAbsoluteResize()
    {
        return m_absoluteIntegersOnlyModeAction->isChecked();
    }

    void notifyResizeStarted() override
    {
        super::notifyResizeStarted();

        QGraphicsItem* parent_ = parentItem();
        if (Manipulator* parent = dynamic_cast<Manipulator*>(parent_)) {
            parent->m_drawSnapGrid = true;
        }
    }

    void notifyResizeProgress(const QPointF &newPos, const QRectF &newRect) override
    {
        super::notifyResizeProgress(newPos, newRect);

        triggerPropertyManagerCallback({"Size", "Position", "Area"});
    }

    void notifyResizeFinished(const QPointF &newPos, const QRectF &newRect) override
    {
        super::notifyResizeFinished(newPos, newRect);

        QGraphicsItem* parent_ = parentItem();
        if (Manipulator* parent = dynamic_cast<Manipulator*>(parent_)) {
            parent->m_drawSnapGrid = false;
        }
    }

    void notifyMoveStarted() override
    {
        super::notifyMoveStarted();

        QGraphicsItem* parent_ = parentItem();
        if (Manipulator* parent = dynamic_cast<Manipulator*>(parent_)) {
            parent->m_drawSnapGrid = true;
        }
    }

    void notifyMoveProgress(const QPointF &newPos) override
    {
        super::notifyMoveProgress(newPos);

        triggerPropertyManagerCallback({"Position", "Area"});
    }

    void notifyMoveFinished(const QPointF &newPos) override
    {
        super::notifyMoveFinished(newPos);

        QGraphicsItem* parent_ = parentItem();
        if (Manipulator* parent = dynamic_cast<Manipulator*>(parent_)) {
            parent->m_drawSnapGrid = false;
        }
    }

    /**Returns whether the painting code should strive to prevent manipulator overlap (crossing outlines and possibly other things)
    Override to change the behavior
    */
    bool getPreventManipulatorOverlap() override
    {
        return settings::getEntry("layout/visual/prevent_manipulator_overlap")->m_value.toBool();
    }

    void impl_paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void triggerPropertyManagerCallback(const QSet<QString> &propertyNames) override;

    void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false) override;

    qreal snapXCoordToGrid(qreal x);

    qreal snapYCoordToGrid(qreal y);

    QPointF constrainMovePoint(const QPointF& point_) override;

    QRectF constrainResizeRect(const QRectF &rect_, const QRectF &oldRect) override;

    void setLocked(bool locked)
    {
        setFlag(QGraphicsItem::ItemIsMovable, !locked);
        setFlag(QGraphicsItem::ItemIsSelectable, !locked);
        setFlag(QGraphicsItem::ItemIsFocusable, !locked);

        setResizingEnabled(!locked);

#if 1 // C++
        setVisible(!locked);
#endif

        update();
    }
};

/*!
\brief SerialisationData

See cegui.widgethelpers.SerialisationData

    The only reason for this class is that we need to create the correct Manipulator (not it's base class!)

*/
class SerialisationData : public cegui::widgethelpers::SerialisationData
{
public:
    visual::VisualEditing* m_visual;

    SerialisationData(visual::VisualEditing* visual, CEGUI::Window* widget = nullptr, bool serialiseChildren = true);

    SerialisationData* createChildData(CEGUI::Window* widget = nullptr, bool serialiseChildren = true) override;

    cegui::widgethelpers::Manipulator* createManipulator(cegui::widgethelpers::Manipulator* parentManipulator, CEGUI::Window* widget, bool recursive = true, bool skipAutoWidgets = true) override;

    void setVisual(visual::VisualEditing* visual);

    Manipulator* reconstruct(Manipulator* rootManipulator);
};

//from ceed import settings
//from ceed import action

} // namespae widgethelpers
} // namespace layout
} // namespace editors
} // namespace CEED

#endif
