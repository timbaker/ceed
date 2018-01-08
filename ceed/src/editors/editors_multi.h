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

#ifndef CEED_editors_multi_
#define CEED_editors_multi_

#include "CEEDBase.h"

/*
 This module contains interfaces for multi mode tabbed editors (visual, source, ...)
*/

#include "editors/editors_init.h"

#include "commands.h"

#include <QTabWidget>
#include <QTextEdit>
#include <QMessageBox>

namespace CEED {
namespace editors {
namespace multi {

class CodeEditMode;
class MultiModeTabbedEditor;

/*!
\brief ModeSwitchCommand

Undo command that is pushed to the undo stack whenever user switches edit modes.

    Switching edit mode has to be an undoable command because the other commands might
    or might not make sense if user is not in the right mode.

    This has a drawback that switching to Live Preview (layout editing) and back is
    undoable even though you can't affect the document in any way whilst in Live Preview
    mode.

*/
class ModeSwitchCommand : public commands::UndoCommand
{
public:
    MultiModeTabbedEditor* m_tabbedEditor;
    int m_oldTabIndex;
    int m_newTabIndex;

    ModeSwitchCommand(MultiModeTabbedEditor* tabbedEditor, int oldTabIndex, int newTabIndex);

    void undo() override;

    void redo() override;
};

/*!
\brief EditMode

Interface class for the edit mode widgets (more a mixin class)
    This practically just ensures that the inherited classes have activate and deactivate
    methods.

*/
class EditMode
{
public:
    EditMode()
    {
    }

    /**This is called whenever this edit mode is activated (user clicked on the tab button
    representing it). It's not called when user switches from a different file tab whilst
    this tab was already active when user was switching from this file tab to another one!

    Activation can't be canceled and must always happen when requested!
    */
    virtual void activate()
    {
    }

    /**This is called whenever this edit mode is deactivated (user clicked on another tab button
    representing another mode). It's not called when user switches to this file tab from another
    file tab and this edit mode was already active before user switched from the file tab to another
    file tab.

    if this returns true, all went well
    if this returns false, the action is terminated and the current edit mode stays in place
    */
    virtual bool deactivate()
    {
        return true;
    }
};

/*!
\brief CodeEditModeCommand


    Undo command for code edit mode.

    TODO: Extremely memory hungry implementation for now, I have to figure out how to use my own
    QUndoStack with QTextDocument in the future to fix this.

*/
class CodeEditModeCommand : public commands::UndoCommand
{
public:
    CodeEditMode* m_codeEditing;
    QString m_oldText;
    QString m_newText;
    int m_totalChange;
    bool m_dryRun;

    CodeEditModeCommand(CodeEditMode* codeEditing, const QString& oldText, const QString& newText, int totalChange);

    void refreshText();

    int id() const override
    {
        return 1000 + 1;
    }

    bool mergeWith(const QUndoCommand* cmd_) override;

    void undo() override;

    void redo() override;
};

class CodeEditMode : public QTextEdit, public EditMode
{
public:
    /**This is the most used alternative editing mode that allows you to edit raw code.

    Raw code is mostly XML in CEGUI formats but can be anything else in a generic sense
    */

    // TODO: Some highlighting and other aids

    bool m_ignoreUndoCommands;
    QString m_lastUndoText;

    CodeEditMode();

    /**Returns native source code from your editor implementation.*/
    virtual QString getNativeCode() = 0;

    /**Synchronizes your editor implementation with given native source code.

    Returns true if changes were accepted (the code was valid, etc...)
    Returns false if changes weren't accepted (invalid code most likely)
    */
    virtual bool propagateNativeCode(const QString& code) = 0;

    /**Refreshes this Code editing mode with current native source code.*/
    virtual void refreshFromVisual();

    /**Propagates source code from this Code editing mode to your editor implementation.

    Returns true if changes were accepted (the code was valid, etc...)
    Returns false if changes weren't accepted (invalid code most likely)*/
    virtual bool propagateToVisual();

    void activate() override;

    bool deactivate() override;

    virtual editors::UndoStackTabbedEditor* getTabbedEditor() = 0;

public slots:
    void slot_contentsChange(int position, int charsRemoved, int charsAdded);
};

/**This class represents tabbed editor that has little tabs on the bottom
allowing you to switch editing "modes" - visual, code, ...

Not all modes have to be able to edit! Switching modes pushes undo actions
onto the UndoStack to avoid confusion when undoing. These actions never merge
together.

You yourself are responsible for putting new tabs into this widget!
You should not add/remove modes after the construction!
*/
class MultiModeTabbedEditor : public editors::UndoStackTabbedEditor, public QTabWidget
{
public:
    int m_currentTabIndex;
    bool m_ignoreCurrentChanged;
    bool m_ignoreCurrentChangedForUndo;

    MultiModeTabbedEditor(compatibility::Manager* compatibilityManager, const QString& filePath);

    virtual void initialise(mainwindow::MainWindow* mainWindow) override;

public slots:
    void slot_currentChanged(int newTabIndex = -1);
};

} // namespace multi
} // namespace editors
} // namespace CEED

#endif
