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

#ifndef CEED_filesystembrowser_
#define CEED_filesystembrowser_

#include "CEEDBase.h"

#include <QDockWidget>

class Ui_FileSystemBrowser;
class QListView;
class QFileSystemModel;
class QToolButton;
class QComboBox;

namespace CEED {

namespace mainwindow {
class MainWindow;
}

namespace filesystembrowser {

/*!
\brief FileSystemBrowser

This class represents the file system browser dock widget, usually located right bottom
    in the main window. It can browse your entire filesystem and if you double click a file
    it will open an editor tab for it.

*/
class FileSystemBrowser : public QDockWidget
{
    Q_OBJECT
public:
    Ui_FileSystemBrowser* m_ui;
    QListView* m_view;
    QFileSystemModel* m_model;
    QToolButton* m_parentDirectoryButton;
    QToolButton* m_homeDirectoryButton;
    QToolButton* m_projectDirectoryButton;
    QToolButton* m_activeFileDirectoryButton;
    QComboBox* m_pathBox;
    QString m_directory;

    FileSystemBrowser();

    void setDirectory(const QString& directory);

signals:
    void fileOpenRequested(const QString &path);

public slots:
    /** Slot that gets triggered whenever user double clicks anything
    in the filesystem view
    */
    void itemDoubleClicked(const QModelIndex& modelIndex);

    /** Slot that gets triggered whenever the "Parent Directory" button gets pressed */
    void parentDirectoryButton();

    void homeDirectoryButton();

    void projectDirectoryButton();

    void activeFileDirectoryButton();

    /** Slot that gets triggered whenever the user selects an path from the list
    or enters a new path and hits enter */
    void pathBoxIndexChanged(int index);
};

} // namespace filesystembrowser
} // namespace CEED

#endif
