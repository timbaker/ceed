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

#include "project.h"

#include "ceed_paths.h"
#include "paths.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

namespace CEED {
namespace project {

QString convertToPortablePath(const QString &inputPath)
{
    // very crude and basic for now
#if 1
    return os.path.normpath(inputPath).replace("\\", "/");
#else
    QFileInfo fileInfo(inputPath);
    if (fileInfo.exists())
        return fileInfo.canonicalFilePath().replace("\\", "/");
    return fileInfo.absoluteFilePath().replace("\\", "/"); // should remove ../ etc
#endif
}

Item::Item(Project* project)
    : QStandardItem()
    , m_project(project)
{
    setItemType(Item::Unknown);

    m_project->m_changed = true;
}

int Item::type() const
{
    // Qt docs say we have to overload type() and return something > QStandardItem.UserType
    return QStandardItem::UserType + 1;
}

QStandardItem *Item::clone() const
{
    Item* ret = new Item(m_project);
    ret->setItemType(getItemType());

    if (getItemType() == Item::File)
        ret->setPath(getPath());

    else if (getItemType() == Item::Folder)
        ret->setName(getName());

    return ret;
}

void Item::setItemType(int value)
{
    setData(value, Qt::UserRole + 1);

    if (value == Item::File) {
        // we can drag files but we can't drop anything to them
        setDragEnabled(true);
        setDropEnabled(false);

    } else if (value == Item::Folder) {
        // we can drag folders and drop other items to them
        setDragEnabled(true);
        setDropEnabled(true);

    } else {
        // in the unknown case, lets disable both
        setDragEnabled(false);
        setDropEnabled(false);
    }
}

void Item::setPath(const QString &value)
{
    Q_ASSERT(getItemType() == Item::File);

    setData(value, Qt::UserRole + 2);
    setLabel(QFileInfo(value).baseName());

    // TODO: File type icons are completely independent from tabbed editors,
    //       do we want to couple them or not?
    //
    //       Project management right now has no idea about mainwindow
    //       or tabbed editing (and that's a good thing IMO)

    QString fileType = "unknown";
    if (value.endsWith(".font"))
        fileType = "font";
    else if (value.endsWith(".layout"))
        fileType = "layout";
    else if (value.endsWith(".imageset"))
        fileType = "imageset";
    else if (value.endsWith(".anim"))
        fileType = "animation";
    else if (value.endsWith(".scheme"))
        fileType = "scheme";
    else if (value.endsWith(".looknfeel"))
        fileType = "looknfeel";
    else if (value.endsWith(".py"))
        fileType = "python_script";
    else if (value.endsWith(".lua"))
        fileType = "lua_script";
    else if (value.endsWith(".xml"))
        fileType = "xml";
    else if (value.endsWith(".txt"))
        fileType = "text";
    else {
        QStringList extensions = {".png", ".jpg", ".jpeg", ".tga", ".dds"};

        for (QString extension : extensions) {
            if (value.endsWith(extension)) {
                fileType = "bitmap";
                break;
            }
        }
    }

    setIcon(QString(":/icons/project_items/%1.png").arg(fileType));

    m_project->m_changed = true;
}

QString Item::getPath() const
{
    Q_ASSERT(getItemType() == Item::File);

    return data(Qt::UserRole + 2).toString();
}

void Item::setName(const QString &value)
{
    Q_ASSERT(getItemType() == Item::Folder);

    setData(value, Qt::UserRole + 2);

    // A hack to cause folders appear first when sorted
    // TODO: Override the sorting method and make this work more cleanly
    setLabel(QString(" %1").arg(value));
    setIcon(":/icons/project_items/folder.png");

    m_project->m_changed = true;
}

QString Item::getName() const
{
    Q_ASSERT(getItemType() == Item::Folder);

    return data(Qt::UserRole + 2).toString();
}

QString Item::getRelativePath()
{
    /** Returns path relative to the projects base directory */
    Q_ASSERT(getItemType() == Item::File);

    return getPath();
}

QString Item::getAbsolutePath()
{
    Q_ASSERT(getItemType() == Item::File);

    return m_project->getAbsolutePathOf(getPath());
}

Item* Item::loadFromElement(Project* project, ElementTree::Element *element)
{
    Item* item = new Item(project);

    QString typeString = element->get("type");
    if (typeString == "file") {
        item->setItemType(Item::File);
        item->setPath(QDir::cleanPath(element->get("path")));

    } else if (typeString == "folder") {
        item->setItemType(Item::Folder);
        item->setName(element->get("name"));

        auto subItemElements = element->findall("Item");
        for (auto subItemElement : subItemElements) {
            auto subItem = item->loadFromElement(project, subItemElement);
            item->appendRow(subItem);
        }
    } else {
        throw Exception(QString("Unknown item type '%1'").arg(typeString));
    }

    return item;
}

ElementTree::Element *Item::saveToElement()
{
    auto ret = new ElementTree::Element("Item");

    if (getItemType() == Item::File) {
        ret->set("type", "file");
        ret->set("path", convertToPortablePath(getPath()));

    } else if (getItemType() == Item::Folder) {
        ret->set("type", "folder");
        ret->set("name", getName());

        int i = 0;
        while (i < rowCount()) {
            auto subItemElement = static_cast<Item*>(child(i))->saveToElement();
            ret->append(subItemElement);
            i = i + 1;
        }
    }
    return ret;
}

bool Item::referencesFilePath(const QString &filePath)
{
    if (getItemType() == Item::File) {
#if 1
        return QFileInfo(getAbsolutePath()) == QFileInfo(filePath);
#else
        // samefile isn't implemented on Windows and possibly other platforms
        if hasattr(os.path, "samefile")
                try:
            return os.path.samefile(getAbsolutePath(), filePath)

          except:
          pass

          // Figuring out whether 2 paths lead to the same file is a tricky
          // business. We will do our best but this definitely doesn't
          // work in all cases!

          thisPath = os.path.normpath(getAbsolutePath())
          otherPath = os.path.normpath(filePath)

          // This clearly is just our best guess! The file might actually
          // be on a case sensitive filesystem AND have a sibling that has
          // the same name except uppercase, the test will fail in that case.
          thisPathCaseInsensitive = os.path.exists(unicode(thisPath).upper()) and \
          os.path.exists(unicode(thisPath).lower())

          otherPathCaseInsensitive = os.path.exists(unicode(otherPath).upper()) and \
          os.path.exists(unicode(otherPath).lower())

          if thisPathCaseInsensitive != otherPathCaseInsensitive:
          return false

          if thisPathCaseInsensitive:
          // the other will also be case insensitive
          thisPath = unicode(thisPath).lower()
          otherPath = unicode(thisPath).lower()

          return thisPath == otherPath;
#endif
    }

    else if (getItemType() == Item::Folder) {
        int i = 0;
        while (i < rowCount()) {
            auto subItemElement = static_cast<Item*>(child(i));
            if (subItemElement->referencesFilePath(filePath)) {
                return true;
            }
            i += 1;
        }

        return false;

    } else {
        // Unknown Item
        return false;
    }
}

/////

Project::Project()
    : QStandardItemModel()
{
    setHorizontalHeaderLabels({"Name"});
    m_prototype = new Item(this);
    setItemPrototype(m_prototype);

    m_projectFilePath = "";

    m_baseDirectory = "./";

    // default to the best case, native version :-)
    m_CEGUIVersion = compatibility::EditorEmbeddedCEGUIVersion;
    // 720p seems like a decent default nowadays, 16:9
    m_CEGUIDefaultResolution = "1280x720";

    m_imagesetsPath = "./imagesets";
    m_fontsPath = "./fonts";
    m_looknfeelsPath = "./looknfeel";
    m_schemesPath = "./schemes";
    m_layoutsPath = "./layouts";
    m_xmlSchemasPath = "./xml_schemas";

    m_changed = true;

    QStringList pmappings = { "mappings/Base.pmappings" };
    QStringList files;
    for (QString path : pmappings) {
#if 1 // I changed this for the C++ version
        files += os.path.join(paths::DATA_DIR, path);
#else
        files += os.path.abspath(path);
#endif
    }
    m_propertyMap = propertymapping::PropertyMap::fromFiles(files);
}

void Project::load(const QString &path)
{
    /** Loads XML project file from given path (preferably absolute path) */

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw IOError(QString("QFile::open failed with file \"%1\"").arg(path));
    }
    QByteArray data = file.readAll();
    file.close();
    QString rawData = QString::fromUtf8(data);
    QString nativeData = compatibility::project::manager->transformTo(compatibility::project::manager->EditorNativeType, rawData, path);

    auto root = ElementTree::fromstring(nativeData);

    m_baseDirectory = QDir::cleanPath(root->get("baseDirectory", "./"));
    m_CEGUIVersion = root->get("CEGUIVersion", compatibility::EditorEmbeddedCEGUIVersion);
    m_CEGUIDefaultResolution = root->get("CEGUIDefaultResolution", "1280x720");

    m_imagesetsPath = QDir::cleanPath(root->get("imagesetsPath", "./imagesets"));
    m_fontsPath = QDir::cleanPath(root->get("fontsPath", "./fonts"));
    m_looknfeelsPath = QDir::cleanPath(root->get("looknfeelsPath", "./looknfeel"));
    m_schemesPath = QDir::cleanPath(root->get("schemesPath", "./schemes"));
    m_layoutsPath = QDir::cleanPath(root->get("layoutsPath", "./layouts"));
    m_xmlSchemasPath = QDir::cleanPath(root->get("xmlSchemasPath", "./xml_schemas"));

    auto items = root->find("Items");

    for (auto itemElement : items->findall("Item")) {
        auto item = Item::loadFromElement(this, itemElement);
        appendRow(item);
    }

    m_changed = false;
    m_projectFilePath = path;
}

void Project::save(const QString &path_)
{
    QString path = path_;
    if (path == "") {
        // "save" vs "save as"
        path = m_projectFilePath;
        m_changed = false;
    }

    auto root = new ElementTree::Element("Project");

    root->set("version", compatibility::project::manager->EditorNativeType);

    root->set("baseDirectory", convertToPortablePath(m_baseDirectory));

    root->set("CEGUIVersion", m_CEGUIVersion);
    root->set("CEGUIDefaultResolution", m_CEGUIDefaultResolution);

    root->set("imagesetsPath", convertToPortablePath(m_imagesetsPath));
    root->set("fontsPath", convertToPortablePath(m_fontsPath));
    root->set("looknfeelsPath", convertToPortablePath(m_looknfeelsPath));
    root->set("schemesPath", convertToPortablePath(m_schemesPath));
    root->set("layoutsPath", convertToPortablePath(m_layoutsPath));
    root->set("xmlSchemasPath", convertToPortablePath(m_xmlSchemasPath));

    auto items = new ElementTree::SubElement(root, "Items");

    int i = 0;
    while (i < rowCount()) {
        items->append(static_cast<Item*>(item(i))->saveToElement());
        i = i + 1;
    }

    xmledit::indent(root);

    QString nativeData = ElementTree::tostring(root, "utf-8");
    QFile outputFile(path);
    outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
//    outputFile.write(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n").toUtf8());
    outputFile.write(nativeData.toUtf8());
    outputFile.close();
}

QString Project::getAbsolutePathOf(const QString &path)
{
    QString absoluteBaseDirectory = os.path.join(os.path.dirname(m_projectFilePath), m_baseDirectory);
    return QDir::cleanPath(os.path.join(absoluteBaseDirectory, path));
}

QString Project::getRelativePathOf(const QString &path)
{
    return os.path.normpath(os.path.relpath(path, os.path.join(os.path.abspath(os.path.dirname(m_projectFilePath)), m_baseDirectory)));
}

QString Project::getResourceFilePath(const QString& filename, const QString& resourceGroup)
{
    // FIXME: The whole resource provider wrapping should be done proper, see http://www.cegui.org.uk/mantis/view.php?id=552
    QString folder = "";
    if (resourceGroup == "imagesets")
        folder = m_imagesetsPath;
    else if (resourceGroup == "fonts")
        folder = m_fontsPath;
    else if (resourceGroup == "looknfeels")
        folder = m_looknfeelsPath;
    else if (resourceGroup == "schemes")
        folder = m_schemesPath;
    else if (resourceGroup == "layouts")
        folder = m_layoutsPath;
    else if (resourceGroup == "xml_schemas")
        folder = m_xmlSchemasPath;
    else
        throw RuntimeError(QString("Unknown resource group '%1'").arg(resourceGroup));

    return getAbsolutePathOf(os.path.join(folder, filename));
}

void Project::checkAllDirectories()
{
    // check the base directory
    if (!os.path.isdir(getAbsolutePathOf(""))) {
        throw IOError(QString("Base directory '%1' isn't a directory or isn't accessible.").arg(getAbsolutePathOf("")));
    }

    QStringList dirs = { "imagesets", "fonts", "looknfeels", "schemes", "layouts", "xml_schemas" };
    for (QString resourceCategory : dirs) {
        QString directoryPath = getResourceFilePath("", resourceCategory);
        if (!os.path.isdir(directoryPath)) {
            throw IOError(QString("Resource directory '%1' for resources of type '%2' isn't a directory or isn't accessible").arg(directoryPath).arg(resourceCategory));
        }
    }
}

bool Project::referencesFilePath(const QString &filePath)
{
    /**Checks whether given absolute path is referenced by any File item
        in the project*/

    int i = 0;
    while (i < rowCount()) {
        Item* item_ = static_cast<Item*>(item(i));

        if (item_->referencesFilePath(filePath)) {
            return true;
        }

        i += 1;
    }

    return false;
}

////

ProjectManager::ProjectManager()
    : QDockWidget()
{
    m_ui = new Ui_ProjectManager();
    m_ui->setupUi(this);

    m_view = m_ui->view;
    m_view->sortByColumn(0, Qt::AscendingOrder);
    connect(m_view, &QTreeView::doubleClicked, this, &ProjectManager::itemDoubleClicked);

    setupContextMenu();

    setProject(nullptr);
}

void ProjectManager::setupContextMenu()
{
    m_view->setContextMenuPolicy(Qt::CustomContextMenu);

    m_contextMenu = new QMenu(this);

    m_createFolderAction = new QAction(QIcon(":icons/project_management/create_folder.png"), "Create folder", this);
    m_contextMenu->addAction(m_createFolderAction);
    connect(m_createFolderAction, &QAction::triggered, this, &ProjectManager::createFolder);

    m_contextMenu->addSeparator();

    m_addNewFileAction = new QAction(QIcon(":icons/project_management/add_new_file.png"), "Add new file", this);
    m_contextMenu->addAction(m_addNewFileAction);
    connect(m_addNewFileAction, &QAction::triggered, this, &ProjectManager::addNewFile);

    m_addExistingFileAction = new QAction(QIcon(":icons/project_management/add_existing_file.png"), "Add existing file(s)", this);
    m_contextMenu->addAction(m_addExistingFileAction);
    connect(m_addExistingFileAction, &QAction::triggered, this, &ProjectManager::addExistingFile);

    m_contextMenu->addSeparator();

    m_renameAction = new QAction(QIcon(":icons/project_management/rename.png"), "Rename file/folder", this);
    m_contextMenu->addAction(m_renameAction);
    connect(m_renameAction, &QAction::triggered, this, &ProjectManager::renameAction);

    m_removeAction = new QAction(QIcon(":icons/project_management/remove.png"), "Remove file(s)/folder(s)", this);
    m_contextMenu->addAction(m_removeAction);
    connect(m_removeAction, &QAction::triggered, this, &ProjectManager::removeAction);

    connect(m_view, &QTreeView::customContextMenuRequested, this, &ProjectManager::customContextMenu);
}

void ProjectManager::setProject(Project *project)
{
    m_project = project;

    setEnabled(project != nullptr);

    m_view->setModel(project);
}

Item *ProjectManager::getItemFromModelIndex(const QModelIndex &modelIndex)
{
    // todo: is this ugly thing really the way to do this?

    // Qt says in the docs that returned parent is never None but can be invalid if it's the root
    if (!modelIndex.parent().isValid()) {
        auto model = static_cast<const Project*>(modelIndex.model()); // FIXME: same as m_project?
        // if it's invalid, we can use the indices as absolute model top level indices
        return static_cast<Item*>(model->item(modelIndex.row(), modelIndex.column()));
    } else {
        // otherwise, resort to recursion
        return static_cast<Item*>(getItemFromModelIndex(modelIndex.parent())->child(modelIndex.row(), modelIndex.column()));
    }
}

void ProjectManager::itemDoubleClicked(const QModelIndex &modelIndex)
{
    if (!modelIndex.model()) {
        return;
    }

    Item* item = getItemFromModelIndex(modelIndex);
    if (item->getItemType() == Item::File) {
        // only react to files, expanding folders is handled by Qt
        emit fileOpenRequested(item->getAbsolutePath());
    }
}

void ProjectManager::customContextMenu(const QPoint &point)
{
    if (isEnabled()) {
        // Qt fails at English a bit?
        QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();
        // </grammar-nazi-mode>

        // we set everything to disabled and then enable what's relevant
        m_createFolderAction->setEnabled(false);
        m_addNewFileAction->setEnabled(false);
        m_addExistingFileAction->setEnabled(false);
        m_renameAction->setEnabled(false);
        m_removeAction->setEnabled(false);

        if (selectedIndices.isEmpty()) {
            // create root folder
            m_createFolderAction->setEnabled(true);
            m_addNewFileAction->setEnabled(true);
            m_addExistingFileAction->setEnabled(true);
        }

        else if (selectedIndices.length() == 1) {
            QModelIndex index = selectedIndices[0];
            Item* item = getItemFromModelIndex(index);

            if (item->getItemType() == Item::Folder) {
                // create folder inside folder
                m_createFolderAction->setEnabled(true);
                m_addNewFileAction->setEnabled(true);
                m_addExistingFileAction->setEnabled(true);
            }

            m_renameAction->setEnabled(true);
            m_removeAction->setEnabled(true);

        } else {
            // more than 1 selected item
            m_removeAction->setEnabled(true);

            //for index in selectedIndices:
            //    item = getItemFromModelIndex(index)
        }
    }

    m_contextMenu->exec(mapToGlobal(point));
}

void ProjectManager::createFolder()
{
    // TODO: Name clashes!

    bool ok;
    QString text = QInputDialog::getText(this,
                                         "Create a folder (only affects project file)",
                                         "Name",
                                         QLineEdit::Normal,
                                         "New folder",
                                         &ok);

    if (ok) {
        Item* item = new Item(m_project);
        item->setItemType(Item::Folder);
        item->setName(text);

        QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();

        if (selectedIndices.isEmpty()) {
            m_project->appendRow(item);

        } else {
            Q_ASSERT(selectedIndices.length() == 1);

            Item* parent = getItemFromModelIndex(selectedIndices[0]);
            Q_ASSERT(parent->getItemType() == Item::Folder);

            parent->appendRow(item);
        }
    }
}

void ProjectManager::addNewFile()
{
    // TODO: name clashes, duplicates

    QString file = QFileDialog::getSaveFileName(
                this,
                "Create a new file and add it to the project",
                m_project->getAbsolutePathOf("")
                );

    if (file.isEmpty()) {
        // user cancelled
        return;
    }

    try {
        QFile f(file);
        if (!f.open(QIODevice::WriteOnly))
            throw OSError("QFile::open failed");
        f.close();

    } catch (OSError e) {
        QMessageBox::question(this,
                              "Can't create file!",
                              QString("Creating file '%1' failed. Exception details follow:\n%2").arg(file).arg("sys.exc_info()[1]"),
                QMessageBox::Ok);

        return;
    }

    QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();

    Item* item = new Item(m_project);
    item->setItemType(Item::File);
    item->setPath(m_project->getRelativePathOf(file));

    if (selectedIndices.isEmpty()) {
        m_project->appendRow(item);

    } else {
        Q_ASSERT(selectedIndices.length() == 1);

        Item* parent = getItemFromModelIndex(selectedIndices[0]);
        Q_ASSERT(parent->getItemType() == Item::Folder);

        parent->appendRow(item);
    }
}

void ProjectManager::addExistingFile()
{
    // TODO: name clashes, duplicates

    QStringList files = QFileDialog::getOpenFileNames(this,
                                                     "Select one or more files to add to the project",
                                                     m_project->getAbsolutePathOf(""));
    QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();

    // lets see if file user wants added isn't already there
    QMessageBox::StandardButton previousResponse = QMessageBox::NoButton;
    for (QString file : files) {
        if (m_project->referencesFilePath(file)) {
            // file is already in the project
            QMessageBox::StandardButton response = QMessageBox::NoButton;

            if (previousResponse == QMessageBox::YesToAll)
                response = QMessageBox::YesToAll;
            else if (previousResponse == QMessageBox::NoToAll)
                response = QMessageBox::NoToAll;
            else {
                response = QMessageBox::question(this, "File is already in the project!",
                                                 QString("File '%1' that you are trying to add is already referenced in the "
                                                         "project.\n\n"
                                                         "Do you want to add it as a duplicate?").arg(file),
                                                 QMessageBox::Yes | QMessageBox::No |
                                                 QMessageBox::YesToAll | QMessageBox::NoToAll,
                                                 QMessageBox::Yes);

                previousResponse = response;
            }
            if (response == QMessageBox::No || response == QMessageBox::NoToAll)
                continue;
        }

        Item* item = new Item(m_project);
        item->setItemType(Item::File);
        item->setPath(m_project->getRelativePathOf(file));

        if (selectedIndices.isEmpty()) {
            m_project->appendRow(item);

        } else {
            Q_ASSERT(selectedIndices.length() == 1);

            Item* parent = getItemFromModelIndex(selectedIndices[0]);
            Q_ASSERT(parent->getItemType() == Item::Folder);

            parent->appendRow(item);
        }
    }
}

void ProjectManager::renameAction()
{
    // TODO: Name clashes!

    QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();
    Q_ASSERT(selectedIndices.length() == 1);

    Item* item = getItemFromModelIndex(selectedIndices[0]);
    if (item->getItemType() == Item::File) {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             "Rename file (renames the file on the disk!)",
                                             "New name",
                                             QLineEdit::Normal,
                                             QFileInfo(item->getPath()).fileName(),
                                             &ok);

        if (ok && text != QFileInfo(item->getPath()).fileName()) {
            // legit change
            QString newPath = os.path.join(os.path.dirname(item->getPath()), text);

            try {
                os.rename(m_project->getAbsolutePathOf(item->getPath()), m_project->getAbsolutePathOf(newPath));
                item->setPath(newPath);
                m_project->m_changed = true;

            } catch (OSError e) {
                QMessageBox::question(this,
                                      "Can't rename!",
                                      QString("Renaming file '%1' to '%2' failed. Exception details follow:\n%3")
                                      .arg(item->getPath())
                                      .arg(newPath).arg("sys.exc_info()[1]"),
                                      QMessageBox::Ok);
            }
        }

    } else if (item->getItemType() == Item::Folder) {
        bool ok;
        QString text = QInputDialog::getText(this,
                                             "Rename folder (only affects the project file)",
                                             "New name",
                                             QLineEdit::Normal,
                                             item->getName(),
                                             &ok);

        if (ok && text != item->getName()) {
            item->setName(text);
            m_project->m_changed = true;
        }
    }
}

void ProjectManager::removeAction()
{
    if (!isEnabled())
        return;

    QModelIndexList selectedIndices = m_view->selectionModel()->selectedIndexes();
    // when this is called the selection must not be empty
    Q_ASSERT(!selectedIndices.isEmpty());

    QString removeSpec;
    if (selectedIndices.length() == 1) {
        Item* item = getItemFromModelIndex(selectedIndices[0]);
        removeSpec = QString("'%1'").arg(item->getLabel());
    } else {
        removeSpec = QString("%1 project items").arg(selectedIndices.length());
    }

    // we have changes, lets ask the user whether we should dump them or save them
    int result = QMessageBox::question(this,
                                           "Remove items?",
                                           QString("Are you sure you want to remove %1 from the project? "
                                                   "This action can't be undone! "
                                                   "(Pressing Cancel will cancel the operation!)").arg(removeSpec),
                                           QMessageBox::Yes | QMessageBox::Cancel,
                                           QMessageBox::Cancel);

    if (result == QMessageBox::Cancel) {
        // user chickened out ;-)
        return;
    } else if (result == QMessageBox::Yes) {
        std::sort(selectedIndices.begin(), selectedIndices.end(), [](const QModelIndex& a, const QModelIndex& b) {
            return a.row() > b.row(); // reverse = true
        });
        int removeCount = 0;

        // we have to remove files first because multi-selection could screw us otherwise
        // (Parent also removes it's children)

        // first remove files
        for (QModelIndex index : selectedIndices) {
            Item* item = getItemFromModelIndex(index);

            if (item == nullptr)
                continue;

            if (item->getItemType() == Item::File) {
                m_project->removeRow(index.row(), index.parent());
                removeCount += 1;
            }
        }

        // then remove folders
        for (QModelIndex& index : selectedIndices) {
            Item* item = getItemFromModelIndex(index);

            if (item == nullptr)
                continue;

            if (item->getItemType() == Item::Folder) {
                m_project->removeRow(index.row(), index.parent());
                removeCount += 1;
            }
        }

        if (selectedIndices.length() - removeCount > 0) {
            logging.error(QString("%1 selected project items are unknown and can't be deleted").arg(selectedIndices.length()));
        }

        if (removeCount > 0) {
            m_project->m_changed = true;
        }
    }
}

/////

NewProjectDialog::NewProjectDialog()
    : QDialog()
{
    m_ui = new Ui_NewProjectDialog();
    m_ui->setupUi(this);

    m_projectFilePath = m_ui->projectFilePath;
    m_projectFilePath->m_filter = "Project file (*.project)";
    m_projectFilePath->m_mode = FileLineEdit::NewFileMode;

    m_createResourceDirs = m_ui->createResourceDirs;
}

void NewProjectDialog::accept()
{
    if (m_projectFilePath->text() == "") {
        QMessageBox::critical(this, "Project file path empty!", "You must supply a valid project file path!");
        return;
    }

    if (!QFileInfo(os.path.dirname(m_projectFilePath->text())).exists()) {
        QMessageBox::critical(this,"Project file path invalid!",  QString("Its parent directory ('%1') is inaccessible!").arg(os.path.dirname(m_projectFilePath->text())));
        return;
    }

    QDialog::accept();
}

Project* NewProjectDialog::createProject()
{
    Project* ret = new Project();
    ret->m_projectFilePath = m_projectFilePath->text();

    if (!ret->m_projectFilePath.endsWith(".project")) {
        // enforce the "project" extension
        ret->m_projectFilePath += ".project";
    }

    if (m_createResourceDirs->checkState() == Qt::Checked) {
        try {
            QString prefix = os.path.dirname(ret->m_projectFilePath);
            QStringList dirs = { "fonts", "imagesets", "looknfeel", "schemes", "layouts", "xml_schemas" };

            for (QString dir : dirs) {
                if (!os.path.exists(os.path.join(prefix, dir))) {
                    os.mkdir(os.path.join(prefix, dir));
                }
            }

        } catch (OSError e) {
            QMessageBox::critical(this, "Cannot create resource directories!",
                                  QString("There was a problem creating the resource "
                                          "directories.  Do you have the proper permissions on the "
                                          "parent directory? (exception info: %1)").arg(e()));
        }
    }
    return ret;
}

/////

ProjectSettingsDialog::ProjectSettingsDialog(Project *project)
    : QDialog()
{
    m_ui = new Ui_ProjectSettingsDialog();
    m_ui->setupUi(this);

    m_baseDirectory = m_ui->baseDirectory;
    m_baseDirectory->m_mode = FileLineEdit::ExistingDirectoryMode;

    m_CEGUIVersion = m_ui->CEGUIVersion;
    for (QString version : compatibility::CEGUIVersions) {
        m_CEGUIVersion->addItem(version);
    }

    m_CEGUIVersion->setEditText(project->m_CEGUIVersion);

    m_CEGUIDefaultResolution = m_ui->CEGUIDefaultResolution;
    m_CEGUIDefaultResolution->setEditText(project->m_CEGUIDefaultResolution);

    m_resourceDirectory = m_ui->resourceDirectory;
    m_resourceDirectory->m_mode = qtwidgets::FileLineEdit::ExistingDirectoryMode;
    m_resourceDirectoryApplyButton = m_ui->resourceDirectoryApplyButton;
    connect(m_resourceDirectoryApplyButton, &QPushButton::pressed, this, &ProjectSettingsDialog::applyResourceDirectory);

    m_imagesetsPath = m_ui->imagesetsPath;
    m_imagesetsPath->m_mode = FileLineEdit::ExistingDirectoryMode;
    m_fontsPath = m_ui->fontsPath;
    m_fontsPath->m_mode = FileLineEdit::ExistingDirectoryMode;
    m_looknfeelsPath = m_ui->looknfeelsPath;
    m_looknfeelsPath->m_mode = FileLineEdit::ExistingDirectoryMode;
    m_schemesPath = m_ui->schemesPath;
    m_schemesPath->m_mode = FileLineEdit::ExistingDirectoryMode;
    m_layoutsPath = m_ui->layoutsPath;
    m_layoutsPath->m_mode = FileLineEdit::ExistingDirectoryMode;
    m_xmlSchemasPath = m_ui->xmlSchemasPath;
    m_xmlSchemasPath->m_mode = FileLineEdit::ExistingDirectoryMode;

    m_baseDirectory->setText(project->getAbsolutePathOf(""));
    m_imagesetsPath->setText(project->getAbsolutePathOf(project->m_imagesetsPath));
    m_fontsPath->setText(project->getAbsolutePathOf(project->m_fontsPath));
    m_looknfeelsPath->setText(project->getAbsolutePathOf(project->m_looknfeelsPath));
    m_schemesPath->setText(project->getAbsolutePathOf(project->m_schemesPath));
    m_layoutsPath->setText(project->getAbsolutePathOf(project->m_layoutsPath));
    m_xmlSchemasPath->setText(project->getAbsolutePathOf(project->m_xmlSchemasPath));
}

void ProjectSettingsDialog::apply(Project *project)
{
    QString absBaseDir = os.path.normpath(os.path.abspath(m_baseDirectory->text()));
    project->m_baseDirectory = os.path.relpath(absBaseDir, os.path.dirname(project->m_projectFilePath));

    project->m_CEGUIVersion = m_CEGUIVersion->currentText();
    project->m_CEGUIDefaultResolution = m_CEGUIDefaultResolution->currentText();

    project->m_imagesetsPath = os.path.relpath(m_imagesetsPath->text(), absBaseDir);
    project->m_fontsPath = os.path.relpath(m_fontsPath->text(), absBaseDir);
    project->m_looknfeelsPath = os.path.relpath(m_looknfeelsPath->text(), absBaseDir);
    project->m_schemesPath = os.path.relpath(m_schemesPath->text(), absBaseDir);
    project->m_layoutsPath = os.path.relpath(m_layoutsPath->text(), absBaseDir);
    project->m_xmlSchemasPath = os.path.relpath(m_xmlSchemasPath->text(), absBaseDir);
}

void ProjectSettingsDialog::applyResourceDirectory()
{
    QString resourceDir = os.path.normpath(os.path.abspath(m_resourceDirectory->text()));

    m_imagesetsPath->setText(QDir(resourceDir).absoluteFilePath("imagesets"));
    m_fontsPath->setText(QDir(resourceDir).absoluteFilePath("fonts"));
    m_looknfeelsPath->setText(QDir(resourceDir).absoluteFilePath("looknfeel"));
    m_schemesPath->setText(QDir(resourceDir).absoluteFilePath("schemes"));
    m_layoutsPath->setText(QDir(resourceDir).absoluteFilePath("layouts"));
    m_xmlSchemasPath->setText(QDir(resourceDir).absoluteFilePath("xml_schemas"));
}


} // namespace project
} // namespace CEED
