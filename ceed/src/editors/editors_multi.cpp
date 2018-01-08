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

#include "editors_multi.h"

namespace CEED {
namespace editors {
namespace multi {

ModeSwitchCommand::ModeSwitchCommand(MultiModeTabbedEditor *tabbedEditor, int oldTabIndex, int newTabIndex)
    : commands::UndoCommand()
{
    m_tabbedEditor = tabbedEditor;

    m_oldTabIndex = oldTabIndex;
    m_newTabIndex = newTabIndex;

    // we never every merge edit mode changes, no need to define this as refreshText
    setText(QString("Change edit mode to '%1'").arg(m_tabbedEditor->tabText(newTabIndex)));
}

void ModeSwitchCommand::undo()
{
    commands::UndoCommand::undo();

    m_tabbedEditor->m_ignoreCurrentChangedForUndo = true;
    m_tabbedEditor->setCurrentIndex(m_oldTabIndex);
    m_tabbedEditor->m_ignoreCurrentChangedForUndo = false;
}

void ModeSwitchCommand::redo()
{
    // to avoid multiple event firing
    if (m_tabbedEditor->currentIndex() != m_newTabIndex) {
        m_tabbedEditor->m_ignoreCurrentChangedForUndo = true;
        m_tabbedEditor->setCurrentIndex(m_newTabIndex);
        m_tabbedEditor->m_ignoreCurrentChangedForUndo = false;
    }
    commands::UndoCommand::redo();
}

/////

CodeEditModeCommand::CodeEditModeCommand(CodeEditMode *codeEditing, const QString &oldText, const QString &newText, int totalChange)
    :  commands::UndoCommand()
{
    m_codeEditing = codeEditing;
    m_oldText = oldText;
    m_newText = newText;

    m_totalChange = totalChange;

    m_dryRun = true;

    refreshText();
}

void CodeEditModeCommand::refreshText()
{
    if (m_totalChange == 1)
        setText("Code edit, changed 1 character");
    else
        setText(QString("Code edit, changed %1 characters").arg(m_totalChange));
}

bool CodeEditModeCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const CodeEditModeCommand*>(cmd_);

    Q_ASSERT(m_codeEditing == cmd->m_codeEditing);

    // TODO: 10 chars for now for testing
    if (m_totalChange + cmd->m_totalChange < 10) {
        m_totalChange += cmd->m_totalChange;
        m_newText = cmd->m_newText;

        refreshText();

        return true;
    }

    return false;
}

void CodeEditModeCommand::undo()
{
    commands::UndoCommand::undo();

    m_codeEditing->m_ignoreUndoCommands = true;
    m_codeEditing->setPlainText(m_oldText);
    m_codeEditing->m_ignoreUndoCommands = false;
}

void CodeEditModeCommand::redo()
{
    if (!m_dryRun) {
        m_codeEditing->m_ignoreUndoCommands = true;
        m_codeEditing->setPlainText(m_newText);
        m_codeEditing->m_ignoreUndoCommands = false;
    }

    m_dryRun = false;

    commands::UndoCommand::redo();
}

/////

CodeEditMode::CodeEditMode()
    : QTextEdit()
    , EditMode()
{
    m_ignoreUndoCommands = false;
    m_lastUndoText = "";

    document()->setUndoRedoEnabled(false);
    connect(document(), &QTextDocument::contentsChange, this, &CodeEditMode::slot_contentsChange);
}

void CodeEditMode::refreshFromVisual()
{
    QString source = getNativeCode();

    m_ignoreUndoCommands = true;
    setPlainText(source);
    m_ignoreUndoCommands = false;
}

bool CodeEditMode::propagateToVisual()
{
    QString source = document()->toPlainText();

    // for some reason, Qt calls hideEvent even though the tab widget was never shown :-/
    // in this case the source will be empty and parsing it will fail
    if (source.isEmpty())
        return true;

    return propagateNativeCode(source);
}

void CodeEditMode::activate()
{
    EditMode::activate();
    refreshFromVisual();
}

bool CodeEditMode::deactivate()
{
    bool changesAccepted = propagateToVisual();
    bool ret = changesAccepted;

    if (!changesAccepted) {
        // the file contains more than just CR LF
        int result = QMessageBox::question(this,
                                           "Parsing the Code changes failed!",
                                           "Parsing of the changes done in Code edit mode failed, the result couldn't be accepted.\n"
                                           "Press Cancel to stay in the Code edit mode to correct the mistake(s) or press Discard to "
                                           "discard the changes and go back to the previous state (before you entered the code edit mode).",
                                           QMessageBox::Cancel | QMessageBox::Discard, QMessageBox::Cancel);

        if (result == QMessageBox::Cancel)
            // return false to indicate we don't want to switch out of this widget
            ret = false;

        else if (result == QMessageBox::Discard)
            // we return true, the visual element wasn't touched (the error is thrown before that)
            ret = true;
    }
    return ret && EditMode::deactivate();
}

void CodeEditMode::slot_contentsChange(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position)

    if (!m_ignoreUndoCommands) {
        int totalChange = charsRemoved + charsAdded;

        auto cmd = new CodeEditModeCommand(this, m_lastUndoText, toPlainText(), totalChange);
        getTabbedEditor()->m_undoStack->push(cmd);
    }

    m_lastUndoText = toPlainText();
}

/////

MultiModeTabbedEditor::MultiModeTabbedEditor(compatibility::Manager *compatibilityManager, const QString &filePath)
    : editors::UndoStackTabbedEditor(compatibilityManager, filePath)
    , QTabWidget()
{
    setTabPosition(QTabWidget::South);
    setTabShape(QTabWidget::Triangular);

    // Qt's moc can't handle multiple inheritance with > 1 base class derived from QObject
    QObject::connect(this, &MultiModeTabbedEditor::currentChanged, [=](int index){ slot_currentChanged(index); });

    // will be -1, that means no tabs are selected
    m_currentTabIndex = currentIndex();
    // when canceling tab transfer we have to switch back and avoid unnecessary deactivate/activate cycle
    m_ignoreCurrentChanged = false;
    // to avoid unnecessary undo command pushes we ignore currentChanged if we are
    // inside ModeChangeCommand.undo or redo
    m_ignoreCurrentChangedForUndo = false;
}

void MultiModeTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    editors::UndoStackTabbedEditor::initialise(mainWindow);

    // the emitted signal only contains the new signal, we have to keep track
    // of the current index ourselves so that we know which one is the "old" one
    // for the undo command

    // by this time tabs should have been added so this should not be -1
    m_currentTabIndex = currentIndex();
    Q_ASSERT(m_currentTabIndex != -1);
}

void MultiModeTabbedEditor::slot_currentChanged(int newTabIndex)
{
    if (m_ignoreCurrentChanged)
        return;

    QWidget* oldTab = widget(m_currentTabIndex);
    QWidget* newTab = widget(newTabIndex);

    if (auto editMode = dynamic_cast<EditMode*>(oldTab)) {
        if (!editMode->deactivate()) {
            m_ignoreCurrentChanged = true;
            setCurrentWidget(oldTab);
            m_ignoreCurrentChanged = false;
            return;
        }
    }

    if (auto editMode = dynamic_cast<EditMode*>(newTab)) {
        editMode->activate();
    }

    if (!m_ignoreCurrentChangedForUndo) {
        auto cmd = new ModeSwitchCommand(this, m_currentTabIndex, newTabIndex);
        m_undoStack->push(cmd);
    }

    m_currentTabIndex = newTabIndex;
}


} // namespace multi
} // namespace editors
} // namespace CEED
