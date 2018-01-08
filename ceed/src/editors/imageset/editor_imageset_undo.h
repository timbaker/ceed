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

#ifndef CEED_editors_imageset_undo_
#define CEED_editors_imageset_undo_

#include "CEEDBase.h"

#include "cegui/ceguitypes.h"

#include "commands.h"
#include "editor_imageset_elements.h"

using CEED::editors::imageset::elements::ImageEntry;

namespace CEED {
namespace editors {
namespace imageset {
namespace undo {

const int idbase = 1100;

// undo commands in this file intentionally use Qt's primitives because
// it's easier to work with and avoids unnecessary conversions all the time
// you should however always use the ImageEntry's properties (xpos, ypos, ...)!

/*!
\brief MoveCommand

This command simply moves given images from old position to the new
    You can use GeometryChangeCommand instead and use the same rects as old new as current rects,
    this is there just to save memory.

*/
class MoveCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_imageNames;
    QMap<QString, QPointF> m_oldPositions;
    QMap<QString, QPointF> m_newPositions;
    int m_biggestDelta;

    MoveCommand(visual::VisualEditing* visual_, const QStringList& imageNames,
                const QMap<QString, QPointF>& oldPositions, const QMap<QString, QPointF>& newPositions);

    void refreshText();

    int id() const override
    {
        return idbase + 1;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief GeometryChangeCommand

Changes geometry of given images, that means that positions as well as rects might change
    Can even implement MoveCommand as a special case but would eat more RAM.

*/
class GeometryChangeCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_imageNames;
    QMap<QString, QPointF> m_oldPositions;
    QMap<QString, QRectF> m_oldRects;
    QMap<QString, QPointF> m_newPositions;
    QMap<QString, QRectF> m_newRects;
    qreal m_biggestMoveDelta;
    qreal m_biggestResizeDelta;

    GeometryChangeCommand(visual::VisualEditing* visual_, const QStringList& imageNames,
                          const QMap<QString, QPointF>& oldPositions,
                          const QMap<QString, QRectF>& oldRects,
                          const QMap<QString, QPointF>& newPositions,
                          const QMap<QString, QRectF>& newRects);

    void refreshText();

    int id() const override
    {
        return idbase + 2;
    }

    bool mergeWith(const QUndoCommand* cmd_);

    void undo() override;

    void redo() override;
};

class OffsetMoveCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_imageNames;
    QMap<QString, QPointF> m_oldPositions;
    QMap<QString, QPointF> m_newPositions;
    qreal m_biggestDelta;

    OffsetMoveCommand(visual::VisualEditing* visual_, const QStringList& imageNames,
                      const QMap<QString, QPointF> & oldPositions, const QMap<QString, QPointF> & newPositions);

    void refreshText();

    int id() const override
    {
        return idbase + 3;
    }

    bool mergeWith(const QUndoCommand *cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief RenameCommand

Changes name of one image (always just one image!)

*/
class RenameCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_oldName;
    QString m_newName;

    RenameCommand(visual::VisualEditing* visual_, const QString& oldName, const QString& newName);

    void refreshText();

    int id() const override
    {
        return idbase + 4;
    }

    bool mergeWith(const QUndoCommand *cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief PropertyEditCommand

Changes one property of the image.

    We do this separately from Move, OffsetMove, etc commands because we want to
    always merge in this case.

*/
class PropertyEditCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_imageName;
    QString m_propertyName;
    QVariant m_oldValue;
    QVariant m_newValue;

    PropertyEditCommand(visual::VisualEditing* visual_, const QString& imageName, const QString& propertyName,
                        const QVariant &oldValue, const QVariant &newValue);

    void refreshText();

    int id() const override
    {
        return idbase + 5;
    }

    bool mergeWith(const QUndoCommand *cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief CreateCommand

Creates one image with given parameters

*/
class CreateCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_name;
    int m_xpos;
    int m_ypos;
    int m_width;
    int m_height;
    int m_xoffset;
    int m_yoffset;

    CreateCommand(visual::VisualEditing* visual_, const QString& name,
                  int xpos, int ypos, int width, int height, int xoffset, int yoffset);

    int id() const override
    {
        return idbase + 6;
    }

    void undo() override;

    void redo() override;
};

/*!
\brief DeleteCommand

Deletes given image entries

*/
class DeleteCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_imageNames;
    QMap<QString, QPointF> m_oldPositions;
    QMap<QString, QRectF> m_oldRects;
    QMap<QString, QPointF> m_oldOffsets;

    DeleteCommand(visual::VisualEditing* visual_,
                  const QStringList& imageNames,
                  const QMap<QString, QPointF>& oldPositions,
                  const QMap<QString, QRectF>& oldRects,
                  const QMap<QString, QPointF>& oldOffsets);

    int id() const override
    {
        return idbase + 7;
    }

    void undo() override;

    void redo() override;
};

/*!
\brief ImagesetRenameCommand

Changes name of the imageset

*/
class ImagesetRenameCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_oldName;
    QString m_newName;

    ImagesetRenameCommand(visual::VisualEditing* visual_, const QString& oldName, const QString& newName);

    void refreshText();

    int id() const override
    {
        return idbase + 8;
    }

    bool mergeWith(const QUndoCommand *cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief ImagesetChangeImageCommand

Changes the underlying image of the imageset

*/
class ImagesetChangeImageCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_oldImageFile;
    QString m_newImageFile;

    ImagesetChangeImageCommand(visual::VisualEditing* visual_, const QString& oldImageFile, const QString& newImageFile);

    void refreshText();

    int id() const override
    {
        return idbase + 9;
    }

    bool mergeWith(const QUndoCommand *cmd_) override;

    void undo() override;

    void redo() override;
};

/*!
\brief ImagesetChangeNativeResolutionCommand

Changes native resolution of the imageset

*/
class ImagesetChangeNativeResolutionCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    int m_oldHorzRes;
    int m_oldVertRes;
    int m_newHorzRes;
    int m_newVertRes;

    ImagesetChangeNativeResolutionCommand(visual::VisualEditing* visual_, int oldHorzRes, int oldVertRes, int newHorzRes, int newVertRes);

    void refreshText();

    int id() const override
    {
        return idbase + 10;
    }

    bool mergeWith(const QUndoCommand *other) override;

    void undo() override;

    void redo() override;
};

/*!
\brief ImagesetChangeAutoScaledCommand

Changes auto scaled value

*/
class ImagesetChangeAutoScaledCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_oldAutoScaled;
    QString m_newAutoScaled;

    ImagesetChangeAutoScaledCommand(visual::VisualEditing *visual_, const QString& oldAutoScaled, const QString& newAutoScaled);

    void refreshText();

    int id() const override
    {
        return idbase + 11;
    }

    bool mergeWith(const QUndoCommand *other) override;

    void undo() override;

    void redo() override;
};

/*!
\brief DuplicateCommand

Duplicates given image entries

*/
class DuplicateCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_newNames;
    QMap<QString, QPointF> m_newPositions;
    QMap<QString, QRectF> m_newRects;
    QMap<QString, QPointF> m_newOffsets;

    DuplicateCommand(visual::VisualEditing* visual_,
                     const QStringList& newNames,
                     const QMap<QString, QPointF>& newPositions,
                     const QMap<QString, QRectF>& newRects,
                     const QMap<QString, QPointF>& newOffsets);

    int id() const override
    {
        return idbase + 12;
    }

    void undo() override;

    void redo() override;
};

/*!
\brief PasteCommand

This command pastes clipboard data to the given imageset.
    Based on DuplicateCommand.

*/
class PasteCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_newNames;
    QMap<QString, QPointF> m_newPositions;
    QMap<QString, QRectF> m_newRects;
    QMap<QString, QPointF> m_newOffsets;

    PasteCommand(visual::VisualEditing* visual_,
                 const QStringList& newNames,
                 const QMap<QString, QPointF>& newPositions,
                 const QMap<QString, QRectF>& newRects,
                 const QMap<QString, QPointF>& newOffsets);

    void refreshText();

    int id() const override
    {
        return idbase + 13;
    }

    void undo() override;

    void redo() override;
};

} // namespace undo
} // namespace imageset
} // editors
} // CEED

#endif
