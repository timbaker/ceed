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

#include "editor_imageset_elements.h"

#include "mainwindow.h"
#include "qtwidgets.h"
#include "editors/imageset/editor_imageset_visual.h"

#include "ceed_paths.h"

#include <QApplication>
#include <QFileSystemWatcher>
#include <QListWidgetItem>
#include <QStatusBar>

namespace CEED {
namespace editors {
namespace imageset {
namespace elements {

ImageLabel::ImageLabel(ImageEntry *imageEntry)
    : QGraphicsTextItem(imageEntry)
{
    m_imageEntry = imageEntry;

    setFlags(QGraphicsItem::ItemIgnoresTransformations);
    setOpacity(0.8);

    setPlainText("Unknown");

    // we make the label a lot more transparent when mouse is over it to make it easier
    // to work around the top edge of the image
    setAcceptHoverEvents(true);
    // the default opacity (when mouse is not over the label)
    setOpacity(0.8);

    // be invisible by default and wait for hover/selection events
    setVisible(false);
}

void ImageLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPalette palette = QApplication::palette();

    painter->fillRect(boundingRect(), palette.color(QPalette::Normal, QPalette::Base));
    painter->drawRect(boundingRect());

    QGraphicsTextItem::paint(painter, option, widget);
}

/////

ImageOffset::ImageOffset(ImageEntry *imageEntry)
    : QGraphicsPixmapItem(imageEntry)
{
    m_imageEntry = imageEntry;

    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIgnoresTransformations |
             QGraphicsItem::ItemSendsGeometryChanges);

    setCursor(Qt::OpenHandCursor);

    setPixmap(QPixmap(":icons/imageset_editing/offset_crosshair.png"));
    // the crosshair pixmap is 15x15, (7, 7) is the centre pixel of it,
    // we want that to be the (0, 0) point of the crosshair
    setOffset(-7, -7);
    // always show this above the label (which has ZValue = 0)
    setZValue(1);

    setAcceptHoverEvents(true);
    // internal attribute to help decide when to hide/show the offset crosshair
    m_isHovered = false;

    // used for undo
    m_potentialMove = false;
    m_oldPosition = QPointF();

    // by default Qt considers parts of the image with alpha = 0 not part of the image,
    // that would make it very hard to move the crosshair, we consider the whole
    // bounding rectangle to be part of the image
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setVisible(false);
}

QVariant ImageOffset::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        if (m_potentialMove && !m_oldPosition)
            m_oldPosition = pos();

        QPointF newPosition = value.toPointF();

        // now round the position to pixels
        newPosition.setX(round(newPosition.x() - 0.5) + 0.5);
        newPosition.setY(round(newPosition.y() - 0.5) + 0.5);

        return newPosition;
    }

    else if (change == QGraphicsItem::ItemSelectedChange) {
        if (!value.toBool()) {
            if (!m_imageEntry->isSelected())
                setVisible(false);
        } else
            setVisible(true);
    }

    return QGraphicsPixmapItem::itemChange(change, value);
}

/////

QVariant ImageEntry::getPropertyValue(const QString &propertyName)
{
    if (propertyName == "xpos") return xpos();
    if (propertyName == "ypos") return ypos();
    if (propertyName == "width") return width();
    if (propertyName == "height") return height();
    if (propertyName == "xoffset") return xoffset();
    if (propertyName == "yoffset") return yoffset();
    if (propertyName == "autoScaled") return m_autoScaled;
    Q_ASSERT(false);
    return QVariant();
}

void ImageEntry::setPropertyValue(const QString &propertyName, const QVariant &newValue)
{
    if (propertyName == "xpos") { set_xpos(newValue.toInt()); return; }
    if (propertyName == "ypos") { set_ypos(newValue.toInt()); return; }
    if (propertyName == "width") { set_width(newValue.toInt()); return; }
    if (propertyName == "height") { set_height(newValue.toInt()); return; }
    if (propertyName == "xoffset") { set_xoffset(newValue.toInt()); return; }
    if (propertyName == "yoffset") { set_yoffset(newValue.toInt()); return; }
    if (propertyName == "autoScaled") { m_autoScaled = newValue.toString(); return; }
    Q_ASSERT(false);
}

ImageEntry::ImageEntry(ImagesetEntry *imagesetEntry)
    : super(imagesetEntry)
{
    m_nativeHorzRes = 0;
    m_nativeVertRes = 0;
    m_autoScaled = "";

    m_imagesetEntry = imagesetEntry;

    setAcceptHoverEvents(true);
    m_isHovered = false;

    // used for undo
    m_potentialMove = false;
    m_oldPosition.reset();
    m_resized = false;

    setFlags(QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setVisible(true);

    m_label = new ImageLabel(this);
    m_offset = new ImageOffset(this);

    // list item in the dock widget's ListWidget
    // this allows fast updates of the list item without looking it up
    // It is safe to assume that this is None or a valid QListWidgetItem
    m_listItem = nullptr;
}

void ImageEntry::loadFromElement(ElementTree::Element *element)
{
    set_name( element->get("name", "Unknown") );

    set_xpos( element->getInt("xPos", 0) );
    set_ypos( element->getInt("yPos", 0) );
    set_width( element->getInt("width", 1) );
    set_height( element->getInt("height", 1) );

    set_xoffset( element->getInt("xOffset", 0) );
    set_yoffset( element->getInt("yOffset", 0) );

    m_nativeHorzRes = element->getInt("nativeHorzRes", 0);
    m_nativeVertRes = element->getInt("nativeVertRes", 0);
    m_autoScaled = element->get("autoScaled", "");
}

ElementTree::Element *ImageEntry::saveToElement()
{
    auto ret = new ElementTree::Element("Image");

    ret->set("name", name());

    ret->set("xPos", xpos());
    ret->set("yPos", ypos());
    ret->set("width", width());
    ret->set("height", height());

    // we write none or both
    if (xoffset() != 0 || yoffset() != 0) {
        ret->set("xOffset", xoffset());
        ret->set("yOffset", yoffset());
    }

    if (m_nativeHorzRes != 0)
        ret->set("nativeHorzRes", m_nativeHorzRes);

    if (m_nativeVertRes != 0)
        ret->set("nativeVertRes", m_nativeVertRes);

    if (m_autoScaled != "")
        ret->set("autoScaled", m_autoScaled);

    return ret;
}

QPixmap ImageEntry::getPixmap()
{
    return m_imagesetEntry->pixmap().copy(int(pos().x()), int(pos().y()),
                                          int(rect().width()), int(rect().height()));
}

void ImageEntry::updateListItem()
{
    if (m_listItem == nullptr)
        return;

    m_listItem->setText(name());

    const int previewWidth = 24;
    const int previewHeight = 24;

    QPixmap preview(previewWidth, previewHeight);
    QPainter painter(&preview);
    painter.setBrush(qtwidgets::getCheckerboardBrush());
    painter.drawRect(0, 0, previewWidth, previewHeight);
    QPixmap scaledPixmap = getPixmap().scaled(QSize(previewWidth, previewHeight),
                                      Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap((previewWidth - scaledPixmap.width()) / 2,
                       (previewHeight - scaledPixmap.height()) / 2,
                       scaledPixmap);
    painter.end();

    m_listItem->setIcon(QIcon(preview));
}

void ImageEntry::updateListItemSelection()
{
    /**Synchronises the selection in the dock widget's list. This makes sure that when you select
        this item the list sets the selection to this item as well.
        */

    if (m_listItem == nullptr)
        return;

    visual::ImagesetEditorDockWidget* dockWidget = m_listItem->m_dockWidget;

    // the dock widget itself is performing a selection, we shall not interfere
    if (dockWidget->m_selectionUnderway)
        return;

    dockWidget->m_selectionSynchronisationUnderway = true;

    if (isSelected() || isAnyHandleSelected() || m_offset->isSelected())
        m_listItem->setSelected(true);
    else
        m_listItem->setSelected(false);

    dockWidget->m_selectionSynchronisationUnderway = false;
}

void ImageEntry::updateDockWidget()
{
    updateListItem();

    if (m_listItem == nullptr)
        return;

    auto dockWidget = m_listItem->m_dockWidget;
    if (dockWidget->m_activeImageEntry == this)
        dockWidget->refreshActiveImageEntry();
}

QVariant ImageEntry::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged) {
        if (value.toBool()) {
            if (settings::getEntry("imageset/visual/overlay_image_labels")->m_value.toBool())
                m_label->setVisible(true);

            if (m_imagesetEntry->m_showOffsets)
                m_offset->setVisible(true);

            setZValue(zValue() + 1);
        } else {
            if (!m_isHovered)
                m_label->setVisible(false);

            if (!m_offset->isSelected() && !m_offset->m_isHovered)
                m_offset->setVisible(false);

            setZValue(zValue() - 1);
        }
        updateListItemSelection();

    } else if (change == QGraphicsItem::ItemPositionChange) {
        if (m_potentialMove && !m_oldPosition) {
            m_oldPosition = pos();
            // hide label when moving so user can see edges clearly
            m_label->setVisible(false);
        }
        QPointF newPosition = value.toPointF();

        // if, for whatever reason, the loading of the pixmap failed,
        // we don't constrain to the empty null pixmap

        // only constrain when the pixmap is valid
        if (!m_imagesetEntry->pixmap().isNull()) {

            QRectF rect = m_imagesetEntry->boundingRect();
            rect.setWidth(rect.width() - this->rect().width());
            rect.setHeight(rect.height() - this->rect().height());

            if (!rect.contains(newPosition)) {
                newPosition.setX(qMin(rect.right(), qMax(newPosition.x(), rect.left())));
                newPosition.setY(qMin(rect.bottom(), qMax(newPosition.y(), rect.top())));
            }
        }

        // now round the position to pixels
        newPosition.setX(round(newPosition.x()));
        newPosition.setY(round(newPosition.y()));

        return newPosition;
    }

    return super::itemChange(change, value);
}

void ImageEntry::notifyResizeStarted()
{
    super::notifyResizeStarted();

    // hide label when resizing so user can see edges clearly
    m_label->setVisible(false);
}

void ImageEntry::notifyResizeFinished(const QPointF &newPos, const QRectF &newRect)
{
    super::notifyResizeFinished(newPos, newRect);

    if (m_mouseOver && settings::getEntry("imageset/visual/overlay_image_labels")->m_value.toBool()) {
        // if mouse is over we show the label again when resizing finishes
        m_label->setVisible(true);
    }

    // mark as resized so we can pick it up in VisualEditing.mouseReleaseEvent
    m_resized = true;
}

void ImageEntry::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    super::hoverEnterEvent(event);

    setZValue(zValue() + 1);

    if (settings::getEntry("imageset/visual/overlay_image_labels")->m_value.toBool())
        m_label->setVisible(true);

    mainwindow::MainWindow::instance->statusBar()->showMessage(QString("Image: '%1'\t\tXPos: %2, YPos: %3, Width: %4, Height: %5")
                                                               .arg(name()).arg(pos().x()).arg(pos().y()).arg(rect().width()).arg(rect().height()));

    m_isHovered = true;
}

void ImageEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    mainwindow::MainWindow::instance->statusBar()->clearMessage();

    m_isHovered = false;

    if (!isSelected())
        m_label->setVisible(false);

    setZValue(zValue() - 1);

    super::hoverLeaveEvent(event);
}

void ImageEntry::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    super::paint(painter, option, widget);

    // to be more visible, we draw yellow rect over the usual dashed double colour rect
    if (isSelected()) {
        painter->setPen(QColor(255, 255, 0, 255));
        painter->drawRect(rect());
    }
}

/////

ImagesetEntry::ImagesetEntry(imageset::visual::FakeVisual *visual_)
    : QGraphicsPixmapItem()
{
    m_name = "Unknown";
    m_imageFile = "";
    m_nativeHorzRes = 800;
    m_nativeVertRes = 600;
    m_autoScaled = "false";

    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setCursor(Qt::ArrowCursor);

    m_visual = visual_;
    //        m_imageEntries = []

    m_showOffsets = false;

    m_transparencyBackground = new QGraphicsRectItem();
    m_transparencyBackground->setParentItem(this);
    m_transparencyBackground->setFlags(QGraphicsItem::ItemStacksBehindParent);

    m_transparencyBackground->setBrush(qtwidgets::getCheckerboardBrush());
    m_transparencyBackground->setPen(QPen(QColor(Qt::transparent)));

    m_imageMonitor = nullptr;
    m_displayingReloadAlert = false;
}

ImageEntry *ImagesetEntry::getImageEntry(const QString &name)
{
    for (ImageEntry* image : m_imageEntries) {
        if (image->name() == name)
            return image;
    }

    Q_ASSERT(false);
    return nullptr;
}

void ImagesetEntry::slot_imageChangedByExternalProgram()
{
    /**Monitor the image with a QFilesystemWatcher, ask user to reload
        if changes to the file were made.*/

    if (!m_displayingReloadAlert) {
        m_displayingReloadAlert = true;
        int ret = QMessageBox::question(m_visual->m_tabbedEditor->m_mainWindow,
                                        QString("Underlying image '%1' has been modified externally!").arg(m_imageFile),
                                                "The file has been modified outside the CEGUI Unified Editor.\n\nReload the file?\n\nIf you select Yes, UNDO HISTORY MIGHT BE PARTIALLY BROKEN UNLESS THE NEW SIZE IS THE SAME OR LARGER THAN THE OLD!",
                                                QMessageBox::No | QMessageBox::Yes,
                                                QMessageBox::No); // defaulting to No is safer IMO

        if (ret == QMessageBox::Yes)
            loadImage(m_imageFile);

        else if (ret == QMessageBox::No)
            ;

        else
            // how did we get here?
            Q_ASSERT(false);

        m_displayingReloadAlert = false;
    }
}

void ImagesetEntry::loadImage(const QString &relativeImagePath)
{
    // If imageMonitor is null, then no images are being watched or the
    // editor is first being opened up
    // Otherwise, the image is being changed or switched, and the monitor
    // should update itself accordingly
    if (m_imageMonitor != nullptr)
        m_imageMonitor->removePath(getAbsoluteImageFile());

    m_imageFile = relativeImagePath;
    setPixmap(QPixmap(getAbsoluteImageFile()));
    m_transparencyBackground->setRect(boundingRect());

    // go over all image entries and set their position to force them to be constrained
    // to the new pixmap's dimensions
    for (ImageEntry* imageEntry : m_imageEntries) {
        imageEntry->setPos(imageEntry->pos());
        imageEntry->updateDockWidget();
    }

    m_visual->refreshSceneRect();

    // If imageMonitor is null, allocate and watch the loaded file
    if (m_imageMonitor == nullptr) {
        m_imageMonitor = new QFileSystemWatcher(nullptr);
        QObject::connect(m_imageMonitor, &QFileSystemWatcher::fileChanged, [=](){ slot_imageChangedByExternalProgram(); });
    }
    m_imageMonitor->addPath(getAbsoluteImageFile());
}

QString ImagesetEntry::getAbsoluteImageFile()
{
    return os.path.join(os.path.dirname(m_visual->m_tabbedEditor->m_filePath), m_imageFile);
}

QString ImagesetEntry::convertToRelativeImageFile(const QString &absoluteImageFile)
{
    return os.path.normpath(os.path.relpath(absoluteImageFile, os.path.dirname(m_visual->m_tabbedEditor->m_filePath)));
}

void ImagesetEntry::loadFromElement(ElementTree::Element *element)
{
    m_name = element->get("name", "Unknown");

    loadImage(element->get("imagefile", ""));

    m_nativeHorzRes = element->getInt("nativeHorzRes", 800);
    m_nativeVertRes = element->getInt("nativeVertRes", 600);
    m_autoScaled = element->get("autoScaled");

    for (auto imageElement : element->findall("Image")) {
        ImageEntry* image = new ImageEntry(this);
        image->loadFromElement(imageElement);
        m_imageEntries.append(image);
    }
}

ElementTree::Element* ImagesetEntry::saveToElement()
{
    auto ret = new ElementTree::Element("Imageset");

    ret->set("version", "2");

    ret->set("name", m_name);
    ret->set("imagefile", m_imageFile);

    ret->set("nativeHorzRes", QString::number(m_nativeHorzRes));
    ret->set("nativeVertRes", QString::number(m_nativeVertRes));
    ret->set("autoScaled", m_autoScaled);

    for (ImageEntry* image : m_imageEntries) {
        ret->append(image->saveToElement());
    }

    return ret;
}


} // namespace elements
} // namespace imageset
} // namespace editors
} // namespace CEED
