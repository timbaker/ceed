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

#include "editor_imageset_undo.h"

#include "editors/imageset/editor_imageset_visual.h"

#include "qtwidgets.h"

#include <QComboBox>
#include <QtMath>

namespace CEED {
namespace editors {
namespace imageset {
namespace undo {

MoveCommand::MoveCommand(visual::VisualEditing *visual_, const QStringList &imageNames, const QMap<QString, QPointF> &oldPositions, const QMap<QString, QPointF> &newPositions)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_imageNames = imageNames;
    m_oldPositions = oldPositions;
    m_newPositions = newPositions;

    m_biggestDelta = 0;

    for (auto it = oldPositions.begin(); it != oldPositions.end(); it++) {
        QString imageName = it.key();
        QPointF oldPosition = it.value();

        QPointF positionDelta = oldPosition - m_newPositions[imageName];

        qreal delta = qSqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y());
        if (delta > m_biggestDelta)
            m_biggestDelta = delta;
    }

    refreshText();
}

void MoveCommand::refreshText()
{
    if (m_imageNames.length() == 1)
        setText(QString("Move '%1'").arg(m_imageNames[0]));
    else
        setText(QString("Move %1 images").arg(m_imageNames.length()));
}

bool MoveCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const MoveCommand*>(cmd_);

    if (m_imageNames == cmd->m_imageNames) {
        // good, images match

        int combinedBiggestDelta = m_biggestDelta + cmd->m_biggestDelta;
        // TODO: 50 used just for testing!
        if (combinedBiggestDelta < 50) {
            // if the combined delta is reasonably small, we can merge the commands
            m_newPositions = cmd->m_newPositions;
            m_biggestDelta = combinedBiggestDelta;

            refreshText();

            return true;
        }
    }

    return false;
}

void MoveCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString& imageName : m_imageNames) {
        elements::ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->setPos(m_oldPositions[imageName]);

        image->updateDockWidget();
    }
}

void MoveCommand::redo()
{
    for (QString& imageName : m_imageNames) {
        elements::ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->setPos(m_newPositions[imageName]);

        image->updateDockWidget();
    }

    commands::UndoCommand::redo();
}

/////

GeometryChangeCommand::GeometryChangeCommand(visual::VisualEditing *visual_, const QStringList &imageNames, const QMap<QString, QPointF> &oldPositions, const QMap<QString, QRectF> &oldRects, const QMap<QString, QPointF> &newPositions, const QMap<QString, QRectF> &newRects)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_imageNames = imageNames;
    m_oldPositions = oldPositions;
    m_oldRects = oldRects;
    m_newPositions = newPositions;
    m_newRects = newRects;

    m_biggestMoveDelta = 0;

    for (auto it = oldPositions.begin(); it != oldPositions.end(); it++) {
        QString imageName = it.key();
        QPointF oldPosition = it.value();

        QPointF moveDelta = oldPosition - m_newPositions[imageName];

        qreal delta = qSqrt(moveDelta.x() * moveDelta.x() + moveDelta.y() * moveDelta.y());
        if (delta > m_biggestMoveDelta)
            m_biggestMoveDelta = delta;
    }

    m_biggestResizeDelta = 0;

    for (auto it = oldRects.begin(); it != oldRects.end(); it++) {
        QString imageName = it.key();
        QRectF oldRect = it.value();

        QPointF resizeDelta = oldRect.bottomRight() - m_newRects[imageName].bottomRight();

        qreal delta = qSqrt(resizeDelta.x() * resizeDelta.x() + resizeDelta.y() * resizeDelta.y());
        if (delta > m_biggestResizeDelta)
            m_biggestResizeDelta = delta;
    }

    refreshText();
}

void GeometryChangeCommand::refreshText()
{
    if (m_imageNames.length() == 1)
        setText(QString("Change geometry of '%1'").arg(m_imageNames[0]));
    else
        setText(QString("Change geometry of %i images").arg(m_imageNames.length()));
}

bool GeometryChangeCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const GeometryChangeCommand*>(cmd_);

    if (m_imageNames.toSet() == cmd->m_imageNames.toSet()) {
        // good, images match

        int combinedBiggestMoveDelta = m_biggestMoveDelta + cmd->m_biggestMoveDelta;
        int combinedBiggestResizeDelta = m_biggestResizeDelta + cmd->m_biggestResizeDelta;

        // TODO: 50 and 20 are used just for testing!
        if (combinedBiggestMoveDelta < 50 && combinedBiggestResizeDelta < 20) {
            // if the combined deltas area reasonably small, we can merge the commands
            m_newPositions = cmd->m_newPositions;
            m_newRects = cmd->m_newRects;

            m_biggestMoveDelta = combinedBiggestMoveDelta;
            m_biggestResizeDelta = combinedBiggestResizeDelta;

            refreshText();

            return true;
        }
    }

    return false;
}

void GeometryChangeCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString imageName : m_imageNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->setPos(m_oldPositions[imageName]);
        image->setRect(m_oldRects[imageName]);

        image->updateDockWidget();
    }
}

void GeometryChangeCommand::redo()
{
    for (QString imageName : m_imageNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->setPos(m_newPositions[imageName]);
        image->setRect(m_newRects[imageName]);

        image->updateDockWidget();
    }

    commands::UndoCommand::redo();
}

/////

OffsetMoveCommand::OffsetMoveCommand(visual::VisualEditing *visual_, const QStringList &imageNames, const QMap<QString, QPointF> &oldPositions, const QMap<QString, QPointF> &newPositions)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_imageNames = imageNames;
    m_oldPositions = oldPositions;
    m_newPositions = newPositions;

    m_biggestDelta = 0;

    for (auto it = oldPositions.begin(); it != oldPositions.end(); it++) {
        QString imageName = it.key();
        QPointF oldPosition = it.value();

        QPointF positionDelta = oldPosition - m_newPositions[imageName];

        qreal delta = qSqrt(positionDelta.x() * positionDelta.x() + positionDelta.y() * positionDelta.y());
        if (delta > m_biggestDelta)
            m_biggestDelta = delta;
    }

    refreshText();
}

void OffsetMoveCommand::refreshText()
{
    if (m_imageNames.length() == 1)
        setText(QString("Move offset of '%1'").arg(m_imageNames[0]));
    else
        setText(QString("Move offset of %1 images").arg(m_imageNames.length()));
}

bool OffsetMoveCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const OffsetMoveCommand*>(cmd_);

    if (m_imageNames.toSet() == cmd->m_imageNames.toSet()) {
        // good, images match

        int combinedBiggestDelta = m_biggestDelta + cmd->m_biggestDelta;
        // TODO: 10 used just for testing!
        if (combinedBiggestDelta < 10) {
            // if the combined delta is reasonably small, we can merge the commands
            m_newPositions = cmd->m_newPositions;
            m_biggestDelta = combinedBiggestDelta;

            refreshText();

            return true;
        }
    }

    return false;
}

void OffsetMoveCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString imageName : m_imageNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->m_offset->setPos(m_oldPositions[imageName]);
    }
}

void OffsetMoveCommand::redo()
{
    for (QString imageName : m_imageNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        image->m_offset->setPos(m_newPositions[imageName]);
    }

    commands::UndoCommand::redo();
}

/////

RenameCommand::RenameCommand(visual::VisualEditing *visual_, const QString &oldName, const QString &newName)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_oldName = oldName;
    m_newName = newName;

    refreshText();
}

void RenameCommand::refreshText()
{
    setText(QString("Rename '%1' to '%2'").arg(m_oldName).arg(m_newName));
}

bool RenameCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const RenameCommand*>(cmd_);

    if (m_newName == cmd->m_oldName) {
        // if our old newName is the same as oldName of the command that
        // comes after this command, we can merge them
        m_newName = cmd->m_newName;
        refreshText();

        return true;
    }

    return false;
}

void RenameCommand::undo()
{
    commands::UndoCommand::undo();

    ImageEntry* imageEntry = m_visual->m_imagesetEntry->getImageEntry(m_newName);
    imageEntry->set_name(m_oldName);
    imageEntry->updateListItem();
}

void RenameCommand::redo()
{
    ImageEntry* imageEntry = m_visual->m_imagesetEntry->getImageEntry(m_oldName);
    imageEntry->set_name(m_newName);
    imageEntry->updateListItem();

    commands::UndoCommand::redo();
}

/////

PropertyEditCommand::PropertyEditCommand(visual::VisualEditing *visual_, const QString &imageName, const QString &propertyName,
                                         const QVariant &oldValue, const QVariant &newValue)
    : commands::UndoCommand()
{
    m_visual = visual_;
    m_imageName = imageName;
    m_propertyName = propertyName;
    m_oldValue = oldValue;
    m_newValue = newValue;

    refreshText();
}

void PropertyEditCommand::refreshText()
{
    setText(QString("Change %1 of '%2' to '%3'").arg(m_propertyName).arg(m_imageName).arg(m_newValue.toString()));
}

bool PropertyEditCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const PropertyEditCommand*>(cmd_);

    if (m_imageName == cmd->m_imageName && m_propertyName == cmd->m_propertyName)
        m_newValue = cmd->m_newValue;

    refreshText();

    return true;

    return false;
}

void PropertyEditCommand::undo()
{
    commands::UndoCommand::undo();

    ImageEntry* imageEntry = m_visual->m_imagesetEntry->getImageEntry(m_imageName);
    imageEntry->setPropertyValue(m_propertyName, m_oldValue);
    imageEntry->updateDockWidget();
}

void PropertyEditCommand::redo()
{
    ImageEntry* imageEntry =m_visual->m_imagesetEntry->getImageEntry(m_imageName);
    imageEntry->setPropertyValue(m_propertyName, m_newValue);
    imageEntry->updateDockWidget();

    commands::UndoCommand::redo();
}

/////

CreateCommand::CreateCommand(visual::VisualEditing *visual_, const QString &name, int xpos, int ypos, int width, int height, int xoffset, int yoffset)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_name = name;

    m_xpos = xpos;
    m_ypos = ypos;
    m_width = width;
    m_height = height;
    m_xoffset = xoffset;
    m_yoffset = yoffset;

    setText(QString("Create '%1'").arg(m_name));
}

void CreateCommand::undo()
{
    commands::UndoCommand::undo();

    ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(m_name);
    m_visual->m_imagesetEntry->m_imageEntries.removeOne(image);

    image->m_listItem->m_imageEntry = nullptr;
    image->m_listItem = nullptr;

    image->setParentItem(nullptr);
    m_visual->scene()->removeItem(image);

    m_visual->m_dockWidget->refresh();
}

void CreateCommand::redo()
{
    auto* image = new elements::ImageEntry(m_visual->m_imagesetEntry);
    m_visual->m_imagesetEntry->m_imageEntries.append(image);

    image->set_name(m_name);
    image->set_xpos(m_xpos);
    image->set_ypos(m_ypos);
    image->set_width(m_width);
    image->set_height(m_height);
    image->set_xoffset(m_xoffset);
    image->set_yoffset(m_yoffset);
    m_visual->m_dockWidget->refresh();

    commands::UndoCommand::redo();
}

/////

DeleteCommand::DeleteCommand(visual::VisualEditing *visual_, const QStringList &imageNames, const QMap<QString, QPointF> &oldPositions, const QMap<QString, QRectF> &oldRects, const QMap<QString, QPointF> &oldOffsets)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_imageNames = imageNames;

    m_oldPositions = oldPositions;
    m_oldRects = oldRects;
    m_oldOffsets = oldOffsets;

    if (m_imageNames.length() == 1)
        setText(QString("Delete '%1'").arg(m_imageNames[0]));
    else
        setText(QString("Delete %1 images").arg(m_imageNames.length()));
}

void DeleteCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString imageName : m_imageNames) {
        ImageEntry* image = new elements::ImageEntry(m_visual->m_imagesetEntry);
        m_visual->m_imagesetEntry->m_imageEntries.append(image);

        image->set_name(imageName);
        image->setPos(m_oldPositions[imageName]);
        image->setRect(m_oldRects[imageName]);
        image->m_offset->setPos(m_oldOffsets[imageName]);
    }

    m_visual->m_dockWidget->refresh();
}

void DeleteCommand::redo()
{
    for (QString imageName : m_imageNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        m_visual->m_imagesetEntry->m_imageEntries.removeOne(image);

        image->m_listItem->m_imageEntry = nullptr;
        image->m_listItem = nullptr;

        image->setParentItem(nullptr);
        m_visual->scene()->removeItem(image);

        delete image;
    }

    m_visual->m_dockWidget->refresh();

    commands::UndoCommand::redo();
}

/////

ImagesetRenameCommand::ImagesetRenameCommand(visual::VisualEditing *visual_, const QString &oldName, const QString &newName):
    commands::UndoCommand()
{
    m_visual = visual_;

    m_oldName = oldName;
    m_newName = newName;

    refreshText();
}

void ImagesetRenameCommand::refreshText()
{
    setText(QString("Rename imageset from '%1' to '%2'").arg(m_oldName, m_newName));
}

bool ImagesetRenameCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const ImagesetRenameCommand*>(cmd_);

    if (m_newName == cmd->m_oldName)
        m_newName = cmd->m_newName;
    refreshText();

    return true;

    return false;
}

void ImagesetRenameCommand::undo()
{
    commands::UndoCommand::undo();

    elements::ImagesetEntry* imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_name = m_oldName;
    m_visual->m_dockWidget->m_name->setText(m_oldName);
}

void ImagesetRenameCommand::redo()
{
    elements::ImagesetEntry* imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_name = m_newName;
    if (m_visual->m_dockWidget->m_name->text() != m_newName)
        m_visual->m_dockWidget->m_name->setText(m_newName);

    commands::UndoCommand::redo();
}

/////

ImagesetChangeImageCommand::ImagesetChangeImageCommand(visual::VisualEditing *visual_, const QString &oldImageFile, const QString &newImageFile):
    commands::UndoCommand()
{
    m_visual = visual_;

    m_oldImageFile = oldImageFile;
    m_newImageFile = newImageFile;

    refreshText();
}

void ImagesetChangeImageCommand::refreshText()
{
    setText(QString("Change underlying image from '%1' to '%2'").arg(m_oldImageFile).arg(m_newImageFile));
}

bool ImagesetChangeImageCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const ImagesetChangeImageCommand*>(cmd_);

    if (m_newImageFile == cmd->m_oldImageFile) {
        m_newImageFile = cmd->m_newImageFile;
        refreshText();

        return true;
    }

    return false;
}

void ImagesetChangeImageCommand::undo()
{
    commands::UndoCommand::undo();

    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->loadImage(m_oldImageFile);
    m_visual->m_dockWidget->m_image->setText(imagesetEntry->getAbsoluteImageFile());
}

void ImagesetChangeImageCommand::redo()
{
    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->loadImage(m_newImageFile);
    m_visual->m_dockWidget->m_image->setText(imagesetEntry->getAbsoluteImageFile());

    commands::UndoCommand::redo();
}

/////

ImagesetChangeNativeResolutionCommand::ImagesetChangeNativeResolutionCommand(visual::VisualEditing *visual_, int oldHorzRes, int oldVertRes, int newHorzRes, int newVertRes)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_oldHorzRes = oldHorzRes;
    m_oldVertRes = oldVertRes;
    m_newHorzRes = newHorzRes;
    m_newVertRes = newVertRes;

    refreshText();
}

void ImagesetChangeNativeResolutionCommand::refreshText()
{
    setText(QString("Change imageset's native resolution to %1x%2").arg(m_newHorzRes).arg(m_newVertRes));
}

bool ImagesetChangeNativeResolutionCommand::mergeWith(const QUndoCommand *other)
{
    auto cmd = static_cast<const ImagesetChangeNativeResolutionCommand*>(other);

    if (m_newHorzRes == cmd->m_oldHorzRes && m_newVertRes == cmd->m_oldVertRes) {
        m_newHorzRes = cmd->m_newHorzRes;
        m_newVertRes = cmd->m_newVertRes;

        refreshText();

        return true;
    }

    return false;
}

void ImagesetChangeNativeResolutionCommand::undo()
{
    commands::UndoCommand::undo();

    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_nativeHorzRes = m_oldHorzRes;
    imagesetEntry->m_nativeVertRes = m_oldVertRes;
    m_visual->m_dockWidget->m_nativeHorzRes->setText(QString::number(m_oldHorzRes));
    m_visual->m_dockWidget->m_nativeVertRes->setText(QString::number(m_oldVertRes));
}

void ImagesetChangeNativeResolutionCommand::redo()
{
    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_nativeHorzRes = m_newHorzRes;
    imagesetEntry->m_nativeVertRes = m_newVertRes;
    m_visual->m_dockWidget->m_nativeHorzRes->setText(QString::number(m_newHorzRes));
    m_visual->m_dockWidget->m_nativeVertRes->setText(QString::number(m_newVertRes));

    commands::UndoCommand::redo();
}

/////

ImagesetChangeAutoScaledCommand::ImagesetChangeAutoScaledCommand(visual::VisualEditing* visual_, const QString &oldAutoScaled, const QString &newAutoScaled):
    commands::UndoCommand()
{
    m_visual = visual_;

    m_oldAutoScaled = oldAutoScaled;
    m_newAutoScaled = newAutoScaled;

    refreshText();
}

void ImagesetChangeAutoScaledCommand::refreshText()
{
    setText(QString("Imageset auto scaled changed to %1").arg(m_newAutoScaled));
}

bool ImagesetChangeAutoScaledCommand::mergeWith(const QUndoCommand *other)
{
    auto cmd = static_cast<const ImagesetChangeAutoScaledCommand*>(other);

    if (m_newAutoScaled == cmd->m_oldAutoScaled) {
        m_newAutoScaled = cmd->m_newAutoScaled;
        refreshText();

        return true;
    }

    return false;
}

void ImagesetChangeAutoScaledCommand::undo()
{
    commands::UndoCommand::undo();

    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_autoScaled = m_oldAutoScaled;

    int index = m_visual->m_dockWidget->m_autoScaled->findText(imagesetEntry->m_autoScaled);
    m_visual->m_dockWidget->m_autoScaled->setCurrentIndex(index);
}

void ImagesetChangeAutoScaledCommand::redo()
{
    auto imagesetEntry = m_visual->m_imagesetEntry;
    imagesetEntry->m_autoScaled = m_newAutoScaled;

    int index = m_visual->m_dockWidget->m_autoScaled->findText(imagesetEntry->m_autoScaled);
    m_visual->m_dockWidget->m_autoScaled->setCurrentIndex(index);

    commands::UndoCommand::redo();
}

/////

DuplicateCommand::DuplicateCommand(visual::VisualEditing *visual_, const QStringList &newNames, const QMap<QString, QPointF> &newPositions, const QMap<QString, QRectF> &newRects, const QMap<QString, QPointF> &newOffsets)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_newNames = newNames;
    m_newPositions = newPositions;
    m_newRects = newRects;
    m_newOffsets = newOffsets;

    if (m_newNames.length() == 1)
        setText("Duplicate image");
    else
        setText(QString("Duplicate %1 images").arg(m_newNames.length()));
}

void DuplicateCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString imageName : m_newNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        m_visual->m_imagesetEntry->m_imageEntries.removeOne(image);

        image->m_listItem->m_imageEntry = nullptr;
        image->m_listItem = nullptr;

        image->setParentItem(nullptr);
        m_visual->scene()->removeItem(image);

        delete image;
    }

    m_visual->m_dockWidget->refresh();
}

void DuplicateCommand::redo()
{
    for (QString imageName : m_newNames) {
        auto image = new elements::ImageEntry(m_visual->m_imagesetEntry);
        m_visual->m_imagesetEntry->m_imageEntries.append(image);

        image->set_name(imageName);
        image->setPos(m_newPositions[imageName]);
        image->setRect(m_newRects[imageName]);
        image->m_offset->setPos(m_newOffsets[imageName]);
    }

    m_visual->m_dockWidget->refresh();

    commands::UndoCommand::redo();
}

/////

PasteCommand::PasteCommand(visual::VisualEditing *visual_, const QStringList &newNames, const QMap<QString, QPointF> &newPositions, const QMap<QString, QRectF> &newRects, const QMap<QString, QPointF> &newOffsets)
    : commands::UndoCommand()
{
    m_visual = visual_;

    m_newNames = newNames;
    m_newPositions = newPositions;
    m_newRects = newRects;
    m_newOffsets = newOffsets;

    refreshText();
}

void PasteCommand::refreshText()
{
    if (m_newNames.length() == 1)
        setText("Paste image");
    else
        setText(QString("Paste %1 images").arg(m_newNames.length()));
}

void PasteCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString imageName : m_newNames) {
        ImageEntry* image = m_visual->m_imagesetEntry->getImageEntry(imageName);
        m_visual->m_imagesetEntry->m_imageEntries.removeOne(image);;

        image->m_listItem->m_imageEntry = nullptr;
        image->m_listItem = nullptr;

        image->setParentItem(nullptr);
        m_visual->scene()->removeItem(image);

        delete image;
    }

    m_visual->m_dockWidget->refresh();
}

void PasteCommand::redo()
{
    for (QString imageName : m_newNames) {
        auto image = new elements::ImageEntry(m_visual->m_imagesetEntry);
        m_visual->m_imagesetEntry->m_imageEntries.append(image);

        image->set_name(imageName);
        image->setPos(m_newPositions[imageName]);
        image->setRect(m_newRects[imageName]);
        image->m_offset->setPos(m_newOffsets[imageName]);
    }

    m_visual->m_dockWidget->refresh();

    commands::UndoCommand::redo();
}



} // namespace undo
} // namespace imageset
} // editors
} // CEED
