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

#include "editor_animation_list_undo.h"

#include "editors/animation_list/editor_animation_list_visual.h"

namespace CEED {
namespace editors {
namespace animation_list {
namespace undo {

ChangeCurrentAnimationDefinitionCommand::ChangeCurrentAnimationDefinitionCommand(visual::VisualEditing *visual_, const QString &newName, const QString &oldName)
    : super()
{
    m_visual = visual_;

    m_newName = newName;
    m_oldName = oldName;

    refreshText();
}

bool ChangeCurrentAnimationDefinitionCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const ChangeCurrentAnimationDefinitionCommand*>(cmd_);
    m_newName = cmd->m_newName;
    refreshText();
    return true;
}

void ChangeCurrentAnimationDefinitionCommand::undo()
{
    super::undo();

    if (m_oldName.isEmpty())
        m_visual->setCurrentAnimation(nullptr);
    else
        m_visual->setCurrentAnimationWrapper(m_visual->getAnimationWrapper(m_oldName));
}

void ChangeCurrentAnimationDefinitionCommand::redo()
{
    if (m_newName.isEmpty())
        m_visual->setCurrentAnimation(nullptr);
    else
        m_visual->setCurrentAnimationWrapper(m_visual->getAnimationWrapper(m_newName));

    super::redo();
}

/////

MoveKeyFramesCommand::MoveKeyFramesCommand(visual::VisualEditing *visual_, const QList<timeline::KeyFrameMoved> &movedKeyFrames)
    : commands::UndoCommand()
{
    m_visual = visual_;
    m_movedKeyFrames = movedKeyFrames;

    // the next redo will be skipped
    m_dryRun = true;

    refreshText();
}

void MoveKeyFramesCommand::refreshText()
{
    setText(QString("Moved '%1' keyframe%2")
            .arg(m_movedKeyFrames.length())
            .arg((m_movedKeyFrames.length() > 1) ? QStringLiteral("s") : QStringLiteral("")));
}

void MoveKeyFramesCommand::undo()
{
    super::undo();

    QList<timeline::KeyFrameMoved> newMovedKeyFrames;
    for (auto& movedKeyFrame : m_movedKeyFrames) {
        auto* keyFrame = m_visual->getKeyFrameOfCurrentAnimation(movedKeyFrame.affectorIndex, movedKeyFrame.keyFrameIndex);
        keyFrame->moveToPosition(movedKeyFrame.oldPosition);

        newMovedKeyFrames += { (int)keyFrame->getIdxInParent(), movedKeyFrame.affectorIndex, movedKeyFrame.oldPosition, movedKeyFrame.newPosition };
    }

    m_movedKeyFrames = newMovedKeyFrames;

    m_visual->m_timelineDockWidget->m_timeline->refresh();
}

void MoveKeyFramesCommand::redo()
{
    if (m_dryRun) {
        m_dryRun = false;
        return;
    }

    QList<timeline::KeyFrameMoved> newMovedKeyFrames;
    for (auto& movedKeyFrame : m_movedKeyFrames) {
        auto keyFrame = m_visual->getKeyFrameOfCurrentAnimation(movedKeyFrame.affectorIndex, movedKeyFrame.keyFrameIndex);
        keyFrame->moveToPosition(movedKeyFrame.newPosition);

        newMovedKeyFrames += { (int)keyFrame->getIdxInParent(), movedKeyFrame.affectorIndex, movedKeyFrame.oldPosition, movedKeyFrame.newPosition };
    }
    m_movedKeyFrames = newMovedKeyFrames;

    m_visual->m_timelineDockWidget->m_timeline->refresh();
}


} // namespace undo
} // namespace animation_list
} // namespace editors
} // namespace CEED
