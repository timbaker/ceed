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

#ifndef CEED_project_
#define CEED_project_

#include "CEEDBase.h"

/** Implements project file data model and GUI for interacting with it
*/

#include "compatibility/compatibility_init.h"
#include "compatibility/project/compat_project_init.h"
#include "propertymapping.h"
#include "qtwidgets.h"
#include "xmledit.h"

#include "elementtree.h"

#include "ui_ProjectManager.h"
#include "ui_NewProjectDialog.h"
#include "ui_ProjectSettingsDialog.h"

#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QInputDialog>
#include <QStandardItem>

namespace CEED {
namespace project {

using qtwidgets::FileLineEdit;

// TODO: Should probably be moved somewhere else because it's reusable
QString convertToPortablePath(const QString& inputPath);

class Project;

/*!
\brief Item

One item in the project
    This is usually a file or a folder

*/
class Item : public QStandardItem
{
public:
    static const int Unknown = 1;
    // A file is an item that can't have any children, it is directly opened instead of
    // being expanded/collapsed like folders
    static const int File = 2;
    // Folder is a group of files. Project folders don't necessarily have to have
    // a counterpart on the HDD, they could be virtual.
    static const int Folder = 3;
#if 0
    itemType = property(lambda self: data(Qt::UserRole + 1),
                        lambda value: setItemType(value))

    label = property(lambda self: text(),
                     lambda value: setText(value))

    icon = property(lambda self: icon(),
                    lambda value: setIcon(QIcon(value)))

    // following properties are only applicable to files

    // path is the path relative to project's base dir
    path = property(lambda self: getPath(),
                    lambda value: setPath(value))
    // only applicable to folders
    name = property(lambda self: getName(),
                    lambda value: setName(value))
#endif

    Project* m_project;

    Item(Project* project);

    int type() const override;

    QStandardItem* clone() const override;

    void setItemType(int value);
    int getItemType() const { return data(Qt::UserRole + 1).toInt(); }

    void setLabel(const QString& label) { setText(label); }
    QString getLabel() const { return text(); }

    void setIcon(const QString& path) { QStandardItem::setIcon(QIcon(path)); }

    void setPath(const QString& value);
    QString getPath() const;

    void setName(const QString& value);
    QString getName() const;

    QString getRelativePath();

    QString getAbsolutePath();

    static Item *loadFromElement(Project *project, ElementTree::Element* element);

    ElementTree::Element* saveToElement();

    /** Checks whether given absolute path is referenced by this Item
        or any of its descendants */
    bool referencesFilePath(const QString& filePath);
};


/*!
\brief Project

This class encapsulates a project edited by the editor

    A project is basically a set of files and folders that are CEGUI related
    (.font, .layout, ...)

*/
class Project : public QStandardItemModel
{
public:
    Item* m_prototype;
    QString m_projectFilePath;
    QString m_baseDirectory;
    QString m_CEGUIVersion;
    QString m_CEGUIDefaultResolution;
    QString m_imagesetsPath;
    QString m_fontsPath;
    QString m_looknfeelsPath;
    QString m_schemesPath;
    QString m_layoutsPath;
    QString m_xmlSchemasPath;
    bool m_changed;
    propertymapping::PropertyMap* m_propertyMap;

    Project();

    Qt::DropAction getSupportedDropActions()
    {
        return Qt::MoveAction;
    }

    Qt::DropActions supportedDragActions() const
    {
        return Qt::MoveAction;
    }

    void load(const QString& path);

    void unload()
    {
    }

    void save(const QString& path_ = "");

    bool hasChanges()
    {
        return m_changed;
    }

    /** Converts project relative paths to absolute paths */
    QString getAbsolutePathOf(const QString& path);

    QString getRelativePathOf(const QString& path);

    QString getResourceFilePath(const QString& filename, const QString& resourceGroup);

    /** Performs base directory and resource directories sanity check,
    raises IOError in case of a failure
    */
    void checkAllDirectories();

    bool referencesFilePath(const QString& filePath);
};


/*!
\brief ProjectManager

This is basically a view of the Project model class,
    it allows browsing and (in the future) changes

*/
class ProjectManager : public QDockWidget
{
    Q_OBJECT
public:
#if 0
    project = property(lambda self: m_view->model(),
                       lambda value: setProject(value))
#endif

    Ui_ProjectManager* m_ui;
    QTreeView* m_view;
    Project* m_project;
    QMenu* m_contextMenu;
    QAction* m_createFolderAction;
    QAction* m_addNewFileAction;
    QAction* m_addExistingFileAction;
    QAction* m_renameAction;
    QAction* m_removeAction;

    ProjectManager();

    void setupContextMenu();

    void setProject(Project* project);

    static Item* getItemFromModelIndex(const QModelIndex& modelIndex);

    void itemDoubleClicked(const QModelIndex& modelIndex);

    void customContextMenu(const QPoint& point);

    void createFolder();

    void addNewFile();

    void addExistingFile();

    void renameAction();

    void removeAction();

signals:
    void fileOpenRequested(const QString& path);
};


/*!
\brief NewProjectDialog

Dialog responsible for creation of entirely new projects.

*/
class NewProjectDialog : public QDialog
{
public:
    Ui_NewProjectDialog* m_ui;
    qtwidgets::FileLineEdit* m_projectFilePath;
    QCheckBox* m_createResourceDirs;

    NewProjectDialog();

    void accept() override;

    // creates the project using data from this dialog
    Project *createProject();
};

/*!
\brief ProjectSettingsDialog

Dialog able to change various project settings

*/
class ProjectSettingsDialog : public QDialog
{
public:
    Ui_ProjectSettingsDialog* m_ui;
    qtwidgets::FileLineEdit* m_baseDirectory;
    QComboBox* m_CEGUIVersion;
    QComboBox* m_CEGUIDefaultResolution;
    FileLineEdit* m_resourceDirectory;
    QPushButton* m_resourceDirectoryApplyButton;
    FileLineEdit* m_imagesetsPath;
    FileLineEdit* m_fontsPath;
    FileLineEdit* m_looknfeelsPath;
    FileLineEdit* m_schemesPath;
    FileLineEdit* m_layoutsPath;
    FileLineEdit* m_xmlSchemasPath;

    ProjectSettingsDialog(Project* project);

    /**Applies values from this dialog to given project
    */
    void apply(Project* project);

    void applyResourceDirectory();
};

} // namespace project
} // namespace CEED

#endif
