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

#ifndef CEED_editors_code_edit_restoring_view_
#define CEED_editors_code_edit_restoring_view_

#include "CEEDBase.h"

#include "editors_multi.h"

namespace CEED {
namespace editors {
namespace code_edit_restoring_view {

class CodeEditingWithViewRestore : public multi::CodeEditMode
{
public:
    int m_lastVertScrollBarValue;
    int m_lastCursorSelectionStart;
    int m_lastCursorSelectionEnd;

    CodeEditingWithViewRestore()
        : multi::CodeEditMode()
    {
        m_lastVertScrollBarValue = 0;
        m_lastCursorSelectionEnd = 0;
        m_lastCursorSelectionStart = 0;
    }

    /**Refreshes this Code editing mode with current native source code and moves to the last scroll and cursor
    positions.*/
    void refreshFromVisual() override;

    /**Propagates source code from this Code editing mode to your editor implementation and stores
    the last scrollbar and cursor positions

    Returns true if changes were accepted (the code was valid, etc...)
    Returns false if changes weren't accepted (invalid code most likely)*/
    bool propagateToVisual() override;
};

} // namespace code_edit_restoring_view
} // namespace editors
} // namespace CEED

#endif
