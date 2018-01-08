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

#ifndef CEED_editors_layout_undo_
#define CEED_editors_layout_undo_

#include "CEEDBase.h"

#include "commands.h"

#include "cegui/ceguitypes.h"
#include "editor_layout_visual.h"
#include "editor_layout_widgethelpers.h"

namespace CEED {
namespace editors {
namespace layout {
namespace undo {

const int idbase = 1200;

/*!
\brief MoveCommand

This command simply moves given widgets from old positions to new

*/
class MoveCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    QMap<QString, CEGUI::UVector2> m_oldPositions;
    QMap<QString, CEGUI::UVector2> m_newPositions;

    MoveCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                const QMap<QString, CEGUI::UVector2>& oldPositions, const QMap<QString, CEGUI::UVector2>& newPositions):
        commands::UndoCommand()
    {
        m_visual = visual;

        m_widgetPaths = widgetPaths;
        m_oldPositions = oldPositions;
        m_newPositions = newPositions;
    }

    virtual void postConstruct()
    {
        refreshText();
    }

    virtual void refreshText()
    {
        if (m_widgetPaths.length() == 1) {
            setText(QString("Move '%1'").arg(m_widgetPaths[0]));
        } else {
            setText(QString("Move %1 widgets").arg(m_widgetPaths.length()));
        }
    }

    int id() const override
    {
        return idbase + 1;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};


/*!
\brief ResizeCommand

This command resizes given widgets from old positions and old sizes to new

*/
class ResizeCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    QMap<QString, CEGUI::UVector2> m_oldPositions;
    QMap<QString, CEGUI::USize> m_oldSizes;
    QMap<QString, CEGUI::UVector2> m_newPositions;
    QMap<QString, CEGUI::USize> m_newSizes;

    ResizeCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                  const QMap<QString, CEGUI::UVector2>& oldPositions,
                  const QMap<QString, CEGUI::USize>& oldSizes,
                  const QMap<QString, CEGUI::UVector2>& newPositions,
                  const QMap<QString, CEGUI::USize>& newSizes);

    virtual void postConstruct();

    virtual void refreshText();

    int id() const override
    {
        return idbase + 2;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};


/*!
\brief DeleteCommand

This command deletes given widgets
*/
class DeleteCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    QMap<QString, widgethelpers::SerialisationData*> m_widgetData;

    DeleteCommand(visual::VisualEditing* visual, const QStringList& widgetPaths);

    void refreshText();

    int id() const override
    {
        return idbase + 3;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        // we never merge deletes
        return false;
    }

    void undo() override;

    void redo() override;
};


/*!
\brief CreateCommand

This command creates one widget
*/
class CreateCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_parentWidgetPath;
    QString m_widgetType;
    QString m_widgetName;

    CreateCommand(visual::VisualEditing* visual, const QString& parentWidgetPath, const QString& widgetType, const QString& widgetName):
        commands::UndoCommand()
    {
        m_visual = visual;

        m_parentWidgetPath = parentWidgetPath;
        m_widgetType = widgetType;
        m_widgetName = widgetName;

        refreshText();
    }

    void refreshText()
    {
        setText(QString("Create '%1' of type '%2'").arg(m_widgetName).arg(m_widgetType));
    }

    int id() const override
    {
        return idbase + 4;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        // we never merge creates
        return false;
    }

    void undo() override;

    void redo() override;
};


/*!
\brief PropertyEditCommand

This command resizes given widgets from old positions and old sizes to new

*/
class PropertyEditCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_propertyName;
    QStringList m_widgetPaths;
    QMap<QString, CEED::Variant> m_oldValues;
    CEED::Variant m_newValue;
    bool m_ignoreNextPropertyManagerCallback;

   PropertyEditCommand(visual::VisualEditing* visual, const QString& propertyName, const QStringList& widgetPaths,
                       const QMap<QString, CEED::Variant>& oldValues, const CEED::Variant& newValue, bool ignoreNextPropertyManagerCallback = false);

    void refreshText();

    int id() const override
    {
        return idbase + 5;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void notifyPropertyManager(widgethelpers::Manipulator* widgetManipulator, bool ignoreTarget);

    void undo() override;

    void redo() override;
};


/*!
\brief HorizontalAlignCommand

This command aligns selected widgets accordingly

*/
class HorizontalAlignCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    QList<CEGUI::HorizontalAlignment> m_oldAlignments;
    CEGUI::HorizontalAlignment m_newAlignment;

    HorizontalAlignCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                           const QList<CEGUI::HorizontalAlignment>& oldAlignments, CEGUI::HorizontalAlignment newAlignment)
        : commands::UndoCommand()
    {
        m_visual = visual;

        m_widgetPaths = widgetPaths;
        m_oldAlignments = oldAlignments;
        m_newAlignment = newAlignment;

        refreshText();
    }

    void refreshText();

    int id() const override
    {
        return idbase + 6;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        auto cmd = static_cast<const HorizontalAlignCommand*>(cmd_);

        if (m_widgetPaths == cmd->m_widgetPaths) {
            m_newAlignment = cmd->m_newAlignment;
            refreshText();

            return true;
        }

        return false;
    }

    void undo() override;

    void redo() override;
};


/*!
\brief VerticalAlignCommand

This command aligns selected widgets accordingly

*/
class VerticalAlignCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    QList<CEGUI::VerticalAlignment> m_oldAlignments;
    CEGUI::VerticalAlignment m_newAlignment;

    VerticalAlignCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                         QList<CEGUI::VerticalAlignment>& oldAlignments, CEGUI::VerticalAlignment newAlignment):
        commands::UndoCommand()
    {
        m_visual = visual;

        m_widgetPaths = widgetPaths;
        m_oldAlignments = oldAlignments;
        m_newAlignment = newAlignment;

        refreshText();
    }

    void refreshText();

    int id() const override
    {
        return idbase + 7;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        auto cmd = static_cast<const VerticalAlignCommand*>(cmd_);

        if (m_widgetPaths == cmd->m_widgetPaths) {
            m_newAlignment = cmd->m_newAlignment;
            refreshText();

            return true;
        }

        return false;
    }

    void undo() override;

    void redo() override;
};


/*!
\brief ReparentCommand

This command changes parent of given windows

*/
class ReparentCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_oldWidgetPaths;
    QStringList m_newWidgetPaths;

    ReparentCommand(visual::VisualEditing* visual, const QStringList& oldWidgetPaths, const QStringList& newWidgetPaths):
        commands::UndoCommand()
    {
        m_visual = visual;

        m_oldWidgetPaths = oldWidgetPaths;
        m_newWidgetPaths = newWidgetPaths;

        refreshText();
    }

    void refreshText();

    int id() const override
    {
        return idbase + 8;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};


/*!
\brief PasteCommand

This command pastes clipboard data to the given widget

*/
class PasteCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_targetWidgetPath;
    QList<widgethelpers::SerialisationData*> m_clipboardData;

    PasteCommand(visual::VisualEditing* visual, const QList<widgethelpers::SerialisationData*>& clipboardData, const QString& targetWidgetPath)
        : commands::UndoCommand()
    {
        m_visual = visual;

        m_clipboardData = clipboardData;
        m_targetWidgetPath = targetWidgetPath;

        refreshText();
    }

    void refreshText();

    int id() const override
    {
        return idbase + 9;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        // we never merge paste commands
        return false;
    }

    void undo() override;

    void redo() override;
};


class NormaliseSizeCommand : public ResizeCommand
{
public:
    NormaliseSizeCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                         const QMap<QString, CEGUI::UVector2>& oldPositions,
                         const QMap<QString, CEGUI::USize>& oldSizes);

    virtual CEGUI::USize normaliseSize(const QString& widgetPath) = 0;

    void postConstruct() override;

    int id() const override
    {
        throw NotImplementedError("Each subclass of NormaliseSizeCommand must implement the id method");
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        // we never merge size normalising commands
        return false;
    }
};


class NormaliseSizeToRelativeCommand : public NormaliseSizeCommand
{
public:
    NormaliseSizeToRelativeCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths,
                                   const QMap<QString, CEGUI::UVector2>& oldPositions,
                                   const QMap<QString, CEGUI::USize>& oldSizes);

    CEGUI::USize normaliseSize(const QString &widgetPath) override;

    void refreshText() override;

    int id() const override
    {
        return idbase + 10;
    }
};


class NormaliseSizeToAbsoluteCommand : public NormaliseSizeCommand
{
public:
    NormaliseSizeToAbsoluteCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths,
                 const QMap<QString, CEGUI::UVector2>& oldPositions,
                 const QMap<QString, CEGUI::USize>& oldSizes);

    CEGUI::USize normaliseSize(const QString& widgetPath);

    void refreshText();

    int id() const override
    {
        return idbase + 11;
    }
};


class NormalisePositionCommand : public MoveCommand
{
public:
    NormalisePositionCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                             const QMap<QString, CEGUI::UVector2>& oldPositions);

    virtual CEGUI::UVector2 normalisePosition(const QString& widgetPath, const CEGUI::UVector2& oldPosition) = 0;

    void postConstruct() override;

    int id() const override = 0;

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        // we never merge position normalising commands
        // FIXME: id()==-1 in this case?
        return false;
    }
};


class NormalisePositionToRelativeCommand : public NormalisePositionCommand
{
public:
    NormalisePositionToRelativeCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths,
                                       const QMap<QString, CEGUI::UVector2>& oldPositions);

    CEGUI::UVector2 normalisePosition(const QString &widgetPath, const CEGUI::UVector2& position) override;

    void refreshText();

    int id() const override
    {
        return idbase + 12;
    }
};


class NormalisePositionToAbsoluteCommand : public NormalisePositionCommand
{
public:
    NormalisePositionToAbsoluteCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths,
                 const QMap<QString, CEGUI::UVector2>& oldPositions);

    CEGUI::UVector2 normalisePosition(const QString &widgetPath, const CEGUI::UVector2 &position) override;

    void refreshText();

    int id() const override
    {
        return idbase + 13;
    }
};


/*!
\brief RenameCommand

This command changes the name of the given widget

*/
class RenameCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QString m_oldWidgetPath;
    QString m_oldWidgetName;
    QString m_newWidgetPath;
    QString m_newWidgetName;

    RenameCommand(visual::VisualEditing* visual_, const QString &oldWidgetPath, const QString& newWidgetName);

    void refreshText();

    int id() const override
    {
        return idbase + 14;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};


class RoundPositionCommand : public MoveCommand
{
public:
    RoundPositionCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths,
                         const QMap<QString, CEGUI::UVector2>& oldPositions);

    /*static */CEGUI::UVector2 roundAbsolutePosition(const CEGUI::UVector2 &oldPosition);

    void postConstruct() override;

    void refreshText() override;

    int id() const override
    {
        return idbase + 15;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        auto cmd = static_cast<const RoundPositionCommand*>(cmd_);

        // merge if the new round position command will apply to the same widget
        if (m_widgetPaths == cmd->m_widgetPaths) {
            return true;
        } else {
            return false;
        }
    }
};


class RoundSizeCommand : public ResizeCommand
{
public:
    RoundSizeCommand(visual::VisualEditing* visual, const QStringList& widgetPaths,
                     const QMap<QString, CEGUI::UVector2>& oldPositions,
                     QMap<QString, CEGUI::USize>& oldSizes);

    /*static */CEGUI::USize roundAbsoluteSize(const CEGUI::USize& oldSize);

    void postConstruct() override;

    void refreshText();

    int id() const override
    {
        return idbase + 16;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;
};


class MoveInParentWidgetListCommand : public commands::UndoCommand
{
public:
    visual::VisualEditing* m_visual;
    QStringList m_widgetPaths;
    int m_delta;

    MoveInParentWidgetListCommand(visual::VisualEditing* visual_, const QStringList& widgetPaths, int delta);

    void refreshText();

    int id() const override
    {
        return idbase + 17;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};

} // namespace undo
} // namespace layout
} // namespace editors
} // namespace CEED

#endif
