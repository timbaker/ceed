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

#include "editor_layout_code.h"

#include "cegui/cegui_container.h"

#include "editors/layout/editor_layout_init.h"

#include "mainwindow.h"

namespace CEED {
namespace editors {
namespace layout {
namespace code {

CodeEditing::CodeEditing(LayoutTabbedEditor *tabbedEditor)
    : code_edit_restoring_view::CodeEditingWithViewRestore()
{
    m_tabbedEditor = tabbedEditor;
}

QString CodeEditing::getNativeCode()
{
    auto* currentRootWidget = m_tabbedEditor->m_visual->getCurrentRootWidget();

    if (currentRootWidget == nullptr)
        return "";
    else
        return TO_QSTR(CEGUI::WindowManager::getSingleton().getLayoutAsString(*currentRootWidget));
}

bool CodeEditing::propagateNativeCode(const QString &code)
{
    // we have to make the context the current context to ensure textures are fine
    mainwindow::MainWindow::instance->ceguiContainerWidget->makeGLContextCurrent();

    if (code == "") {
        m_tabbedEditor->m_visual->setRootWidget(nullptr);
        return true;

    } else {
        try {
            CEGUI::Window* newRoot = CEGUI::WindowManager::getSingleton().loadLayoutFromString(FROM_QSTR(code));
            m_tabbedEditor->m_visual->setRootWidget(newRoot);

            return true;
        }

        catch (...) {
            return false;
        }
    }
}

UndoStackTabbedEditor *CodeEditing::getTabbedEditor()
{
    return m_tabbedEditor;
}


} // namespace code
} // namespace layout
} // namespace editors
} // namespace CEED
