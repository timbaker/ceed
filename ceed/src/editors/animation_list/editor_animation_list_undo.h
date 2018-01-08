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

#ifndef CEED_editors_animation_list_undo_
#define CEED_editors_animation_list_undo_

#include "CEEDBase.h"

#include "commands.h"

#include "editor_animation_list_timeline.h"

namespace CEED {
namespace editors {
namespace animation_list {
namespace undo {

const int idbase = 1300;

/*!
\brief ChangeCurrentAnimationDefinitionCommand

Changes currently edited animation definition.

    We have to make this an undo command to be sure that the context for other
    undo commands is always right.

*/
class ChangeCurrentAnimationDefinitionCommand : public commands::UndoCommand
{
    typedef commands::UndoCommand super;
public:
    visual::VisualEditing* m_visual;
    QString m_newName;
    QString m_oldName;

    ChangeCurrentAnimationDefinitionCommand(visual::VisualEditing* visual_, const QString& newName, const QString& oldName);

    void refreshText()
    {
        setText(QString("Now editing '%1'").arg(m_newName));
    }

    int id() const override
    {
        return idbase + 1;
    }

    bool mergeWith(const QUndoCommand* cmd) override;

    void undo() override;

    void redo() override;
};

/*!
\brief MoveKeyFramesCommand

Moves gives key frames to given positions

*/
class MoveKeyFramesCommand : public commands::UndoCommand
{
    typedef commands::UndoCommand super;
public:
    visual::VisualEditing* m_visual;
    QList<timeline::KeyFrameMoved> m_movedKeyFrames;
    bool m_dryRun;

    MoveKeyFramesCommand(visual::VisualEditing* visual_, const QList<timeline::KeyFrameMoved>& movedKeyFrames);

    void refreshText();

    int id() const override
    {
        return idbase + 2;
    }

    bool mergeWith(const QUndoCommand* cmd_) override
    {
        Q_UNUSED(cmd_)
        return false;
    }

    void undo() override;

    void redo() override;
};

} // namespace undo
} // namespace animation_list
} // namespace editors
} // namespace CEED

#endif
