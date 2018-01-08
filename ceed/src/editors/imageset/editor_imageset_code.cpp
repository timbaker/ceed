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

#include "editors/imageset/editor_imageset_code.h"

#include "editors/imageset/editor_imageset_init.h"
#include "editors/imageset/editor_imageset_elements.h"
#include "editors/imageset/editor_imageset_visual.h"

#include "xmledit.h"

namespace CEED {
namespace editors {
namespace imageset {
namespace code {

QString CodeEditing::getNativeCode()
{
    auto element = m_tabbedEditor->m_visual->m_imagesetEntry->saveToElement();
    xmledit::indent(element);

    return ElementTree::tostring(element, "utf-8");
}

bool CodeEditing::propagateNativeCode(const QString &code)
{
    try {
        auto element = ElementTree::fromstring(code);
        m_tabbedEditor->m_visual->loadImagesetEntryFromElement(element);
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
} // namespace imageset
} // namespace editors
} // namespace CEED
