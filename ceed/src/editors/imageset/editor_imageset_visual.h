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

#ifndef CEED_editors_imageset_visual_
#define CEED_editors_imageset_visual_

#include "CEEDBase.h"

#include "editors/editors_multi.h"
#include "resizable.h"

#include <QDockWidget>
#include <QItemDelegate>
#include <QListWidgetItem>
#include <QMimeData>

class Ui_ImagesetEditorDockWidget;

class QComboBox;
class QLineEdit;
class QListWidget;
class QMenu;
class QPushButton;
class QToolBar;

namespace CEED {
namespace editors {
namespace imageset {
namespace visual {

/*!
\brief ImageEntryItemDelegate

The only reason for this is to track when we are editing.

    We need this to discard key events when editor is open.
    TODO: Isn't there a better way to do this?

*/
class ImageEntryItemDelegate : public QItemDelegate
{
public:
    bool m_editing;

    ImageEntryItemDelegate()
        : QItemDelegate()
        , m_editing(false)
    {

    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};


// added for CEED++
class ImagesetEditorItem : public QListWidgetItem
{
public:
    ImagesetEditorDockWidget* m_dockWidget;
    elements::ImageEntry* m_imageEntry;
};


/*!
\brief ImagesetEditorDockWidget

Provides list of images, property editing of currently selected image and create/delete

*/
class ImagesetEditorDockWidget : public QDockWidget
{
public:
    Ui_ImagesetEditorDockWidget* m_ui;
    VisualEditing* m_visual;
    QLineEdit* m_name;
    qtwidgets::FileLineEdit* m_image;
    QPushButton* m_imageLoad;
    QComboBox* m_autoScaled;
    QLineEdit* m_nativeHorzRes;
    QLineEdit* m_nativeVertRes;
    QLineEdit* m_filterBox;
    QListWidget* m_list;
    bool m_selectionUnderway;
    bool m_selectionSynchronisationUnderway;
    QLineEdit* m_positionX;
    QLineEdit* m_positionY;
    QLineEdit* m_width;
    QLineEdit* m_height;
    QLineEdit* m_offsetX;
    QLineEdit* m_offsetY;
    QComboBox* m_autoScaledPerImage;
    QLineEdit* m_nativeHorzResPerImage;
    QLineEdit* m_nativeVertResPerImage;
    elements::ImagesetEntry* m_imagesetEntry;
    elements::ImageEntry* m_activeImageEntry;

    ImagesetEditorDockWidget(VisualEditing* visual);

    void setImagesetEntry(elements::ImagesetEntry* imagesetEntry)
    {
        m_imagesetEntry = imagesetEntry;
    }

    /**Refreshes the whole list

    Note: User potentially looses selection when this is called!
    */
    void refresh();

    /**Active image entry is the image entry that is selected when there are no
    other image entries selected. It's properties show in the property box.

    Note: Imageset editing doesn't allow multi selection property editing because
          IMO it doesn't make much sense.
    */
    void setActiveImageEntry(imageset::elements::ImageEntry* imageEntry);

    void refreshActiveImageEntry();

    void keyReleaseEvent(QKeyEvent *event) override;

    void slot_itemSelectionChanged();

    void slot_itemChanged(QListWidgetItem* item);

    void filterChanged(const QString& filter_);

    void slot_nameEdited(const QString& newValue);

    void slot_imageLoadClicked();

    void slot_autoScaledChanged(const QString& index);

    void slot_nativeResolutionEdited(const QString& newValue);

    void metaslot_propertyChangedInt(const QString& propertyName, const QString& newTextValue);

    void metaslot_propertyChangedString(const QString& propertyName, const QString& newValue);

    void slot_positionXChanged(const QString& text)
    {
        metaslot_propertyChangedInt("xpos", text);
    }

    void slot_positionYChanged(const QString& text)
    {
        metaslot_propertyChangedInt("ypos", text);
    }

    void slot_widthChanged(const QString& text)
    {
        metaslot_propertyChangedInt("width", text);
    }

    void slot_heightChanged(const QString& text)
    {
        metaslot_propertyChangedInt("height", text);
    }

    void slot_offsetXChanged(const QString& text)
    {
        metaslot_propertyChangedInt("xoffset", text);
    }

    void slot_offsetYChanged(const QString& text)
    {
        metaslot_propertyChangedInt("yoffset", text);
    }

    void slot_autoScaledPerImageChanged(int index);

    void slot_nativeResolutionPerImageEdited(const QString& newValue);
};

// Hack for metaimageset
class FakeVisual
{
public:
    editors::UndoStackTabbedEditor* m_tabbedEditor;
    virtual void refreshSceneRect() = 0;
};

/**This is the "Visual" tab for imageset editing
*/
class VisualEditing : public resizable::GraphicsView, public multi::EditMode, public FakeVisual
{
public:
    QPoint m_lastMousePosition;
    elements::ImagesetEntry* m_imagesetEntry;
    action::ConnectionGroup* m_connectionGroup;
    action::declaration::Action* m_editOffsetsAction;
    action::declaration::Action* m_cycleOverlappingAction;
    action::declaration::Action* m_createImageAction;
    action::declaration::Action* m_duplicateSelectedImagesAction;
    action::declaration::Action* m_focusImageListFilterBoxAction;
    QToolBar* m_toolBar;
    QMenu* m_contextMenu;
//    editors::UndoStackTabbedEditor* m_tabbedEditor;
    ImagesetEditorDockWidget* m_dockWidget;

    VisualEditing(editors::UndoStackTabbedEditor* tabbedEditor);

    void setupActions();

    /**Adds actions to the editor menu*/
    void rebuildEditorMenu(QMenu* editorMenu);

    void initialise(ElementTree::Element* rootElement)
    {
        loadImagesetEntryFromElement(rootElement);
    }

    void refreshSceneRect();

    void loadImagesetEntryFromElement(ElementTree::Element* element);

    bool moveImageEntries(const QList<elements::ImageEntry*>& imageEntries, const QPointF& delta);

    bool resizeImageEntries(const QList<elements::ImageEntry*>& imageEntries, const QPointF& topLeftDelta, const QPointF& bottomRightDelta);

    bool cycleOverlappingImages();

    /**Centre position is the position of the centre of the newly created image,
    the newly created image will then 'encapsulate' the centrepoint
    */
    void createImage(int centrePositionX, int centrePositionY);

    void createImageAtCursor();

    elements::ImageEntry* getImageByName(const QString& name);

    /**Returns an image name that is not used in this imageset

    TODO: Can be used for the copy-paste functionality too
    */
    QString getNewImageName(const QString& desiredName, const QString& copyPrefix = "", const QString& copySuffix = "_copy");

    bool duplicateImageEntries(const QList<elements::ImageEntry*>& imageEntries);

    bool duplicateSelectedImageEntries();

    bool deleteImageEntries(const QList<elements::ImageEntry*>& imageEntries);

    bool deleteSelectedImageEntries();

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    /**When mouse is released, we have to check what items were moved and resized.

    AFAIK Qt doesn't give us any move finished notification so I do this manually
    */
    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void slot_selectionChanged();

    void slot_toggleEditOffsets(bool enabled);

    void slot_customContextMenu(const QPoint &point);

    /**Focuses into image list filter

    This potentially allows the user to just press a shortcut to find images,
    instead of having to reach for a mouse.
    */
    void focusImageListFilterBox();

    bool performCut();

    class MimeData : public QMimeData
    {
    public:
        QStringList copyNames;
        QMap<QString, QPointF> copyPositions;
        QMap<QString, QRectF> copyRects;
        QMap<QString, QPointF> copyOffsets;
    };

    bool performCopy();

    bool performPaste();

    bool performDelete();
};

} // namespace visual
} // namespace imageset
} // namespace editors
} // namespace CEED

#endif
