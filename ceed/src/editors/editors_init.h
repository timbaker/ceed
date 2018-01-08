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

#ifndef CEED_editors___init___
#define CEED_editors___init___

#include "CEEDBase.h"

/**This module contains interfaces needed to run editors tabs (multi-file editing)

Also groups all the editors together to avoid cluttering the root directory.
*/

#include <QDialog>

class Ui_NoTypeDetectedDialog;
class Ui_MultipleTypesDetectedDialog;
class Ui_MultiplePossibleFactoriesDialog;

class QFileSystemWatcher;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QMenu;
class QTabWidget;
class QUndoStack;

namespace CEED {
namespace editors {

class TabbedEditorFactory;

class NoTypeDetectedDialog : public QDialog
{
public:
    Ui_NoTypeDetectedDialog* m_ui;
    QListWidget* m_typeChoice;

    NoTypeDetectedDialog(compatibility::Manager* compatibilityManager);
};

class MultipleTypesDetectedDialog : public QDialog
{
public:
    Ui_MultipleTypesDetectedDialog* m_ui;
    QListWidget* m_typeChoice;

    MultipleTypesDetectedDialog(compatibility::Manager* compatibilityManager, const QStringList& possibleTypes);
};

class MultiplePossibleFactoriesDialog : public QDialog
{
public:
    Ui_MultiplePossibleFactoriesDialog *m_ui;
    QListWidget* m_factoryChoice;
    QMap<QListWidgetItem*, TabbedEditorFactory*> m_itemToFactory;

    MultiplePossibleFactoriesDialog(const QList<TabbedEditorFactory*>& possibleFactories);
    TabbedEditorFactory* selectedFactory();
};

/*!
\brief TabbedEditor

This is the base class for a class that takes a file and allows manipulation
    with it. It occupies exactly 1 tab space.

*/
class TabbedEditor : public QObject
{
    Q_OBJECT
public:
    compatibility::Manager* m_compatibilityManager;
    QString m_desiredSavingDataType;
    QString m_nativeData;
    bool m_requiresProject;
    bool m_initialised;
    bool m_active;
    mainwindow::MainWindow* m_mainWindow;
    QString m_filePath;
    QWidget* m_tabWidget;
    QString m_tabLabel;
    QFileSystemWatcher* m_fileMonitor;
    bool m_fileChangedByExternalProgram;
    bool m_displayingReloadAlert;

    /**Constructs the editor.

    compatibilityManager - manager that should be used to transform data between
                           various data types using compatibility layers
    filePath - absolute file path of the file that should be opened
    */
    TabbedEditor(compatibility::Manager* compatibilityManager, const QString& filePath);

public slots:
    void fileChangedByExternalProgram();

public:
    /**Pops a alert menu open asking the user s/he would like to reload the
        file due to external changes being made*/
    void askForFileReload();

    /**This method loads everything up so this editor is ready to be switched to*/
    virtual void initialise(mainwindow::MainWindow* mainWindow);

    /**Cleans up after itself
    this is usually called when you want the tab closed
    */
    virtual void finalise()
    {
        Q_ASSERT(m_initialised);
        Q_ASSERT(m_tabWidget);

        m_initialised = false;
    }

    /**Reinitialises this tabbed editor, effectivelly reloading the file
    off the hard drive again
    */
    virtual void reinitialise();

    /**Removes itself from the tab list and irrevocably destroys
    data associated with itself
    */
    virtual void destroy();

    /**Returns the editorMenu for this editor, or None.
    The editorMenu is shared across editors so this returns
    None if this editor is not the active editor.
    */
    QMenu* editorMenu();

    /**The main window has one special menu on its menubar (editorMenu)
    whose items are updated dynamically to match the currently active
    editor. Implement this method if you want to use this menu for an editor.
    Returns two booleans: Visible, Enabled
    */
    virtual void rebuildEditorMenu(QMenu* editorMenu, bool& visible, bool& enabled)
    {
        Q_UNUSED(editorMenu)
        visible = false;
        enabled = false;
    }

    /**The tab gets "on stage", it's been clicked on and is now the only active
    tab. There can be either 0 tabs active (blank screen) or exactly 1 tab
    active.
    */
    virtual void activate();

    /**The tab gets "off stage", user switched to another tab.
    This is also called when user closes the tab (deactivate and then finalise
    is called).
    */
    virtual void deactivate();

    /**Makes this tab editor current (= the selected tab)*/
    virtual void makeCurrent();

    /**Checks whether this TabbedEditor contains changes
    (= it should be saved before closing it)*/
    virtual bool hasChanges()
    {
        return false;
    }

    /**Marks that this tabbed editor has changes, in this implementation this means
    that the tab in the tab list gets an icon
    */
    void markHasChanges(bool hasChanges);

    /**Causes the tabbed editor to save all it's progress to the file.
    targetPath should be absolute file path.
    */
    virtual bool saveAs(const QString& targetPath, bool updateCurrentPath = true);

    /**Adds a file monitor to the specified file so CEED will alert the
    user that an external change happened to the file*/
    void addFileMonitor(const QString& path);

    /**Removes the file monitor from the specified path*/
    void removeFileMonitor(const QString& path);

    /**Saves all progress to the same file we have opened at the moment
    */
    virtual bool save()
    {
        return saveAs(m_filePath);
    }

    /**Causes the tabbed editor to discard all it's progress*/
    virtual void discardChanges()
    {
        // early out
        if (!hasChanges()) {
            return;
        }

        // the default but kind of wasteful implementation
        reinitialise();

        // the state of the tabbed editor should be valid at this point
    }

    /**Called by the mainwindow whenever undo is requested*/
    virtual void undo()
    {
    }

    /**Called by the mainwindow whenever redo is requested*/
    virtual void redo()
    {
    }

    /**Called by the mainwindow whenever revert is requested

    Revert simply goes back to the state of the file on disk,
    disregarding any changes made since then
    */
    void revert()
    {
        discardChanges();
    }

    /**Returns UndoStack or None is the tabbed editor doesn't have undo stacks.
    This is useful for QUndoView

    Note: If you use UndoStack in your tabbed editor, inherit from UndoStackTabbedEditor
          below, it will save you a lot of work (synchronising all the actions and their texts, etc..)
    */
    virtual QUndoStack* getUndoStack()
    {
        return nullptr;
    }

    /**Returns current desired saving data type. Data type that will be used when user requests to save this file
    */
    QString getDesiredSavingDataType()
    {
        return m_compatibilityManager ? m_desiredSavingDataType : QString();
    }

    /**Performs copy of the editor's selection to the clipboard.

    Default implementation doesn't do anything.

    Returns: true if the operation was successful
    */
    virtual bool performCopy()
    {
        return false;
    }

    /**Performs cut of the editor's selection to the clipboard.

    Default implementation doesn't do anything.

    Returns: true if the operation was successful
    */
    virtual bool performCut()
    {
        return false;
    }

    /**Performs paste from the clipboard to the editor.

    Default implementation doesn't do anything

    Returns: true if the operation was successful
    */
    virtual bool performPaste()
    {
        return false;
    }

    /**Deletes the selected items in the editor.

    Default implementation doesn't do anything

    Returns: true if the operation was successful
    */
    virtual bool performDelete()
    {
        return false;
    }

    /**Called by the mainwindow whenever zoom is requested*/
    virtual bool zoomIn()
    {
        return false;
    }

    /**Called by the mainwindow whenever zoom is requested*/
    virtual bool zoomOut()
    {
        return false;
    }

    /**Called by the mainwindow whenever zoom is requested*/
    virtual bool zoomReset()
    {
        return false;
    }
};

/*!
\brief UndoStackTabbedEditor

Used for tabbed editors that have one shared undo stack. This saves a lot
    of boilerplate code for undo/redo action synchronisation and the undo/redo itself

*/
class UndoStackTabbedEditor : public TabbedEditor
{
    Q_OBJECT
public:
    QUndoStack* m_undoStack;

    UndoStackTabbedEditor(compatibility::Manager* compatibilityManager, const QString& filePath);

    virtual void initialise(mainwindow::MainWindow* mainWindow) override;

    void activate() override;

    bool hasChanges() override;

    bool saveAs(const QString& targetPath, bool updateCurrentPath = true) override;

    void discardChanges() override;

    void undo() override;

    void redo() override;

    QUndoStack* getUndoStack() override
    {
        return m_undoStack;
    }

private slots:
    void slot_undoAvailable(bool available);

    void slot_redoAvailable(bool available);

    void slot_undoTextChanged(const QString& text);

    void slot_redoTextChanged(const QString& text);

    void slot_cleanChanged(bool clean);
};

/*!
\brief TabbedEditorFactory

Constructs instances of TabbedEditor (multiple instances of one TabbedEditor
    can coexist - user editing 2 layouts for example - with the ability to switch
    from one to another)

*/
class TabbedEditorFactory
{
public:
    virtual QString getName() = 0;

    /**Returns a set of file extensions (without prefix dots) that can be edited by this factory*/
    virtual QSet<QString> getFileExtensions() = 0;

    /**This checks whether instance created by this factory can edit given file*/
    virtual bool canEditFile(const QString& filePath) = 0;

    /**Creates the respective TabbedEditor instance

    This should only be called with a filePath the factory reported
    as editable by the instances
    */
    virtual TabbedEditor* create(const QString& filePath) = 0;

    // note: destroy doesn't really make sense as python is reference counted
    //       and everything is garbage collected
};

/*!
\brief MessageTabbedEditor

This is basically a stub tabbed editor, it simply displays a message
    and doesn't allow any sort of editing at all, all functionality is stubbed

    This is for internal use only so there is no factory for this particular editor

*/
class MessageTabbedEditor : public TabbedEditor
{
public:
    QLabel* m_label;

    MessageTabbedEditor(const QString& filePath, const QString& message);

    bool hasChanges() override
    {
        return false;
    }
};

//from ceed import settings

} // namespace editors
} // namespace CEED

#endif
