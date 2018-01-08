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

#ifndef CEED_editors_imageset_elements_
#define CEED_editors_imageset_elements_

#include "CEEDBase.h"

#include "resizable.h"

#include "elementtree.h"

#include <QGraphicsTextItem>

class QFileSystemWatcher;
class QListWidgetItem;

namespace CEED {
namespace editors {
namespace imageset {

namespace visual {
class VisualEditing;
}

namespace elements {

class ImageEntry;
class ImagesetEntry;

/*!
\brief ImageLabel

Text item showing image's label when the image is hovered or selected.
    You should not use this directly! Use ImageEntry.name instead to get the name.

*/
class ImageLabel : public QGraphicsTextItem
{
public:
    ImageEntry* m_imageEntry;

    ImageLabel(ImageEntry* imageEntry);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        QGraphicsTextItem::hoverEnterEvent(event);

        setOpacity(0.2);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override
    {
        setOpacity(0.8);

        QGraphicsTextItem::hoverLeaveEvent(event);
    }
};

/*!
\brief ImageOffset

A crosshair showing where the imaginary (0, 0) point of the image is. The actual offset
    is just a negated vector of the crosshair's position but this is easier to work with from
    the artist's point of view.

*/
class ImageOffset : public QGraphicsPixmapItem
{
public:
    ImageEntry* m_imageEntry;
    bool m_isHovered;
    bool m_potentialMove;
    optional<QPointF> m_oldPosition;

    ImageOffset(ImageEntry* imageEntry);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override
    {
        QGraphicsPixmapItem::hoverEnterEvent(event);

        m_isHovered = true;
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override
    {
        m_isHovered = false;

        QGraphicsPixmapItem::hoverLeaveEvent(event);
    }
};

/*!
\brief ImageEntry

Represents the image of the imageset, can be drag moved, selected, resized, ...

*/
class ImageEntry : public resizable::ResizableRectItem
{
    typedef resizable::ResizableRectItem super;

public:
    int m_nativeHorzRes;
    int m_nativeVertRes;
    QString m_autoScaled;
    ImagesetEntry* m_imagesetEntry;
    bool m_isHovered;
    bool m_potentialMove;
    optional<QPointF> m_oldPosition;
    bool m_resized;
    ImageLabel* m_label;
    ImageOffset* m_offset;
    visual::ImagesetEditorItem* m_listItem;

    // the image's "real parameters" are properties that directly access Qt's
    // facilities, this is done to make the code cleaner and save a little memory

    QString name() { return m_label->toPlainText(); }
    void set_name(const QString&value) { m_label->setPlainText(value); }

    int xpos() { return int(pos().x()); }
    void set_xpos(int value) { setPos(value, pos().y()); }

    int ypos() { return int(pos().y()); }
    void set_ypos(int value) { setPos(pos().x(), value); }

    int width() { return int(rect().width()); }
    void set_width(int value) { setRect(QRectF(0, 0, qMax(1, value), height())); }

    int height() { return int(rect().height()); }
    void set_height(int value) { setRect(QRectF(0, 0, width(), qMax(1, value))); }

    int xoffset() { return int(-(m_offset->pos().x() - 0.5)); }
    void set_xoffset(int value) { m_offset->setX(-float(value) + 0.5); }

    int yoffset() {return int(-(m_offset->pos().y() - 0.5)); }
    void set_yoffset(int value) { m_offset->setY(-float(value) + 0.5); }

    QPoint nativeRes() { return { m_nativeHorzRes, m_nativeVertRes }; }
    void set_nativeRes(const QPoint& value) { m_nativeHorzRes = value.x(); m_nativeVertRes = value.y(); }

     // Added for C++ version to replace getattr()/setattr()
    QVariant getPropertyValue(const QString& propertyName);
    void setPropertyValue(const QString& propertyName, const QVariant& newValue);

    ImageEntry(ImagesetEntry* imagesetEntry);

    QRectF constrainResizeRect(const QRectF& rect_, const QRectF& oldRect) override
    {
        // we simply round the rectangle because we only support "full" pixels

        // NOTE: Imageset as such might support floating point pixels but it's never what you
        //       really want, image quality deteriorates a lot

#if 1
        QRectF rect = rect_.toAlignedRect();
#else
        QRectF rect = QRectF(QPointF(round(rect_.topLeft().x()), round(rect_.topLeft().y())),
                             QPointF(round(rect_.bottomRight().x()), round(rect_.bottomRight().y())));
#endif
        return super::constrainResizeRect(rect, oldRect);
    }

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    /**Creates and returns a pixmap containing what's in the underlying image in the rectangle
    that this ImageEntry has set.

    This is mostly used for preview thumbnails in the dock widget.
    */
    QPixmap getPixmap();

    /**Updates the list item associated with this image entry in the dock widget
    */
    void updateListItem();

    void updateListItemSelection();

    /**If we are selected in the dock widget, this updates the property box
    */
    void updateDockWidget();

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void notifyResizeStarted();

    void notifyResizeFinished(const QPointF& newPos, const QRectF& newRect);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
};

/*!
\brief ImagesetEntry

This is the whole imageset containing all the images (ImageEntries).

    The main reason for this is not to have multiple imagesets editing at once but rather
    to have the transparency background working properly.

*/
class ImagesetEntry : public QGraphicsPixmapItem
{
public:
    QString m_name;
    QString m_imageFile;
    int m_nativeHorzRes;
    int m_nativeVertRes;
    QString m_autoScaled;
    QList<ImageEntry*> m_imageEntries;
    imageset::visual::FakeVisual/*VisualEditing*/* m_visual;
    bool m_showOffsets;
    QGraphicsRectItem* m_transparencyBackground;
    QFileSystemWatcher* m_imageMonitor;
    bool m_displayingReloadAlert;

    ImagesetEntry(imageset::visual::FakeVisual/*VisualEditing*/* visual_);

    ImageEntry* getImageEntry(const QString& name);

    void slot_imageChangedByExternalProgram();

    /**
    Replaces the underlying image (if any is loaded) to the image on given relative path

    Relative path is relative to the directory where the .imageset file resides
    (which is usually your project's imageset resource group path)
    */
    void loadImage(const QString& relativeImagePath);

    /**Returns an absolute (OS specific!) path of the underlying image
    */
    QString getAbsoluteImageFile();

    /**Converts given absolute underlying image path to relative path (relative to the directory where
    the .imageset file resides
    */
    QString convertToRelativeImageFile(const QString& absoluteImageFile);

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element *saveToElement();
};

} // namespace elements
} // namespace imageset
} // namespace editors
} // namespace CEED

// needs to be at the end, import to get the singleton
//from ceed import mainwindow
//from ceed import settings

#endif
