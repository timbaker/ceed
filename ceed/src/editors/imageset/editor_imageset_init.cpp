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

#include "editors/imageset/editor_imageset_init.h"
#include "editors/imageset/editor_imageset_code.h"

#include "compatibility/imageset/compat_imageset_init.h"

#include "editors/imageset/editor_imageset_code.h"
#include "editors/imageset/editor_imageset_elements.h"
#include "editors/imageset/editor_imageset_visual.h"

#include "ceed_paths.h"
#include "mainwindow.h"

#include <QMenu>
#include <QToolBar>

namespace CEED {
namespace editors {
namespace imageset {

ImagesetTabbedEditor::ImagesetTabbedEditor(const QString &filePath)
    : multi::MultiModeTabbedEditor(compatibility::imageset::manager, filePath)
{
    m_visual = new visual::VisualEditing(this);
    addTab(m_visual, "Visual");

    m_code = new code::CodeEditing(this);
    addTab(m_code, "Code");

    m_tabWidget = this;

    // set the toolbar icon size according to the setting and subscribe to it
    m_tbIconSizeEntry = settings::getEntry("global/ui/toolbar_icon_size");
    m_subscribeID = m_tbIconSizeEntry->subscribe([=](const QVariant& v){ updateToolbarSize(v.toInt()); });
}

void ImagesetTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    super::initialise(mainWindow);

    ElementTree::Element* root = nullptr;
    try {
        root = ElementTree::fromstring(m_nativeData);

    } catch (...) {
        // things didn't go smooth
        // 2 reasons for that
        //  * the file is empty
        //  * the contents of the file are invalid
        //
        // In the first case we will silently move along (it is probably just a new file),
        // in the latter we will output a message box informing about the situation

        // the file should exist at this point, so we are not checking and letting exceptions
        // fly out of this method
        if (os.path.getsize(m_filePath) > 2) {
            // the file contains more than just CR LF
            QMessageBox::question(this,
                                  "Can't parse given imageset!",
                                  QString("Parsing '%1' failed, it's most likely not a valid XML file. "
                                          "Constructing empty imageset instead (if you save you will override the invalid data!). "
                                          "Exception details follow:\n%2").arg(m_filePath, "sys.exc_info()[1]"),
                                  QMessageBox::Ok);
        }

        // we construct the minimal empty imageset
        root = new ElementTree::Element("Imageset");
        root->set("Name", "");
        root->set("Imagefile", "");
    }

    m_visual->initialise(root);
}

void ImagesetTabbedEditor::finalise()
{
    // unsubscribe from the toolbar icon size setting
    m_tbIconSizeEntry->unsubscribe(m_subscribeID);

    super::finalise();
}

void ImagesetTabbedEditor::rebuildEditorMenu(QMenu *editorMenu, bool &visible, bool &enabled)
{
    editorMenu->setTitle("&Imageset");
    m_visual->rebuildEditorMenu(editorMenu);

    visible = true;
    enabled = currentWidget() == m_visual;
}

void ImagesetTabbedEditor::activate()
{
    super::activate();

    m_mainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_visual->m_toolBar);
    m_visual->m_toolBar->show();

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_dockWidget);
    m_visual->m_dockWidget->setVisible(true);
}

void ImagesetTabbedEditor::updateToolbarSize(int size)
{
    if (size < 16)
        size = 16;
    m_visual->m_toolBar->setIconSize(QSize(size, size));
}

void ImagesetTabbedEditor::deactivate()
{
    m_mainWindow->removeDockWidget(m_visual->m_dockWidget);
    m_mainWindow->removeToolBar(m_visual->m_toolBar);

    super::deactivate();
}

bool ImagesetTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    bool codeMode = currentWidget() == m_code;

    // if user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)

    if (codeMode)
        m_code->propagateToVisual();

    auto rootElement = m_visual->m_imagesetEntry->saveToElement();
    // we indent to make the resulting files as readable as possible
    xmledit::indent(rootElement);

    m_nativeData = ElementTree::tostring(rootElement, "utf-8");

    return super::saveAs(targetPath, updateCurrentPath);
}

bool ImagesetTabbedEditor::performCut()
{
    if (currentWidget() == m_visual)
        return m_visual->performCut();

    return false;
}

bool ImagesetTabbedEditor::performCopy()
{
    if (currentWidget() == m_visual)
        return m_visual->performCopy();

    return false;
}

bool ImagesetTabbedEditor::performPaste()
{
    if (currentWidget() == m_visual)
        return m_visual->performPaste();

    return false;
}

bool ImagesetTabbedEditor::performDelete()
{
    if (currentWidget() == m_visual)
        return m_visual->performDelete();

    return false;
}

bool ImagesetTabbedEditor::zoomIn()
{
    if (currentWidget() == m_visual)
        m_visual->zoomIn();
    return true;
}

bool ImagesetTabbedEditor::zoomOut()
{
    if (currentWidget() == m_visual)
        m_visual->zoomOut();
    return true;
}

bool ImagesetTabbedEditor::zoomReset()
{
    if (currentWidget() == m_visual)
        m_visual->zoomOriginal();
    return true;
}

/////

QSet<QString> ImagesetTabbedEditorFactory::getFileExtensions()
{
    auto extensions = compatibility::imageset::manager->getAllPossibleExtensions();
    return extensions;
}

bool ImagesetTabbedEditorFactory::canEditFile(const QString &filePath)
{
    auto extensions = getFileExtensions();

    for (QString extension : extensions) {
        if (filePath.endsWith("." + extension))
            return true;
    }

    return false;
}

TabbedEditor *ImagesetTabbedEditorFactory::create(const QString &filePath)
{
    return new ImagesetTabbedEditor(filePath);
}

} // namespace imageset
} // namespace editors
} // namespace CEED
