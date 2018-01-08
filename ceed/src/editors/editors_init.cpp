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

#include "editors/editors_init.h"

#include "action/declaration.h"
#include "compatibility/compatibility_init.h"
#include "commands.h"
#include "mainwindow.h"
#include "project.h"

#include "ui_NoTypeDetectedDialog.h"
#include "ui_MultipleTypesDetectedDialog.h"
#include "ui_MultiplePossibleFactoriesDialog.h"

#include "ceed_paths.h"

#include <QFileSystemWatcher>
#include <QMenu>
#include <QMessageBox>
#include <QUndoStack>

namespace CEED {
namespace editors {

NoTypeDetectedDialog::NoTypeDetectedDialog(compatibility::Manager *compatibilityManager)
    : QDialog()
{
    m_ui = new Ui_NoTypeDetectedDialog();
    m_ui->setupUi(this);

    m_typeChoice = m_ui->typeChoice;

    for (QString& type_ : compatibilityManager->getKnownTypes()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(type_);

        // TODO: We should give a better feedback about what's compatible with what
        item->setToolTip(tr("Compatible with CEGUI: %1").arg(compatibilityManager->getCEGUIVersionsCompatibleWithType(type_).join(", ")));

        m_typeChoice->addItem(item);
    }
}

MultipleTypesDetectedDialog::MultipleTypesDetectedDialog(compatibility::Manager *compatibilityManager, const QStringList &possibleTypes)
    : QDialog()
{
    m_ui = new Ui_MultipleTypesDetectedDialog();
    m_ui->setupUi(this);

    m_typeChoice = m_ui->typeChoice;

    for (QString& type_ : compatibilityManager->getKnownTypes()) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(type_);

        if (possibleTypes.contains(type_)) {
            QFont font;
            font.setBold(true);
            item->setFont(font);
        }

        // TODO: We should give a better feedback about what's compatible with what
        item->setToolTip(QString("Compatible with CEGUI: %1").arg(compatibilityManager->getCEGUIVersionsCompatibleWithType(type_).join(", ")));

        m_typeChoice->addItem(item);
    }
}

MultiplePossibleFactoriesDialog::MultiplePossibleFactoriesDialog(const QList<TabbedEditorFactory *> &possibleFactories)
    : QDialog()
{
    m_ui = new Ui_MultiplePossibleFactoriesDialog();
    m_ui->setupUi(this);

    m_factoryChoice = m_ui->factoryChoice;// findChild(QListWidget, "factoryChoice")

    for (TabbedEditorFactory* factory : possibleFactories) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(factory->getName());
//        item->setData(Qt::UserRole, factory);
        m_itemToFactory[item] = factory;
        m_factoryChoice->addItem(item);
    }
}

TabbedEditorFactory *MultiplePossibleFactoriesDialog::selectedFactory()
{
    auto selection = m_factoryChoice->selectedItems();
    if (selection.length() == 1) {
        return m_itemToFactory[selection[0]];
    }
    return nullptr;
}

TabbedEditor::TabbedEditor(compatibility::Manager *compatibilityManager, const QString &filePath)
{
    m_compatibilityManager = compatibilityManager;
    m_desiredSavingDataType = m_compatibilityManager ? m_compatibilityManager->EditorNativeType : "";
    //        m_nativeData = None;

    m_requiresProject = false;

    m_initialised = false;
    m_active = false;

    m_mainWindow = nullptr;

    m_filePath = os.path.normpath(filePath);

    m_tabWidget = nullptr;
    m_tabLabel = os.path.basename(m_filePath);

    //Set up a QFileSystemWatcher to watch for external changes to a file
    m_fileMonitor = nullptr;
    m_fileChangedByExternalProgram = false;
    m_displayingReloadAlert = false;
}

void TabbedEditor::fileChangedByExternalProgram()
{
    /**The callback method for external file changes.  This method is
        immediately called when this tab is open.  Otherwise, it's called when
        the user clicks on the tab*/
    m_fileChangedByExternalProgram = true;
    if (m_active)
        askForFileReload();
}

void TabbedEditor::askForFileReload()
{
    // If multiple file writes are made by an external program, multiple
    // pop-ups will appear. Prevent that with a switch.
    if (!m_displayingReloadAlert) {
        m_displayingReloadAlert = true;
        int ret = QMessageBox::question(m_mainWindow,
                                        "File has been modified externally!",
                                        "The file that you have currently opened has been modified outside the CEGUI Unified Editor.\n\nReload the file?\n\nIf you select Yes, ALL UNDO HISTORY WILL BE DESTROYED!",
                                        QMessageBox::No | QMessageBox::Yes,
                                        QMessageBox::No); // defaulting to No is safer IMO

        m_fileChangedByExternalProgram = false;

        if (ret == QMessageBox::Yes)
            reinitialise();

        else if (ret == QMessageBox::No) {
            // FIXME: We should somehow make CEED think that we have changes :-/
            //        That is hard to do because CEED relies on QUndoStack to get that info

        } else {
            // how did we get here?
            Q_ASSERT(false);
        }
        m_displayingReloadAlert = false;
    }
}

void TabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    Q_ASSERT(m_initialised == false);
    Q_ASSERT(m_tabWidget);

    m_mainWindow = mainWindow;
#if 0 // FIXME?
    m_tabWidget->m_tabbedEditor = this;
#endif

    m_mainWindow->m_tabEditors << this; // FIXME: because there is no common class for m_tabWidget (other than QWidget) I added this to assocate editors with widgets
    m_mainWindow->m_tabs->addTab(m_tabWidget, m_tabLabel);

    if (m_compatibilityManager != nullptr) {
        QFile file(m_filePath);
        if (!file.open(QFile::ReadOnly | QFile::Text))
            throw IOError(QString("QFile::open failed with file \"%1\"").arg(m_filePath));
        QByteArray data = file.readAll();
        file.close();
        QString rawData = QString::fromUtf8(data);
        QString rawDataType;
        bool nativeDataOK = true;

        if (rawData.isEmpty()) {
            // it's an empty new file, the derived classes deal with this separately
            m_nativeData = rawData;
            nativeDataOK = false;

            if (mainWindow->m_project == nullptr) {
                m_desiredSavingDataType = m_compatibilityManager->EditorNativeType;
            } else {
                m_desiredSavingDataType = m_compatibilityManager->getSuitableDataTypeForCEGUIVersion(mainWindow->m_project->m_CEGUIVersion);
            }
        } else {
            try {
                rawDataType = m_compatibilityManager->guessType(rawData, m_filePath);

                // A file exists and the editor has it open, so watch it for
                // external changes.
                addFileMonitor(m_filePath);

            } catch (compatibility::NoPossibleTypesError e) {
                NoTypeDetectedDialog dialog(m_compatibilityManager);
                int result = dialog.exec();

                rawDataType = m_compatibilityManager->EditorNativeType;
                nativeDataOK = false;

                if (result == QDialog::Accepted) {
                    QList<QListWidgetItem*> selection = dialog.m_typeChoice->selectedItems();

                    if (selection.length() == 1) {
                        rawDataType = selection[0]->text();
                        nativeDataOK = true;
                    }
                }

            } catch (compatibility::MultiplePossibleTypesError e) {
                // if no project is opened or if the opened file was detected as something not suitable for the target CEGUI version of the project
                if ((mainWindow->m_project == nullptr) ||
                        !e.m_possibleTypes.contains( m_compatibilityManager->getSuitableDataTypeForCEGUIVersion(mainWindow->m_project->m_CEGUIVersion) )) {
                    MultipleTypesDetectedDialog dialog(m_compatibilityManager, e.m_possibleTypes);
                    int result = dialog.exec();

                    rawDataType = m_compatibilityManager->EditorNativeType;
                    nativeDataOK = false;

                    if (result == QDialog::Accepted) {
                        QList<QListWidgetItem*> selection = dialog.m_typeChoice->selectedItems();

                        if (selection.length() == 1) {
                            rawDataType = selection[0]->text();
                            nativeDataOK = true;
                        }
                    }

                } else {
                    rawDataType = m_compatibilityManager->getSuitableDataTypeForCEGUIVersion(mainWindow->m_project->m_CEGUIVersion);
                }
            }

            // by default, save in the same format as we opened in
            m_desiredSavingDataType = rawDataType;

            if (mainWindow->m_project != nullptr) {
                QString projectCompatibleDataType = m_compatibilityManager->CEGUIVersionTypes[mainWindow->m_project->m_CEGUIVersion];

                if (projectCompatibleDataType != rawDataType) {
                    if (QMessageBox::question(mainWindow,
                                              "Convert to format suitable for opened project?",
                                              QString("File you are opening isn't suitable for the project that is opened at the moment.\n"
                                                      "Do you want to convert it to a suitable format upon saving?\n"
                                                      "(from '%1' to '%1')\n"
                                                      "Data COULD be lost, make a backup!)").arg(rawDataType).arg(projectCompatibleDataType),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
                        m_desiredSavingDataType = projectCompatibleDataType;
                    }
                }
            }

            // if nativeData is "" at this point, data type was not successful and user didn't select
            // any data type as well so we will just use given file as an empty file

            if (nativeDataOK) {
                try {
                    m_nativeData = m_compatibilityManager->transform(rawDataType, m_compatibilityManager->EditorNativeType, rawData);

                } catch (compatibility::LayerNotFoundError e) {
                    // TODO: Dialog, can't convert
                    m_nativeData.clear();
                }
            }
        }
    }

    m_initialised = true;
}

void TabbedEditor::reinitialise()
{
    bool wasCurrent = m_mainWindow->m_activeEditor == this;

    auto* mainWindow = m_mainWindow;
    finalise();
    initialise(mainWindow);

    if (wasCurrent)
        makeCurrent();
}

void TabbedEditor::destroy()
{
    int i = 0;
    QWidget* wdt = m_mainWindow->m_tabs->widget(i);
    bool tabRemoved = false;

    while (wdt != nullptr) {
        if (wdt == m_tabWidget) {
            m_mainWindow->m_tabs->removeTab(i);
            tabRemoved = true;
            break;
        }

        i = i + 1;
        wdt = m_mainWindow->m_tabs->widget(i);
    }

    Q_ASSERT(tabRemoved);
}

QMenu *TabbedEditor::editorMenu()
{
    return m_active ? m_mainWindow->m_editorMenu : nullptr;
}

void TabbedEditor::activate()
{
    TabbedEditor* currentActive = m_mainWindow->m_activeEditor;

    // no need to deactivate and then activate again
    if (currentActive == this)
        return;

    if (currentActive != nullptr)
        currentActive->deactivate();

    m_active = true;

    m_mainWindow->m_activeEditor = this;
    m_mainWindow->m_undoViewer->setUndoStack(getUndoStack());

    // If the file was changed by an external program, ask the user to reload
    // the changes
    if (m_fileMonitor != nullptr && m_fileChangedByExternalProgram)
        askForFileReload();

    QMenu* edMenu = m_mainWindow->m_editorMenu;
    edMenu->clear();
    bool visible, enabled;
    rebuildEditorMenu(edMenu, visible, enabled);
    edMenu->menuAction()->setVisible(visible);
    edMenu->menuAction()->setEnabled(enabled);
}

void TabbedEditor::deactivate()
{
    m_active = false;

    if (m_mainWindow->m_activeEditor == this) {
        m_mainWindow->m_activeEditor = nullptr;
        QMenu* edMenu = m_mainWindow->m_editorMenu;
        edMenu->clear();
        edMenu->menuAction()->setEnabled(false);
        edMenu->menuAction()->setVisible(false);
    }
}

void TabbedEditor::makeCurrent()
{
    // (this should automatically handle the respective deactivate and activate calls)
    m_mainWindow->m_tabs->setCurrentWidget(m_tabWidget);
}

void TabbedEditor::markHasChanges(bool hasChanges)
{
    if (m_mainWindow == nullptr)
        return;

    if (hasChanges)
        m_mainWindow->m_tabs->setTabIcon(m_mainWindow->m_tabs->indexOf(m_tabWidget), QIcon(":icons/tabs/has_changes.png"));
    else
        m_mainWindow->m_tabs->setTabIcon(m_mainWindow->m_tabs->indexOf(m_tabWidget), QIcon());
}

bool TabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    QString outputData = m_nativeData;
    if (m_compatibilityManager != nullptr)
        outputData = m_compatibilityManager->transform(m_compatibilityManager->EditorNativeType, m_desiredSavingDataType, m_nativeData);

    // Stop monitoring the file, the changes that are about to occur are not
    // picked up as being from an external program!
    removeFileMonitor(m_filePath);

    try {
        QFile f(targetPath);
        if (!f.open(QFile::WriteOnly | QFile::Text))
            throw IOError(QString("QFile::open failed for file \"%1\"").arg(targetPath));
        f.write(outputData.toUtf8());
        f.close();

    } catch (IOError e) {
        // The rest of the code is skipped, so be sure to turn file
        // monitoring back on
        addFileMonitor(m_filePath);
        QMessageBox::critical(nullptr, "Error saving file!",
                              QString("CEED encountered an error trying to save the file.\n\n(exception details: %1)").arg(QLatin1String(e.what())));
        return false;
    }

    if (updateCurrentPath) {
        // changes current path to the path we saved to
        m_filePath = targetPath;

        // update tab text
        m_tabLabel = os.path.basename(m_filePath);

        // because this might be called even before initialise is called!
        if (m_mainWindow != nullptr)
            m_mainWindow->m_tabs->setTabText(m_mainWindow->m_tabs->indexOf(m_tabWidget), m_tabLabel);
    }

    addFileMonitor(m_filePath);
    return true;
}

void TabbedEditor::addFileMonitor(const QString &path)
{
    if (m_fileMonitor == nullptr) {
        m_fileMonitor = new QFileSystemWatcher(m_mainWindow);
        QObject::connect(m_fileMonitor, &QFileSystemWatcher::fileChanged, this, &TabbedEditor::fileChangedByExternalProgram);
    }
    m_fileMonitor->addPath(path);
}

void TabbedEditor::removeFileMonitor(const QString &path)
{
    if (m_fileMonitor != nullptr) // FIXME: Will it ever be None at this point?
        m_fileMonitor->removePath(path);
}

/////

UndoStackTabbedEditor::UndoStackTabbedEditor(compatibility::Manager *compatibilityManager, const QString &filePath)
    : TabbedEditor(compatibilityManager, filePath)
{
    m_undoStack = new QUndoStack();

    m_undoStack->setUndoLimit(settings::getEntry("global/undo/limit")->m_value.toInt());
    m_undoStack->setClean();

    QObject::connect(m_undoStack, &QUndoStack::canUndoChanged, this, &UndoStackTabbedEditor::slot_undoAvailable);
    QObject::connect(m_undoStack, &QUndoStack::canRedoChanged, this, &UndoStackTabbedEditor::slot_redoAvailable);

    QObject::connect(m_undoStack, &QUndoStack::undoTextChanged, this, &UndoStackTabbedEditor::slot_undoTextChanged);
    QObject::connect(m_undoStack, &QUndoStack::redoTextChanged, this, &UndoStackTabbedEditor::slot_redoTextChanged);

    QObject::connect(m_undoStack, &QUndoStack::cleanChanged, this, &UndoStackTabbedEditor::slot_cleanChanged);
}

void UndoStackTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    TabbedEditor::initialise(mainWindow);
    m_undoStack->clear();
}

void UndoStackTabbedEditor::activate()
{
    TabbedEditor::activate();

    QUndoStack* undoStack = getUndoStack();

    m_mainWindow->m_undoAction->setEnabled(undoStack->canUndo());
    m_mainWindow->m_redoAction->setEnabled(undoStack->canRedo());

    m_mainWindow->m_undoAction->setText(QString("Undo %1").arg(undoStack->undoText()));
    m_mainWindow->m_redoAction->setText(QString("Redo %1").arg(undoStack->redoText()));

    m_mainWindow->m_saveAction->setEnabled(!undoStack->isClean());
}

bool UndoStackTabbedEditor::hasChanges()
{
    return !m_undoStack->isClean();
}

bool UndoStackTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    if (TabbedEditor::saveAs(targetPath, updateCurrentPath)) {
        m_undoStack->setClean();
        return true;
    }

    return false;
}

void UndoStackTabbedEditor::discardChanges()
{
    // since we have undo stack, we can simply use it instead of the slower
    // reinitialisation approach

    auto undoStack = getUndoStack();
    int cleanIndex = undoStack->cleanIndex();

    if (cleanIndex == -1) {
        // it is possible that we can't undo/redo to cleanIndex
        // this can happen because of undo limitations (at most 100 commands
        // and we need > 100 steps)
        TabbedEditor::discardChanges();

    } else {
        undoStack->setIndex(cleanIndex);
    }
}

void UndoStackTabbedEditor::undo()
{
    getUndoStack()->undo();
}

void UndoStackTabbedEditor::redo()
{
    getUndoStack()->redo();
}

void UndoStackTabbedEditor::slot_undoAvailable(bool available)
{
    // conditional because this might be called even before initialise is called!
    if (m_mainWindow != nullptr)
        m_mainWindow->m_undoAction->setEnabled(available);
}

void UndoStackTabbedEditor::slot_redoAvailable(bool available)
{
    // conditional because this might be called even before initialise is called!
    if (m_mainWindow != nullptr)
        m_mainWindow->m_redoAction->setEnabled(available);
}

void UndoStackTabbedEditor::slot_undoTextChanged(const QString &text)
{
    // conditional because this might be called even before initialise is called!
    if (m_mainWindow != nullptr)
        m_mainWindow->m_undoAction->setText(QString("Undo %1").arg(text));
}

void UndoStackTabbedEditor::slot_redoTextChanged(const QString &text)
{
    // conditional because this might be called even before initialise is called!
    if (m_mainWindow != nullptr)
        m_mainWindow->m_redoAction->setText(QString("Redo %1").arg(text));
}

void UndoStackTabbedEditor::slot_cleanChanged(bool clean)
{
    // clean means that the undo stack is at a state where it's in sync with the underlying file
    // we set the undostack as clean usually when saving the file so we will assume that there
    if (m_mainWindow != nullptr)
        m_mainWindow->m_saveAction->setEnabled(!clean);

    markHasChanges(!clean);
}

/////

MessageTabbedEditor::MessageTabbedEditor(const QString &filePath, const QString &message)
    : TabbedEditor(nullptr, filePath)
{
    m_label = new QLabel(message);
    m_label->setWordWrap(true);
    auto widget = new QScrollArea();
    m_tabWidget = widget;
    widget->setWidget(m_label);
}


} // namespace editors
} // namespace CEED
