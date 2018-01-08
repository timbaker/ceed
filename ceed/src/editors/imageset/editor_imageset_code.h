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

#ifndef CEED_editors_imageset_code_
#define CEED_editors_imageset_code_

#include "CEEDBase.h"

#include "elementtree.h"

#include "editors/code_edit_restoring_view.h"

namespace CEED {
namespace editors {
namespace imageset {
namespace code {

class CodeEditing : public code_edit_restoring_view::CodeEditingWithViewRestore
{
public:
    ImagesetTabbedEditor* m_tabbedEditor;

    CodeEditing(ImagesetTabbedEditor* tabbedEditor)
        : code_edit_restoring_view::CodeEditingWithViewRestore()
    {
        m_tabbedEditor = tabbedEditor;
    }

    QString getNativeCode() override;

    bool propagateNativeCode(const QString& code) override;

    UndoStackTabbedEditor* getTabbedEditor() override;
};

} // namespace code
} // namespace imageset
} // namespace editors
} // namespace CEED

#endif
