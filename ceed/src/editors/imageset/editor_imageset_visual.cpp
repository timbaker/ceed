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

#include "editor_imageset_visual.h"

#include "action/action__init__.h"
#include "action/declaration.h"

#include "mainwindow.h"
#include "project.h"
#include "qtwidgets.h"
#include "resizable.h"

#include "editor_imageset_elements.h"
#include "editor_imageset_undo.h"

#include "ui_ImagesetEditorDockWidget.h"

#include <QClipboard>
#include <QGLWidget>
#include <QMenu>
#include <QToolBar>

namespace CEED {
namespace editors {
namespace imageset {
namespace visual {


void ImageEntryItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
#if 0 // FIXME: can't change, method is 'const'
    m_editing = true;
#endif
    QItemDelegate::setEditorData(editor, index);
}

void ImageEntryItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QItemDelegate::setModelData(editor, model, index);

#if 0 // FIXME: can't change, method is 'const'
    m_editing = false;
#endif
}

/////

ImagesetEditorDockWidget::ImagesetEditorDockWidget(VisualEditing *visual)
    : QDockWidget()
{
    m_visual = visual;

    m_ui = new Ui_ImagesetEditorDockWidget();
    m_ui->setupUi(this);

    m_name = m_ui->name;
    connect(m_name, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::slot_nameEdited);

    m_image = m_ui->image;
    // nasty, but at this point tabbedEditor.mainWindow isn't set yet
    m_image->startDirectory = [](){
        project::Project* project = mainwindow::MainWindow::instance->m_project;
        return project ? project->getResourceFilePath("", "imagesets") : "";
    };

    m_imageLoad = m_ui->imageLoad;
    connect(m_imageLoad, &QPushButton::clicked, this, &ImagesetEditorDockWidget::slot_imageLoadClicked);

    m_autoScaled = m_ui->autoScaled;
    connect(m_autoScaled, (void(QComboBox::*)(const QString&))&QComboBox::currentIndexChanged, this, &ImagesetEditorDockWidget::slot_autoScaledChanged);

    m_nativeHorzRes = m_ui->nativeHorzRes;
    m_nativeHorzRes->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_nativeHorzRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::slot_nativeResolutionEdited);

    m_nativeVertRes = m_ui->nativeVertRes;
    m_nativeVertRes->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_nativeVertRes, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::slot_nativeResolutionEdited);

    m_filterBox = m_ui->filterBox;
    connect(m_filterBox, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::filterChanged);

    m_list = m_ui->list;
    m_list->setItemDelegate(new ImageEntryItemDelegate());
    connect(m_list, &QListWidget::itemSelectionChanged, this, &ImagesetEditorDockWidget::slot_itemSelectionChanged);
    connect(m_list, &QListWidget::itemChanged, this, &ImagesetEditorDockWidget::slot_itemChanged);

    m_selectionUnderway = false;
    m_selectionSynchronisationUnderway = false;

    m_positionX = m_ui->positionX;
    m_positionX->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_positionX, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_positionXChanged);

    m_positionY = m_ui->positionY;
    m_positionY->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_positionY, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_positionYChanged);

    m_width = m_ui->width;
    m_width->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_width, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_widthChanged);

    m_height = m_ui->height;
    m_height->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_height, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_heightChanged);

    m_offsetX = m_ui->offsetX;
    m_offsetX->setValidator(new QIntValidator(-9999999, 9999999, this));
    connect(m_offsetX, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_offsetXChanged);

    m_offsetY = m_ui->offsetY;
    m_offsetY->setValidator(new QIntValidator(-9999999, 9999999, this));
    connect(m_offsetY, &QLineEdit::textChanged, this, &ImagesetEditorDockWidget::slot_offsetYChanged);

    m_autoScaledPerImage = m_ui->autoScaledPerImage;
    connect(m_autoScaledPerImage, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &ImagesetEditorDockWidget::slot_autoScaledPerImageChanged);

    m_nativeHorzResPerImage = m_ui->nativeHorzResPerImage;
    m_nativeHorzResPerImage->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_nativeHorzResPerImage, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::slot_nativeResolutionPerImageEdited);

    m_nativeVertResPerImage = m_ui->nativeVertResPerImage;
    m_nativeVertResPerImage->setValidator(new QIntValidator(0, 9999999, this));
    connect(m_nativeVertResPerImage, &QLineEdit::textEdited, this, &ImagesetEditorDockWidget::slot_nativeResolutionPerImageEdited);

    setActiveImageEntry(nullptr);
}

void ImagesetEditorDockWidget::refresh()
{
    // FIXME: This is really really weird!
    //        If I call list.clear() it crashes when undoing image deletes for some reason
    //        I already spent several hours tracking it down and I couldn't find anything
    //
    //        If I remove items one by one via takeItem, everything works :-/
    //m_list.clear()

    m_selectionSynchronisationUnderway = true;

    while (auto item = m_list->takeItem(0)) { delete item; }

    m_selectionSynchronisationUnderway = false;

    setActiveImageEntry(nullptr);

    m_name->setText(m_imagesetEntry->m_name);
    m_image->setText(m_imagesetEntry->getAbsoluteImageFile());
    m_autoScaled->setCurrentIndex(m_autoScaled->findText(m_imagesetEntry->m_autoScaled));
    m_nativeHorzRes->setText(QString::number(m_imagesetEntry->m_nativeHorzRes));
    m_nativeVertRes->setText(QString::number(m_imagesetEntry->m_nativeVertRes));

    for (ImageEntry* imageEntry : m_imagesetEntry->m_imageEntries) {
        ImagesetEditorItem* item = new ImagesetEditorItem();
        item->m_dockWidget = this;
        item->setFlags(Qt::ItemIsSelectable |
                       Qt::ItemIsEditable |
                       Qt::ItemIsEnabled);

        item->m_imageEntry = imageEntry;
        imageEntry->m_listItem = item;
        // nothing is selected (list was cleared) so we don't need to call
        //  the whole updateDockWidget here
        imageEntry->updateListItem();

        m_list->addItem(item);
    }

    // explicitly call the filtering again to make sure it's in sync
    filterChanged(m_filterBox->text());
}

void ImagesetEditorDockWidget::setActiveImageEntry(elements::ImageEntry *imageEntry)
{
    m_activeImageEntry = imageEntry;

    refreshActiveImageEntry();
}

void ImagesetEditorDockWidget::refreshActiveImageEntry()
{
    /**Refreshes the properties of active image entry (from image entry to the property box)
        */

    if (m_activeImageEntry == nullptr) {
        m_positionX->setText("");
        m_positionX->setEnabled(false);
        m_positionY->setText("");
        m_positionY->setEnabled(false);
        m_width->setText("");
        m_width->setEnabled(false);
        m_height->setText("");
        m_height->setEnabled(false);
        m_offsetX->setText("");
        m_offsetX->setEnabled(false);
        m_offsetY->setText("");
        m_offsetY->setEnabled(false);

        m_autoScaledPerImage->setCurrentIndex(0);
        m_autoScaledPerImage->setEnabled(false);
        m_nativeHorzResPerImage->setText("");
        m_nativeHorzResPerImage->setEnabled(false);
        m_nativeVertResPerImage->setText("");
        m_nativeVertResPerImage->setEnabled(false);

    } else {
        m_positionX->setText(QString::number(m_activeImageEntry->xpos()));
        m_positionX->setEnabled(true);
        m_positionY->setText(QString::number(m_activeImageEntry->ypos()));
        m_positionY->setEnabled(true);
        m_width->setText(QString::number(m_activeImageEntry->width()));
        m_width->setEnabled(true);
        m_height->setText(QString::number(m_activeImageEntry->height()));
        m_height->setEnabled(true);
        m_offsetX->setText(QString::number(m_activeImageEntry->xoffset()));
        m_offsetX->setEnabled(true);
        m_offsetY->setText(QString::number(m_activeImageEntry->yoffset()));
        m_offsetY->setEnabled(true);

        m_autoScaledPerImage->setCurrentIndex(m_autoScaledPerImage->findText(m_activeImageEntry->m_autoScaled));
        m_autoScaledPerImage->setEnabled(true);
        m_nativeHorzResPerImage->setText(QString::number(m_activeImageEntry->m_nativeHorzRes));
        m_nativeHorzResPerImage->setEnabled(true);
        m_nativeVertResPerImage->setText(QString::number(m_activeImageEntry->m_nativeVertRes));
        m_nativeVertResPerImage->setEnabled(true);
    }
}

void ImagesetEditorDockWidget::keyReleaseEvent(QKeyEvent *event)
{
    // if we are editing, we should discard key events
    // (delete means delete character, not delete image entry in this context)

    if (!dynamic_cast<ImageEntryItemDelegate*>(m_list->itemDelegate())->m_editing) {
        if (event->key() == Qt::Key_Delete) {
            auto selection = m_visual->scene()->selectedItems();

            QList<ImageEntry*> entries;
            std::for_each(selection.begin(), selection.end(), [&](QGraphicsItem* i) {
                if (auto imageEntry = dynamic_cast<ImageEntry*>(i)) {
                    entries += imageEntry;
                }
            });
            bool handled = m_visual->deleteImageEntries(entries);

            if (handled) {
                return;
            }
        }
    }

    QDockWidget::keyReleaseEvent(event);
}

void ImagesetEditorDockWidget::slot_itemSelectionChanged()
{
    auto imageEntryNames = m_list->selectedItems();
    if (imageEntryNames.length() == 1) {
        auto item = dynamic_cast<ImagesetEditorItem*>(imageEntryNames[0]);
        ImageEntry* imageEntry = item->m_imageEntry;
        setActiveImageEntry(imageEntry);
    } else {
        setActiveImageEntry(nullptr);
    }

    // we are getting synchronised with the visual editing pane, do not interfere
    if (m_selectionSynchronisationUnderway) {
        return;
    }

    m_selectionUnderway = true;
    m_visual->scene()->clearSelection();

    imageEntryNames = m_list->selectedItems();
    for (QListWidgetItem* imageEntryName : imageEntryNames) {
        auto item = dynamic_cast<ImagesetEditorItem*>(imageEntryName);
        ImageEntry* imageEntry = item->m_imageEntry;
        imageEntry->setSelected(true);
    }

    if (imageEntryNames.length() == 1) {
        auto item = dynamic_cast<ImagesetEditorItem*>(imageEntryNames[0]);
        ImageEntry* imageEntry = item->m_imageEntry;
        m_visual->centerOn(imageEntry);
    }

    m_selectionUnderway = false;
}

void ImagesetEditorDockWidget::slot_itemChanged(QListWidgetItem *item_)
{
    auto item = dynamic_cast<ImagesetEditorItem*>(item_);

    QString oldName = item->m_imageEntry->name();
    QString newName = item->text();

    if (oldName == newName) {
        // most likely caused by RenameCommand doing it's work or is bogus anyways
        return;
    }

    auto cmd = new undo::RenameCommand(m_visual, oldName, newName);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::filterChanged(const QString &filter_)
{
    // we append star at the beginning and at the end by default (makes property filtering much more practical)
    QString filter = ".*" + QRegExp::escape(filter_) + ".*";
    QRegExp regex(filter, Qt::CaseInsensitive);

    int i = 0;
    while (i < m_list->count()) {
        QListWidgetItem* listItem = m_list->item(i);
        bool match = regex.indexIn(listItem->text()) != -1;
        listItem->setHidden(!match);

        i += 1;
    }
}

void ImagesetEditorDockWidget::slot_nameEdited(const QString &newValue)
{
    QString oldName = m_imagesetEntry->m_name;
    QString newName = m_name->text();

    if (oldName == newName) {
        return;
    }

    auto cmd = new undo::ImagesetRenameCommand(m_visual, oldName, newName);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::slot_imageLoadClicked()
{
    QString oldImageFile = m_imagesetEntry->m_imageFile;
    QString newImageFile = m_imagesetEntry->convertToRelativeImageFile(m_image->text());

    if (oldImageFile == newImageFile) {
        return;
    }

    auto cmd = new undo::ImagesetChangeImageCommand(m_visual, oldImageFile, newImageFile);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::slot_autoScaledChanged(const QString &index)
{
    QString oldAutoScaled = m_imagesetEntry->m_autoScaled;
    QString newAutoScaled = m_autoScaled->currentText();

    if (oldAutoScaled == newAutoScaled) {
        return;
    }

    auto cmd = new undo::ImagesetChangeAutoScaledCommand(m_visual, oldAutoScaled, newAutoScaled);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::slot_nativeResolutionEdited(const QString &newValue)
{
    int oldHorzRes = m_imagesetEntry->m_nativeHorzRes;
    int oldVertRes = m_imagesetEntry->m_nativeVertRes;

    bool ok;
    int newHorzRes = m_nativeHorzRes->text().toInt(&ok);
    if (!ok)
        return;

    int newVertRes = m_nativeVertRes->text().toInt(&ok);
    if (!ok)
        return;

    if (oldHorzRes == newHorzRes && oldVertRes == newVertRes) {
        return;
    }

    auto* cmd = new undo::ImagesetChangeNativeResolutionCommand(m_visual, oldHorzRes, oldVertRes, newHorzRes, newVertRes);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::metaslot_propertyChangedInt(const QString &propertyName, const QString &newTextValue)
{
    if (!m_activeImageEntry) {
        return;
    }

    int oldValue = m_activeImageEntry->getPropertyValue(propertyName).toInt();

    // if the string is not a valid integer literal, we allow user to edit some more
    bool ok;
    int newValue = newTextValue.toInt(&ok);
    if (!ok)
        return;

    if (oldValue == newValue) {
        return;
    }

    auto* cmd = new undo::PropertyEditCommand(m_visual, m_activeImageEntry->name(), propertyName, oldValue, newValue);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::metaslot_propertyChangedString(const QString &propertyName, const QString &newValue)
{
    if (!m_activeImageEntry) {
        return;
    }

    QString oldValue = m_activeImageEntry->getPropertyValue(propertyName).toString();

    if (oldValue == newValue) {
        return;
    }

    auto* cmd = new undo::PropertyEditCommand(m_visual, m_activeImageEntry->name(), propertyName, oldValue, newValue);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

void ImagesetEditorDockWidget::slot_autoScaledPerImageChanged(int index)
{
    QString text;
    if (index == 0) {
        // first is the "default" / inheriting state
        text = "";
    } else {
        text = m_autoScaledPerImage->currentText();
    }

    metaslot_propertyChangedString("autoScaled", text);
}

void ImagesetEditorDockWidget::slot_nativeResolutionPerImageEdited(const QString &newValue)
{
    int oldHorzRes = m_activeImageEntry->m_nativeHorzRes;
    int oldVertRes = m_activeImageEntry->m_nativeVertRes;

    QString newHorzResStr = m_nativeHorzResPerImage->text();
    QString newVertResStr = m_nativeVertResPerImage->text();
    int newHorzRes = 0;
    int newVertRes = 0;
    bool ok;
    if (newHorzResStr != "") {
        newHorzRes = newHorzResStr.toInt(&ok);
        if (!ok)
            return;
    }
    if (newVertResStr != "") {
        newVertRes = newVertResStr.toInt(&ok);
        if (ok)
            return;
    }


    if (oldHorzRes == newHorzRes && oldVertRes == newVertRes) {
        return;
    }

    auto* cmd = new undo::PropertyEditCommand(m_visual, m_activeImageEntry->name(), "nativeRes", (oldHorzRes, oldVertRes), (newHorzRes, newVertRes));
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

/////

VisualEditing::VisualEditing(UndoStackTabbedEditor *tabbedEditor)
    : resizable::GraphicsView()
    , multi::EditMode()
{
    m_wheelZoomEnabled = true;
    m_middleButtonDragScrollEnabled = true;

    m_lastMousePosition = QPoint();

    QGraphicsScene* scene = new QGraphicsScene();
    setScene(scene);

    setFocusPolicy(Qt::ClickFocus);
    setFrameStyle(QFrame::NoFrame);

    if (settings::getEntry("imageset/visual/partial_updates")->m_value.toBool()) {
        // the commented lines are possible optimisation, I found out that they don't really
        // speed it up in a noticeable way so I commented them out

        //setOptimizationFlag(QGraphicsView.DontSavePainterState, true)
        //setOptimizationFlag(QGraphicsView.DontAdjustForAntialiasing, true)
        //setCacheMode(QGraphicsView.CacheBackground)
        setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
        //setRenderHint(QPainter.Antialiasing, false)
        //setRenderHint(QPainter.TextAntialiasing, false)
        //setRenderHint(QPainter.SmoothPixmapTransform, false)

    } else {
        // use OpenGL for view redrawing
        // depending on the platform and hardware this may be faster or slower
        setViewport(new QGLWidget());
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    }

    connect(scene, &QGraphicsScene::selectionChanged, this, &VisualEditing::slot_selectionChanged);

    m_tabbedEditor = tabbedEditor;

    setDragMode(QGraphicsView::RubberBandDrag);
    setBackgroundBrush(QBrush(Qt::lightGray));

    m_imagesetEntry = nullptr;

    m_dockWidget = new ImagesetEditorDockWidget(this);

    setupActions();
}

void VisualEditing::setupActions()
{
    m_connectionGroup = new action::ConnectionGroup(action::ActionManager::instance);

    m_editOffsetsAction = action::getAction("imageset/edit_offsets");
    m_connectionGroup->add(m_editOffsetsAction, [=](bool enabled){ slot_toggleEditOffsets(enabled); });

    m_cycleOverlappingAction = action::getAction("imageset/cycle_overlapping");
    m_connectionGroup->add(m_cycleOverlappingAction, [=](){ cycleOverlappingImages(); });

    m_createImageAction = action::getAction("imageset/create_image");
    m_connectionGroup->add(m_createImageAction, [=](){ createImageAtCursor(); });

    m_duplicateSelectedImagesAction = action::getAction("imageset/duplicate_image");
    m_connectionGroup->add(m_duplicateSelectedImagesAction, [=](){ duplicateSelectedImageEntries(); });

    m_focusImageListFilterBoxAction = action::getAction("imageset/focus_image_list_filter_box");
    m_connectionGroup->add(m_focusImageListFilterBoxAction, [=](){ focusImageListFilterBox(); });

    m_toolBar = new QToolBar("Imageset");
    m_toolBar->setObjectName("ImagesetToolbar");
    m_toolBar->setIconSize(QSize(32, 32));

    m_toolBar->addAction(m_createImageAction);
    m_toolBar->addAction(m_duplicateSelectedImagesAction);
    m_toolBar->addSeparator(); // ---------------------------
    m_toolBar->addAction(m_editOffsetsAction);
    m_toolBar->addAction(m_cycleOverlappingAction);

    setContextMenuPolicy(Qt::CustomContextMenu);

    m_contextMenu = new QMenu(this);
    connect(this, &VisualEditing::customContextMenuRequested, this, &VisualEditing::slot_customContextMenu);

    m_contextMenu->addAction(m_createImageAction);
    m_contextMenu->addAction(m_duplicateSelectedImagesAction);
    m_contextMenu->addAction(action::getAction("all_editors/delete"));
    m_contextMenu->addSeparator(); // ----------------------
    m_contextMenu->addAction(m_cycleOverlappingAction);
    m_contextMenu->addSeparator(); // ----------------------
    m_contextMenu->addAction(action::getAction("all_editors/zoom_in"));
    m_contextMenu->addAction(action::getAction("all_editors/zoom_out"));
    m_contextMenu->addAction(action::getAction("all_editors/zoom_reset"));
    m_contextMenu->addSeparator(); // ----------------------
    m_contextMenu->addAction(m_editOffsetsAction);
}

void VisualEditing::rebuildEditorMenu(QMenu *editorMenu)
{
    // similar to the toolbar, includes the focus filter box action
    editorMenu->addAction(m_createImageAction);
    editorMenu->addAction(m_duplicateSelectedImagesAction);
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(m_cycleOverlappingAction);
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(m_editOffsetsAction);
    editorMenu->addSeparator(); // ---------------------------
    editorMenu->addAction(m_focusImageListFilterBoxAction);
}

void VisualEditing::refreshSceneRect()
{
    QRectF boundingRect = m_imagesetEntry->boundingRect();

    // the reason to make the bounding rect 100px bigger on all the sides is to make
    // middle button drag scrolling easier (you can put the image where you want without
    // running out of scene

    boundingRect.adjust(-100, -100, 100, 100);
    scene()->setSceneRect(boundingRect);
}

void VisualEditing::loadImagesetEntryFromElement(ElementTree::Element *element)
{
    scene()->clear();

    m_imagesetEntry = new elements::ImagesetEntry(this);
    m_imagesetEntry->loadFromElement(element);
    scene()->addItem(m_imagesetEntry);

    refreshSceneRect();

    m_dockWidget->setImagesetEntry(m_imagesetEntry);
    m_dockWidget->refresh();
}

bool VisualEditing::moveImageEntries(const QList<elements::ImageEntry *> &imageEntries, const QPointF &delta)
{
    if (delta.manhattanLength() > 0 && !imageEntries.isEmpty()) {
        QStringList imageNames;
        QMap<QString, QPointF> oldPositions;
        QMap<QString, QPointF> newPositions;

        for (ImageEntry* imageEntry : imageEntries) {
            imageNames.append(imageEntry->name());
            oldPositions[imageEntry->name()] = imageEntry->pos();
            newPositions[imageEntry->name()] = imageEntry->pos() + delta;
        }

        auto* cmd = new undo::MoveCommand(this, imageNames, oldPositions, newPositions);
        m_tabbedEditor->m_undoStack->push(cmd);

        // we handled this
        return true;
    }

    // we didn't handle this
    return false;
}

bool VisualEditing::resizeImageEntries(const QList<elements::ImageEntry *> &imageEntries, const QPointF &topLeftDelta, const QPointF &bottomRightDelta)
{
    if ((topLeftDelta.manhattanLength() > 0 || bottomRightDelta.manhattanLength() > 0) && !imageEntries.isEmpty()) {
        QStringList imageNames;
        QMap<QString, QPointF> oldPositions;
        QMap<QString, QRectF> oldRects;
        QMap<QString, QPointF> newPositions;
        QMap<QString, QRectF> newRects;

        for (ImageEntry* imageEntry : imageEntries) {

            imageNames.append(imageEntry->name());
            oldPositions[imageEntry->name()] = imageEntry->pos();
            newPositions[imageEntry->name()] = imageEntry->pos() - topLeftDelta;
            oldRects[imageEntry->name()] = imageEntry->rect();

            QRectF newRect = imageEntry->rect();
            newRect.setBottomRight(newRect.bottomRight() - topLeftDelta + bottomRightDelta);

            if (newRect.width() < 1) {
                newRect.setWidth(1);
            }
            if (newRect.height() < 1) {
                newRect.setHeight(1);
            }

            newRects[imageEntry->name()] = newRect;
        }

        auto* cmd = new undo::GeometryChangeCommand(this, imageNames, oldPositions, oldRects, newPositions, newRects);
        m_tabbedEditor->m_undoStack->push(cmd);

        // we handled this
        return true;
    }

    // we didn't handle this
    return false;
}

bool VisualEditing::cycleOverlappingImages()
{
    QList<QGraphicsItem*> selection = scene()->selectedItems();

    if (selection.length() == 1) {
        QRectF rect = selection[0]->boundingRect();
        rect.translate(selection[0]->pos());

        QList<QGraphicsItem*> overlappingItems = scene()->items(rect);

        // first we stack everything before our current selection
        QGraphicsItem* successor = nullptr;
        for (QGraphicsItem* item : overlappingItems) {
            if ((item == selection[0]) || (item->parentItem() != selection[0]->parentItem())) {
                continue;
            }

            if (!successor && dynamic_cast<elements::ImageEntry*>(item)) {
                successor = item;
            }
        }

        if (successor) {
            for (QGraphicsItem* item : overlappingItems) {
                if (item == successor || item->parentItem() != successor->parentItem()) {
                    continue;
                }
                successor->stackBefore(item);
            }

            // we deselect current
            selection[0]->setSelected(false);
            if (auto imageEntry = dynamic_cast<elements::ImageEntry*>(selection[0]))
                imageEntry->hoverLeaveEvent(nullptr);
            else if (auto imageLabel = dynamic_cast<elements::ImageLabel*>(selection[0]))
                imageLabel->hoverLeaveEvent(nullptr);
            else if (auto imageOffset = dynamic_cast<elements::ImageOffset*>(selection[0]))
                imageOffset->hoverLeaveEvent(nullptr);

            // and select what was at the bottom (thus getting this to the top)
            successor->setSelected(true);
            if (auto imageEntry = dynamic_cast<elements::ImageEntry*>(selection[0]))
                imageEntry->hoverEnterEvent(nullptr);
            else if (auto imageLabel = dynamic_cast<elements::ImageLabel*>(selection[0]))
                imageLabel->hoverEnterEvent(nullptr);
            else if (auto imageOffset = dynamic_cast<elements::ImageOffset*>(selection[0]))
                imageOffset->hoverEnterEvent(nullptr);
        }

        // we handled this
        return true;
    }

    // we didn't handle this
    return false;
}

void VisualEditing::createImage(int centrePositionX, int centrePositionY)
{
    // find a unique image name
    QString name = "NewImage";
    int index = 1;

    while (true) {
        bool found = false;
        for (ImageEntry* imageEntry : m_imagesetEntry->m_imageEntries) {
            if (imageEntry->name() == name) {
                found = true;
                break;
            }
        }

        if (found) {
            name = QString("NewImage_%1").arg(index);
            index += 1;
        } else {
            break;
        }
    }

   int  halfSize = 25;

    int xpos = centrePositionX - halfSize;
    int ypos = centrePositionY - halfSize;
    int width = 2 * halfSize;
    int height = 2 * halfSize;
    int xoffset = 0;
    int yoffset = 0;

    auto* cmd = new undo::CreateCommand(this, name, xpos, ypos, width, height, xoffset, yoffset);
    m_tabbedEditor->m_undoStack->push(cmd);
}

void VisualEditing::createImageAtCursor()
{
    Q_ASSERT(!m_lastMousePosition.isNull());
    QPointF sceneCoordinates = mapToScene(m_lastMousePosition);

    createImage(int(sceneCoordinates.x()), int(sceneCoordinates.y()));
}

elements::ImageEntry *VisualEditing::getImageByName(const QString &name)
{
    for (ImageEntry* imageEntry : m_imagesetEntry->m_imageEntries) {
        if (imageEntry->name() == name) {
            return imageEntry;
        }
    }
    return nullptr;
}

QString VisualEditing::getNewImageName(const QString &desiredName_, const QString &copyPrefix, const QString &copySuffix)
{
    // Try the desired name exactly
    if (getImageByName(desiredName_) == nullptr) {
        return desiredName_;
    }

    // Try with prefix and suffix
    QString desiredName = copyPrefix + desiredName_ + copySuffix;
    if (getImageByName(desiredName) == nullptr) {
        return desiredName;
    }

    // We're forced to append a counter, start with number 2 (_copy2, copy3, etc.)
    int counter = 2;
    while (true) {
        QString tmpName = desiredName + QString::number(counter);
        if (getImageByName(tmpName) == nullptr) {
            return tmpName;
        }
        counter += 1;
    }
}

bool VisualEditing::duplicateImageEntries(const QList<elements::ImageEntry *> &imageEntries)
{
    if (!imageEntries.isEmpty()) {
        QStringList newNames;

        QMap<QString, QPointF> newPositions;
        QMap<QString, QRectF> newRects;
        QMap<QString, QPointF> newOffsets;

        for (ImageEntry* imageEntry : imageEntries) {
            QString newName = getNewImageName(imageEntry->name());
            newNames.append(newName);

            newPositions[newName] = imageEntry->pos();
            newRects[newName] = imageEntry->rect();
            newOffsets[newName] = imageEntry->m_offset->pos();
        }

        auto* cmd = new undo::DuplicateCommand(this, newNames, newPositions, newRects, newOffsets);
        m_tabbedEditor->m_undoStack->push(cmd);

        return true;

    } else {
        // we didn't handle this
        return false;
    }
}

bool VisualEditing::duplicateSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    QList<ImageEntry*> imageEntries;
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<elements::ImageEntry*>(item_)) {
            imageEntries.append(item);
        }
    }

    return duplicateImageEntries(imageEntries);
}

bool VisualEditing::deleteImageEntries(const QList<elements::ImageEntry *> &imageEntries)
{
    if (imageEntries.length() > 0) {
        QStringList oldNames;

        QMap<QString, QPointF> oldPositions;
        QMap<QString, QRectF> oldRects;
        QMap<QString, QPointF> oldOffsets;


        for (ImageEntry* imageEntry : imageEntries) {
            oldNames.append(imageEntry->name());

            oldPositions[imageEntry->name()] = imageEntry->pos();
            oldRects[imageEntry->name()] = imageEntry->rect();
            oldOffsets[imageEntry->name()] = imageEntry->m_offset->pos();
        }

        auto* cmd = new undo::DeleteCommand(this, oldNames, oldPositions, oldRects, oldOffsets);
        m_tabbedEditor->m_undoStack->push(cmd);

        return true;

    } else {
        // we didn't handle this
        return false;
    }
}

bool VisualEditing::deleteSelectedImageEntries()
{
    auto selection = scene()->selectedItems();

    QList<ImageEntry*> imageEntries;
    for (QGraphicsItem* item_ : selection) {
        if (auto item = dynamic_cast<elements::ImageEntry*>(item_)) {
            imageEntries.append(item);
        }
    }

    return deleteImageEntries(imageEntries);
}

void VisualEditing::showEvent(QShowEvent *event)
{
    m_dockWidget->setEnabled(true);
    m_toolBar->setEnabled(true);
    if (m_tabbedEditor->editorMenu() != nullptr) {
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(true);
    }

    // connect all our actions
    m_connectionGroup->connectAll();
    // call this every time the visual editing is shown to sync all entries up
    slot_toggleEditOffsets(m_editOffsetsAction->isChecked());

    resizable::GraphicsView::showEvent(event);
}

void VisualEditing::hideEvent(QHideEvent *event)
{
    // disconnect all our actions
    m_connectionGroup->disconnectAll();

    m_dockWidget->setEnabled(false);
    m_toolBar->setEnabled(false);
    if (m_tabbedEditor->editorMenu() != nullptr) {
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(false);
    }

    resizable::GraphicsView::hideEvent(event);
}

void VisualEditing::mousePressEvent(QMouseEvent *event)
{
    resizable::GraphicsView::mousePressEvent(event);

    if (event->buttons() & Qt::LeftButton) {
        for (QGraphicsItem* selectedItem : scene()->selectedItems()) {
            if (auto imageEntry = dynamic_cast<elements::ImageEntry*>(selectedItem)) {
                imageEntry->m_potentialMove = true;
                imageEntry->m_oldPosition = nonstd::nullopt;
            } else if (auto imageOffset = dynamic_cast<elements::ImageOffset*>(selectedItem)) {
                imageOffset->m_potentialMove = true;
                imageOffset->m_oldPosition = nonstd::nullopt;
            } else if (auto resizeHandle = dynamic_cast<resizable::ResizingHandle*>(selectedItem)) {
            }
        }
    }
}

void VisualEditing::mouseReleaseEvent(QMouseEvent *event)
{
    resizable::GraphicsView::mouseReleaseEvent(event);

    // moving
    QStringList moveImageNames;
    QMap<QString, QPointF> moveImageOldPositions;
    QMap<QString, QPointF> moveImageNewPositions;

    QStringList moveOffsetNames;
    QMap<QString, QPointF> moveOffsetOldPositions;
    QMap<QString, QPointF> moveOffsetNewPositions;

    // resizing
    QStringList resizeImageNames;
    QMap<QString, QPointF> resizeImageOldPositions;
    QMap<QString, QRectF> resizeImageOldRects ;
    QMap<QString, QPointF> resizeImageNewPositions;
    QMap<QString, QRectF> resizeImageNewRects;

    // we have to "expand" the items, adding parents of resizing handles
    // instead of the handles themselves
    QList<QGraphicsItem*> expandedSelectedItems;
    for (QGraphicsItem* selectedItem : scene()->selectedItems()) {
        if (dynamic_cast<elements::ImageEntry*>(selectedItem)) {
            expandedSelectedItems.append(selectedItem);
        } else if (dynamic_cast<elements::ImageOffset*>(selectedItem)) {
            expandedSelectedItems.append(selectedItem);
        } else if (dynamic_cast<resizable::ResizingHandle*>(selectedItem)) {
            expandedSelectedItems.append(selectedItem->parentItem());
        }
    }

    for (QGraphicsItem* selectedItem_ : expandedSelectedItems) {
        if (auto* selectedItem = dynamic_cast<elements::ImageEntry*>(selectedItem_)) {
            if (selectedItem->m_oldPosition) {
                if (selectedItem->m_mouseOver) {
                    // show the label again if mouse is over because moving finished
                    selectedItem->m_label->setVisible(true);
                }

                // only include that if the position really changed
                if (selectedItem->m_oldPosition != selectedItem->pos()) {
                    moveImageNames.append(selectedItem->name());
                    moveImageOldPositions[selectedItem->name()] = *selectedItem->m_oldPosition;
                    moveImageNewPositions[selectedItem->name()] = selectedItem->pos();
                }
            }

            if (selectedItem->m_resized) {
                // only include that if the position or rect really changed
                if (selectedItem->m_resizeOldPos != selectedItem->pos() || selectedItem->m_resizeOldRect != selectedItem->rect()) {
                    resizeImageNames.append(selectedItem->name());
                    resizeImageOldPositions[selectedItem->name()] = selectedItem->m_resizeOldPos;
                    resizeImageOldRects[selectedItem->name()] = selectedItem->m_resizeOldRect;
                    resizeImageNewPositions[selectedItem->name()] = selectedItem->pos();
                    resizeImageNewRects[selectedItem->name()] = selectedItem->rect();
                }
            }

            selectedItem->m_potentialMove = false;
            selectedItem->m_oldPosition = nonstd::nullopt;
            selectedItem->m_resized = false;

        } else if (auto* selectedItem = dynamic_cast<elements::ImageOffset*>(selectedItem_)) {
            if (selectedItem->m_oldPosition) {
                // only include that if the position really changed
                QPointF oldPosition = *selectedItem->m_oldPosition;
                if (oldPosition != selectedItem->pos()) {
                    moveOffsetNames.append(selectedItem->m_imageEntry->name());
                    moveOffsetOldPositions[selectedItem->m_imageEntry->name()] = oldPosition;
                    moveOffsetNewPositions[selectedItem->m_imageEntry->name()] = selectedItem->pos();
                }
            }

            selectedItem->m_potentialMove = false;
            selectedItem->m_oldPosition = nonstd::nullopt;
        }
    }

    // NOTE: It should never happen that more than one of these sets is populated
    //       User moves images XOR moves offsets XOR resizes images
    //
    //       I don't do elif for robustness though, who knows what can happen ;-)

    if (!moveImageNames.isEmpty()) {
        auto* cmd = new undo::MoveCommand(this, moveImageNames, moveImageOldPositions, moveImageNewPositions);
        m_tabbedEditor->m_undoStack->push(cmd);
    }

    if (!moveOffsetNames.isEmpty()) {
        auto* cmd = new undo::OffsetMoveCommand(this, moveOffsetNames, moveOffsetOldPositions, moveOffsetNewPositions);
        m_tabbedEditor->m_undoStack->push(cmd);
    }

    if (!resizeImageNames.isEmpty()) {
        auto* cmd = new undo::GeometryChangeCommand(this, resizeImageNames, resizeImageOldPositions, resizeImageOldRects, resizeImageNewPositions, resizeImageNewRects);
        m_tabbedEditor->m_undoStack->push(cmd);
    }
}

void VisualEditing::mouseMoveEvent(QMouseEvent *event)
{
    m_lastMousePosition = event->pos();

    resizable::GraphicsView::mouseMoveEvent(event);
}

void VisualEditing::keyReleaseEvent(QKeyEvent *event)
{
    // TODO: offset keyboard handling

    bool handled = false;

    QList<Qt::Key> wasd = { Qt::Key_A, Qt::Key_D, Qt::Key_W, Qt::Key_S };
    if (wasd.contains(static_cast<Qt::Key>(event->key()))) {
        QList<elements::ImageEntry*> selection;

        for (QGraphicsItem* item : scene()->selectedItems()) {
            if (auto* imageEntry = dynamic_cast<elements::ImageEntry*>(item)) {
                if (!selection.contains(imageEntry)) {
                    selection.append(imageEntry);
                }

            } else if (dynamic_cast<resizable::ResizingHandle*>(item)) {
                auto* parent = dynamic_cast<elements::ImageEntry*>(item->parentItem());
                if (!selection.contains(parent)) {
                    selection.append(parent);
                }
            }
        }

        if (!selection.isEmpty()) {
            QPointF delta;

            if (event->key() == Qt::Key_A) {
                delta += QPointF(-1, 0);
            } else if (event->key() == Qt::Key_D) {
                delta += QPointF(1, 0);
            } else if (event->key() == Qt::Key_W) {
                delta += QPointF(0, -1);
            } else if (event->key() == Qt::Key_S) {
                delta += QPointF(0, 1);
            }

            if (event->modifiers() & Qt::ControlModifier) {
                delta *= 10;
            }

            if (event->modifiers() & Qt::ShiftModifier) {
                handled = resizeImageEntries(selection, QPointF(0, 0), delta);
            } else {
                handled = moveImageEntries(selection, delta);
            }
        }

    } else if (event->key() == Qt::Key_Q) {
        handled = cycleOverlappingImages();

    } else if (event->key() == Qt::Key_Delete) {
        handled = deleteSelectedImageEntries();
    }

    if (!handled) {
        resizable::GraphicsView::keyReleaseEvent(event);

    } else {
        event->accept();
    }
}

void VisualEditing::slot_selectionChanged()
{
    // if dockWidget is changing the selection, back off
    if (m_dockWidget->m_selectionUnderway) {
        return;
    }

    auto selectedItems = scene()->selectedItems();
    if (selectedItems.length() == 1) {
        if (auto* imageEntry = dynamic_cast<elements::ImageEntry*>(selectedItems[0])) {
            m_dockWidget->m_list->scrollToItem(imageEntry->m_listItem);
        }
    }
}

void VisualEditing::slot_toggleEditOffsets(bool enabled)
{
    scene()->clearSelection();

    if (m_imagesetEntry != nullptr) {
        m_imagesetEntry->m_showOffsets = enabled;
    }
}

void VisualEditing::slot_customContextMenu(const QPoint &point)
{
    m_contextMenu->exec(mapToGlobal(point));
}

void VisualEditing::focusImageListFilterBox()
{
    auto* filterBox = m_dockWidget->m_filterBox;
    // selects all contents of the filter so that user can replace that with their search phrase
    filterBox->selectAll();
    // sets focus so that typing puts text into the filter box without clicking
    filterBox->setFocus();
}

bool VisualEditing::performCut()
{
    if (performCopy()) {
        deleteSelectedImageEntries();
        return true;
    }

    return false;
}

bool VisualEditing::performCopy()
{
    auto selection = scene()->selectedItems();
    if (selection.isEmpty()) {
        return false;
    }

    QStringList copyNames;
    QMap<QString, QPointF> copyPositions;
    QMap<QString, QRectF> copyRects;
    QMap<QString, QPointF> copyOffsets;
    for (QGraphicsItem* item_ : selection) {
        if (auto* item = dynamic_cast<elements::ImageEntry*>(item_)) {
            copyNames.append(item->name());
            copyPositions[item->name()] = item->pos();
            copyRects    [item->name()] = item->rect();
            copyOffsets  [item->name()] = item->m_offset->pos();
        }
    }
    if (copyNames.isEmpty()) {
        return false;
    }

    MimeData* data = new MimeData();
    data->copyNames = copyNames;
    data->copyPositions = copyPositions;
    data->copyRects = copyRects;
    data->copyOffsets = copyOffsets;
    data->setData("application/x-ceed-imageset-image-list", QByteArray());
    QApplication::clipboard()->setMimeData(data);

    return true;
}

bool VisualEditing::performPaste()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();
    if (!data->hasFormat("application/x-ceed-imageset-image-list")) {
        return false;
    }

    const MimeData* imageData = dynamic_cast<const MimeData*>(data);
    if (imageData == nullptr) {
        return false;
    }

    QStringList newNames;
    QMap<QString, QPointF> newPositions;
    QMap<QString, QRectF> newRects;
    QMap<QString, QPointF> newOffsets;
    for (QString copyName : imageData->copyNames) {
        QString newName = getNewImageName(copyName);
        newNames.append(newName);
        newPositions[newName] = imageData->copyPositions[copyName];
        newRects    [newName] = imageData->copyRects[copyName];
        newOffsets  [newName] = imageData->copyOffsets[copyName];
    }
    if (newNames.isEmpty()) {
        return false;
    }

    auto* cmd = new undo::PasteCommand(this, newNames, newPositions, newRects, newOffsets);
    m_tabbedEditor->m_undoStack->push(cmd);

    // select just the pasted image definitions for convenience
    scene()->clearSelection();
    for (QString& name : newNames) {
        m_imagesetEntry->getImageEntry(name)->setSelected(true);
    }

    return true;
}

bool VisualEditing::performDelete()
{
    return deleteSelectedImageEntries();
}


} // namespace visual
} // namespace imageset
} // namespace editors
} // namespace CEED
