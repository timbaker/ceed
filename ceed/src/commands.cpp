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

#include "commands.h"

#include <QIcon>
#include <QWidget>
#include <QUndoView>
#include <QVBoxLayout>

namespace CEED {
namespace commands {

UndoViewer::UndoViewer()
    : QDockWidget()
{
    setObjectName("Undo Viewer dock widget");
    setWindowTitle("Undo Viewer");

    // main undo view
    m_view = new QUndoView();
    m_view->setCleanIcon(QIcon("icons/clean_undo_state.png"));
    // root widget and layout
    QWidget* contentsWidget = new QWidget();
    QVBoxLayout* contentsLayout = new QVBoxLayout();
    contentsWidget->setLayout(contentsLayout);
    QMargins margins = contentsLayout->contentsMargins();
    margins.setTop(0);
    contentsLayout->setContentsMargins(margins);

    contentsLayout->addWidget(m_view);
    setWidget(contentsWidget);
}

void UndoViewer::setUndoStack(QUndoStack *stack)
{
    m_view->setStack(stack);
    // if stack is None this effectively disables the entire dock widget to improve UX
    setEnabled(stack != nullptr);
}

} // namespace commands
} // namespace CEED
