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

#ifndef CEED_editors_layout_preview_
#define CEED_editors_layout_preview_

#include "CEEDBase.h"

#include "editors/editors_multi.h"

namespace CEED {
namespace editors {
namespace layout {
namespace preview {

/**Provides "Live Preview" which is basically interactive CEGUI rendering
without any other outlines or what not over it.
*/
class LayoutPreviewer : public QWidget, public multi::EditMode
{
public:
    LayoutTabbedEditor* m_tabbedEditor;
    CEGUI::Window* m_rootWidget;

    LayoutPreviewer(LayoutTabbedEditor* tabbedEditor);

    void activate() override;

    bool deactivate() override;

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;
};

} // namespace preview
} // namespace layout
} // namespace editors
} // namespace CEED


#endif
