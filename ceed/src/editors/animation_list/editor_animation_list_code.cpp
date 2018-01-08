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

#include "editor_animation_list_code.h"

#include "editors/animation_list/editor_animation_list_init.h"

namespace CEED {
namespace editors {
namespace animation_list {
namespace code {

QString CodeEditing::getNativeCode()
{
    return m_tabbedEditor->m_visual->generateNativeData();
}

bool CodeEditing::propagateNativeCode(const QString &code)
{
    try {
        auto element = ElementTree::fromstring(code);
        m_tabbedEditor->m_visual->loadFromElement(element);
        return true;
    } catch (...) {
        return false;
    }
}

UndoStackTabbedEditor *CodeEditing::getTabbedEditor()
{
    return m_tabbedEditor;
}


} // namespace code
} // namespace animation_list
} // namespace editors
} // namespace CEED
