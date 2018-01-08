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

#include "editors/layout/editor_layout_init.h"

#include "cegui/cegui_container.h"

#include "editors/layout/editor_layout_visual.h"

#include "mainwindow.h"

#include <QMenu>
#include <QToolBar>

namespace CEED {
namespace editors {
namespace layout {

LayoutTabbedEditor::LayoutTabbedEditor(const QString &filePath)
    : super(compatibility::layout::manager, filePath)
{
    m_requiresProject = true;

    m_visual = new visual::VisualEditing(this);
    addTab(m_visual, "Visual");

    m_code = new code::CodeEditing(this);
    addTab(m_code, "Code");

    // Layout Previewer is not actually an edit mode, you can't edit the layout from it,
    // however for everything to work smoothly we do push edit mode changes to it to the
    // undo stack.
    //
    // TODO: This could be improved at least a little bit if 2 consecutive edit mode changes
    //       looked like this: A->Preview, Preview->C.  We could simply turn this into A->C,
    //       and if A = C it would eat the undo command entirely.
    m_previewer = new preview::LayoutPreviewer(this);
    addTab(m_previewer, "Live Preview");

    m_tabWidget = this;

    // set the toolbar icon size according to the setting and subscribe to it
    m_tbIconSizeEntry = settings::getEntry("global/ui/toolbar_icon_size");
    updateToolbarSize(m_tbIconSizeEntry->m_value.toInt());
    m_subscribeID = m_tbIconSizeEntry->subscribe([=](const QVariant& v){ updateToolbarSize(v.toInt()); });
}

void LayoutTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    super::initialise(mainWindow);

    // we have to make the context the current context to ensure textures are fine
    m_mainWindow->ceguiContainerWidget->makeGLContextCurrent();

    CEGUI::Window* root = nullptr;
    if (m_nativeData != "")
        root = CEGUI::WindowManager::getSingleton().loadLayoutFromString(FROM_QSTR(m_nativeData));

    m_visual->initialise(root);
}

void LayoutTabbedEditor::destroy()
{
    // unsubscribe from the toolbar icon size setting
    m_tbIconSizeEntry->unsubscribe(m_subscribeID);

    TabbedEditor::destroy(); // not QWidget::destroy()
}

void LayoutTabbedEditor::rebuildEditorMenu(QMenu *editorMenu, bool &visible, bool &enabled)
{
    editorMenu->setTitle("&Layout");
    m_visual->rebuildEditorMenu(editorMenu);

    visible = true;
    enabled = currentWidget() == m_visual;
}

void LayoutTabbedEditor::activate()
{
    super::activate();

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_hierarchyDockWidget);
    m_visual->m_hierarchyDockWidget->setVisible(true);
    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_visual->m_propertiesDockWidget);
    m_visual->m_propertiesDockWidget->setVisible(true);
    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_createWidgetDockWidget);
    m_visual->m_createWidgetDockWidget->setVisible(true);
    m_mainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_visual->m_toolBar);
    m_visual->m_toolBar->show();
}

void LayoutTabbedEditor::updateToolbarSize(int size)
{
    if (size < 16)
        size = 16;
    m_visual->m_toolBar->setIconSize(QSize(size, size));
}

void LayoutTabbedEditor::deactivate()
{
    m_mainWindow->removeDockWidget(m_visual->m_hierarchyDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_propertiesDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_createWidgetDockWidget);
    m_mainWindow->removeToolBar(m_visual->m_toolBar);

    super::deactivate();
}

bool LayoutTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    bool codeMode = currentWidget() == m_code;

    // if user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)

    if (codeMode)
        m_code->propagateToVisual();

    CEGUI::Window* currentRootWidget = m_visual->getCurrentRootWidget();

    if (currentRootWidget == nullptr) {
        QMessageBox::warning(m_mainWindow, "No root widget in the layout!",
                             "I am refusing to save your layout, CEGUI layouts are invalid unless they have a root widget!\n\nPlease create a root widget before saving.");
        return false;
    }

    m_nativeData = TO_QSTR(CEGUI::WindowManager::getSingleton().getLayoutAsString(*currentRootWidget));
    return super::saveAs(targetPath, updateCurrentPath);
}

bool LayoutTabbedEditor::performCut()
{
    if (currentWidget() == m_visual)
        return m_visual->performCut();

    return false;
}

bool LayoutTabbedEditor::performCopy()
{
    if (currentWidget() == m_visual)
        return m_visual->performCopy();

    return false;
}

bool LayoutTabbedEditor::performPaste()
{
    if (currentWidget() == m_visual)
        return m_visual->performPaste();

    return false;
}

bool LayoutTabbedEditor::performDelete()
{
    if (currentWidget() == m_visual)
        return m_visual->performDelete();

    return false;
}

bool LayoutTabbedEditor::zoomIn()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomIn();
    return false;
}

bool LayoutTabbedEditor::zoomOut()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomOut();
    return false;
}

bool LayoutTabbedEditor::zoomReset()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomOriginal();
    return false;
}

/////

QSet<QString> LayoutTabbedEditorFactory::getFileExtensions()
{
    auto extensions = compatibility::layout::manager->getAllPossibleExtensions();
    return extensions;
}

bool LayoutTabbedEditorFactory::canEditFile(const QString &filePath)
{
    auto extensions = getFileExtensions();

    for (QString extension : extensions) {
        if (filePath.endsWith("." + extension))
            return true;
    }

    return false;
}


} // namespace layout
} // namespace editors
} // namespace CEED
