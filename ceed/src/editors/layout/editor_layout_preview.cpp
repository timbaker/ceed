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

#include "editors/layout/editor_layout_preview.h"

#include "cegui/cegui_container.h"
#include "editors/layout/editor_layout_init.h"
#include "editors/layout/editor_layout_visual.h"

#include "mainwindow.h"

#include <QVBoxLayout>

namespace CEED {
namespace editors {
namespace layout {
namespace preview {

LayoutPreviewer::LayoutPreviewer(LayoutTabbedEditor *tabbedEditor)
    : QWidget()
{
    m_tabbedEditor = tabbedEditor;
    m_rootWidget = nullptr;

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

void LayoutPreviewer::activate()
{
    multi::EditMode::activate();

    Q_ASSERT(m_rootWidget == nullptr);

    // we have to make the context the current context to ensure textures are fine
    mainwindow::MainWindow::instance->ceguiContainerWidget->makeGLContextCurrent();

    CEGUI::Window* currentRootWidget = m_tabbedEditor->m_visual->getCurrentRootWidget();
    if (currentRootWidget == nullptr) {
        // FIXME: destroy old rootWidget?
        m_rootWidget = nullptr;
    } else {
        // lets clone so we don't affect the layout at all
        m_rootWidget = currentRootWidget->clone();
    }

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootWidget);
}

bool LayoutPreviewer::deactivate()
{
    if (m_rootWidget != nullptr) {
        CEGUI::WindowManager::getSingleton().destroyWindow(m_rootWidget);
        m_rootWidget = nullptr;
    }

    return multi::EditMode::deactivate();
}

void LayoutPreviewer::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    mainwindow::MainWindow::instance->ceguiContainerWidget->activate(this);
    // we always want continuous rendering in live preview
    mainwindow::MainWindow::instance->ceguiContainerWidget->setViewFeatures(false, false, true);
    mainwindow::MainWindow::instance->ceguiContainerWidget->enableInput();

    if (m_rootWidget != nullptr)
        CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootWidget);
}

void LayoutPreviewer::hideEvent(QHideEvent *event)
{
    mainwindow::MainWindow::instance->ceguiContainerWidget->disableInput();
    mainwindow::MainWindow::instance->ceguiContainerWidget->deactivate(this);

    if (m_rootWidget != nullptr)
        CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(nullptr);

    QWidget::hideEvent(event);
}


} // namespace preview
} // namespace layout
} // namespace editors
} // namespace CEED
