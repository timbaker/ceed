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

#include "code_edit_restoring_view.h"

#include <QScrollBar>

namespace CEED {
namespace editors {
namespace code_edit_restoring_view {

void CodeEditingWithViewRestore::refreshFromVisual()
{
    multi::CodeEditMode::refreshFromVisual();

    auto vbar = verticalScrollBar();
    vbar->setValue(m_lastVertScrollBarValue);

    textCursor().setPosition(m_lastCursorSelectionStart);
    textCursor().setPosition(m_lastCursorSelectionEnd, QTextCursor::KeepAnchor);
    setFocus();
    setTextCursor(textCursor());
}

bool CodeEditingWithViewRestore::propagateToVisual()
{
    auto vbar = verticalScrollBar();
    m_lastVertScrollBarValue = vbar->value();

    m_lastCursorSelectionStart = textCursor().selectionStart();
    m_lastCursorSelectionEnd = textCursor().selectionEnd();

    return multi::CodeEditMode::propagateToVisual();
}


} // namespace code_edit_restoring_view
} // namespace editors
} // namespace CEED
