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

#include "filesystembrowser.h"

#include "mainwindow.h"
#include "project.h"
#include "editors/editors_init.h"

#include "ui_FileSystemBrowser.h"

#include <QtCore>
#include <QFileSystemModel>
#include <QtGui>

namespace CEED {
namespace filesystembrowser {

FileSystemBrowser::FileSystemBrowser()
    : QDockWidget()
{
    m_ui = new Ui_FileSystemBrowser();
    m_ui->setupUi(this);

    m_view = m_ui->view;
    m_model = new QFileSystemModel();
    // causes way too many problems
    //      self.model.setReadOnly(false)
    m_view->setModel(m_model);

    connect(m_view, &QListView::doubleClicked, this, &FileSystemBrowser::itemDoubleClicked);

    m_parentDirectoryButton = m_ui->parentDirectoryButton;
    connect(m_parentDirectoryButton, &QToolButton::pressed, this, &FileSystemBrowser::parentDirectoryButton);

    m_homeDirectoryButton = m_ui->homeDirectoryButton;
    connect(m_homeDirectoryButton, &QToolButton::pressed, this, &FileSystemBrowser::homeDirectoryButton);

    m_projectDirectoryButton = m_ui->projectDirectoryButton;
    connect(m_projectDirectoryButton, &QToolButton::pressed, this, &FileSystemBrowser::projectDirectoryButton);

    m_activeFileDirectoryButton = m_ui->activeFileDirectoryButton;
    connect(m_activeFileDirectoryButton, &QToolButton::pressed, this, &FileSystemBrowser::activeFileDirectoryButton);

    m_pathBox = m_ui->pathBox;
    connect(m_pathBox, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &FileSystemBrowser::pathBoxIndexChanged);

    m_directory = "";

    // Set to project directory if project open, otherwise to user's home
    if (mainwindow::MainWindow::instance->m_project != nullptr)
        setDirectory(mainwindow::MainWindow::instance->m_project->getAbsolutePathOf(""));
    else
        setDirectory(QDir("~").absolutePath());
}

void FileSystemBrowser::setDirectory(const QString &directory)
{
    /** Sets the browser to view given directory */

    QDir dir(directory);
    if (!dir.exists())
        return;

    QString directory_ = dir.absolutePath();
    m_model->setRootPath(directory_);
    m_view->setRootIndex(m_model->index(directory_));

    // Add the path to pathBox and select it
    //
    // If the path already exists in the pathBox, remove it and
    // add it to the top.
    // Path comparisons are done case-sensitive because there's
    // no true way to tell if the path is on a case-sensitive
    // file system or not, apart from creating a (temp) file
    // on that file system (and this can't be done once at start-up
    // because the user may have and use multiple file systems).
    int existingIndex = m_pathBox->findText(directory);
    m_pathBox->blockSignals(true);
    if (existingIndex != -1) {
        m_pathBox->removeItem(existingIndex);
    }
    m_pathBox->insertItem(0, directory);
    m_pathBox->setCurrentIndex(0);
    m_pathBox->blockSignals(false);

    m_directory = directory;
}

void FileSystemBrowser::itemDoubleClicked(const QModelIndex &modelIndex)
{
    QString childPath = modelIndex.data().toString();
    QString absolutePath = QDir(m_directory).absoluteFilePath(childPath); // os.path.normpath(os.path.join(self.directory, childPath))

    if (QDir(absolutePath).exists())
        setDirectory(absolutePath);
    else
        emit fileOpenRequested(absolutePath);
}

void FileSystemBrowser::parentDirectoryButton()
{
    setDirectory(QFileInfo(m_directory).absolutePath());
}

void FileSystemBrowser::homeDirectoryButton()
{
    setDirectory(QDir("~").absolutePath());
}

void FileSystemBrowser::projectDirectoryButton()
{
    if (mainwindow::MainWindow::instance->m_project != nullptr)
        setDirectory(mainwindow::MainWindow::instance->m_project->getAbsolutePathOf(""));
}

void FileSystemBrowser::activeFileDirectoryButton()
{
    if (mainwindow::MainWindow::instance->m_activeEditor != nullptr) {
        QString filePath = mainwindow::MainWindow::instance->m_activeEditor->m_filePath;
        QString dirPath = QFileInfo(filePath).absolutePath();
        setDirectory(dirPath);
        // select the active file
        QModelIndex modelIndex = m_model->index(filePath);
        if (modelIndex.isValid()) {
            m_view->setCurrentIndex(modelIndex);
        }
    }
}

void FileSystemBrowser::pathBoxIndexChanged(int index)
{
    if (index != -1) {
        // Normally this should be a simple:
        //   self.setDirectory(self.pathBox.currentText())
        // However, when the user edits the text and hits enter, their text
        // is automatically appended to the list of items and this signal
        // is fired. This is fine except that the text may not be a valid
        // directory (typo) and then the pathBox becomes poluted with junk
        // entries.
        // To solve all this, we get the new text, remove the item and then
        // call set directory which will validate and then add the path
        // to the list.
        // The alternative would be to prevent the editted text from being
        // automatically inserted (InsertPolicy(QComboBox::NoInsert)) but
        // then we need custom keyPress handling to detect the enter key
        // press etc (editTextChanged is fired on every keyPress!).
        QString newPath = m_pathBox->currentText();
        m_pathBox->blockSignals(true);
        m_pathBox->removeItem(index);
        m_pathBox->blockSignals(false);
        setDirectory(newPath);
    }
}


} // namespace filesystembrowser
} // namespace CEED
