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

#ifndef CEED_cegui_widgethelpers_
#define CEED_cegui_widgethelpers_

#include "CEEDBase.h"

#include "resizable.h"
#include "cegui_qtgraphics.h"

#include <functional>

// This module contains helping classes for CEGUI widget handling

namespace CEED {
namespace cegui {
namespace widgethelpers {

/*!
\brief GraphicsScene

If you plan to use widget manipulators, use a scene inherited from this class.

*/
class GraphicsScene : public qtgraphics::GraphicsScene
{
public:
    GraphicsScene(Instance* ceguiInstance)
        : qtgraphics::GraphicsScene(ceguiInstance)
    {
    }
};

/*!
\brief Manipulator

This is a rectangle that is synchronised with given CEGUI widget,
    it provides moving and resizing functionality

*/
class Manipulator : public resizable::ResizableRectItem
{
    typedef resizable::ResizableRectItem super;
public:
    CEGUI::Window* m_widget;
    optional<CEGUI::UVector2> m_preResizePos;
    optional<CEGUI::USize> m_preResizeSize;
    QPointF m_lastResizeNewPos;
    QRectF m_lastResizeNewRect;
    optional<CEGUI::UVector2> m_preMovePos;
    QPointF m_lastMoveNewPos;

    /**Constructor

    widget - CEGUI::Widget to wrap
    recursive - if true, even children of given widget are wrapped
    skipAutoWidgets - if true, auto widgets are skipped (only applicable if recursive is true)
    */
    Manipulator(QGraphicsItem* parent, CEGUI::Window* widget, bool recursive = true, bool skipAutoWidgets = false);

    QString getWidgetPath()
    {
        return m_widget ? TO_QSTR(m_widget->getNamePath()) : "<Unknown>";
    }

    /**Creates a child manipulator suitable for a child widget of manipulated widget

    recursive - recurse into children?
    skipAutoWidgets - if true, auto widgets will be skipped over

    This is there to allow overriding (if user subclasses the Manipulator, child manipulators are
    likely to be also subclassed
    */
    virtual Manipulator* createChildManipulator(CEGUI::Window* childWidget, bool recursive = true, bool skipAutoWidgets = false);

    /**Returns function pointers for the child getter and the child count functions. This is
    necessary since some windows, such as TabControl and ScrollablePane, use AutoWindows
    as containers, but we want to attach the children directly to them and display them as such,
    effectively hiding the auto windows.

    Returns a tuple of a child-count getter and a children-by-index getter
    */
    void getFunctionsChildCountAndChildGet(std::function<size_t()>& countGetter, std::function<CEGUI::Window*(size_t)> &childGetter);

    /**Creates manipulators for child widgets of widget manipulated by this manipulator
    Special handling for widgets using children AutoWindows that act as  containers, such
    as TabControl and ScrollablePane is done.

    recursive - recurse into children?
    skipAutoWidgets - if true, auto widgets will be skipped over
    */
    void createChildManipulators(bool recursive = true, bool skipAutoWidgets = false);

    /**Goes through child widgets of the manipulated widget and creates manipulator
    for each missing one.

    recursive - recurse into children?
    skipAutoWidgets - if true, auto widgets will be skipped over
    */
    void createMissingChildManipulators(bool recursive = true, bool skipAutoWidgets = false);

    /**Detaches itself from the GUI hierarchy and the manipulator hierarchy.

    detachWidget - should we detach the CEGUI widget as well?
    destroyWidget - should we destroy the CEGUI widget after it's detached?
    recursive - recurse into children?

    This method doesn't destroy this instance immediately but it will be destroyed automatically
    when nothing is referencing it.
    */
    virtual void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true);

    /**Detaches all child manipulators

    detachWidget - should we detach the CEGUI widget as well?
    destroyWidget - should we destroy the CEGUI widget after it's detached?
    recursive - recurse into children?
    */
    void detachChildManipulators(bool detachWidget = true, bool destroyWidget = true, bool recursive = true);

    /**Retrieves a manipulator relative to this manipulator by given widget path

    Throws LookupError on failure.
    */
    Manipulator* getManipulatorByPath(const QString& widgetPath);

    /**Retrieves a manipulator relative to this manipulator by given widget path
    for widget's that use autoWindow containers, such as ScrollablePanes and TabControl.
    The children in these case should be treated as if they were attached to the window
    directly, whereas in reality they use a container widget, which forces us to handle
    these cases using this function
    */
    Manipulator* getManipulatorFromChildContainerByPath(const QString& widgetPath);

    QList<Manipulator*> getChildManipulators();

    QList<Manipulator*> getAllDescendantManipulators();

    /**Updates this manipulator with associated widget properties. Mainly
    position and size.

    callUpdate - if true we also call update on the widget itself before
                 querying its properties
    updateParentLCs - if true we update ancestor layout containers
    */
    virtual void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false);

    void moveToFront();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void notifyHandleSelected(resizable::ResizingHandle* handle) override;

    QSizeF getMinSize();

    QSizeF getMaxSize();

    CEGUI::Sizef getBaseSize();

    virtual bool useAbsoluteCoordsForMove()
    {
        return false;
    }

    virtual bool useAbsoluteCoordsForResize()
    {
        return false;
    }

    virtual bool useIntegersForAbsoluteMove()
    {
        return false;
    }

    virtual bool useIntegersForAbsoluteResize()
    {
        return false;
    }

    void notifyResizeStarted() override;

    void notifyResizeProgress(const QPointF &newPos, const QRectF &newRect) override;

    void notifyResizeFinished(const QPointF &newPos, const QRectF &newRect) override;

    void notifyMoveStarted() override;

    void notifyMoveProgress(const QPointF &newPos) override;

    void notifyMoveFinished(const QPointF &newPos) override;

    /**Retrieves clip path containing the bounding rectangle*/
    QPainterPath boundingClipPath();

    bool isAboveItem(QGraphicsItem* item);

    /**Paints horizontal dimension guides - position X and width guides*/
    void paintHorizontalGuides(CEGUI::Sizef baseSize, QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**Paints vertical dimension guides - position Y and height guides*/
    void paintVerticalGuides(CEGUI::Sizef baseSize, QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**Returns whether the painting code should strive to prevent manipulator overlap (crossing outlines and possibly other things)
    Override to change the behavior
    */
    virtual bool getPreventManipulatorOverlap()
    {
        return false;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    virtual void impl_paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /**Notify the property manager that the values of the given
    properties have changed for this widget.
    */
    virtual void triggerPropertyManagerCallback(const QSet<QString>& propertyNames);

    /**Checks whether there are non-auto widgets nested in this widget

    Self is a descendant of self in this context!
    */
    bool hasNonAutoWidgetDescendants()
    {
        return impl_hasNonAutoWidgetDescendants(m_widget);
    }

    bool impl_hasNonAutoWidgetDescendants(CEGUI::Window* widget);
};

/*!
\brief SerialisationData

Allows to "freeze" CEGUI widget to data that is easy to retain in python,
    this is a helper class that can be used for copy/paste, undo commands, etc...

*/
class SerialisationData
{
public:
    CEGUI::String m_name;
    CEGUI::String m_type;
    CEGUI::String m_parentPath;
    bool m_autoWidget;
    QMap<CEGUI::String, CEGUI::String> m_properties;
    QList<SerialisationData*> m_children;

    SerialisationData(CEGUI::Window* widget = nullptr, bool serialiseChildren = true);

    /**Recursively changes the parent path
    */
    void setParentPath(const QString& parentPath);

    /**Creates child serialisation data of this widget
    */
    virtual SerialisationData* createChildData(CEGUI::Window* widget, bool serialiseChildren = true);

    /**Creates a manipulator suitable for the widget resulting from reconstruction
    */
    virtual Manipulator* createManipulator(Manipulator* parentManipulator, CEGUI::Window* widget, bool recursive = true, bool skipAutoWidgets = false);

    /**Takes a snapshot of all properties of given widget and stores them
    in a string form
    */
    void serialiseProperties(CEGUI::Window* widget);

    /**Serialises all child widgets of given widgets

    skipAutoWidgets - should auto widgets be skipped over?
    */
    void serialiseChildren(CEGUI::Window* widget, bool skipAutoWidgets = false);

    /**Reconstructs widget serialised in this SerialisationData.

    rootManipulator - manipulator at the root of the target tree
    */
    Manipulator* reconstruct(Manipulator* rootManipulator);
};

} // namespace widgethelpers
} // namespace cegui
} // namespace CEED

#endif
