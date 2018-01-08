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

#ifndef CEED_editors_looknfeel_preview_
#define CEED_editors_looknfeel_preview_

#include "CEEDBase.h"

#include "editors/editors_multi.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace preview {

/**Provides "Live Preview" which is basically interactive CEGUI rendering
without any other outlines or what not over it.
*/
class LookNFeelPreviewer : public QWidget, public multi::EditMode
{
    typedef QWidget super;
public:
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    CEGUI::Window* m_rootWidget;

    /**
    :param tabbedEditor: LookNFeelTabbedEditor
    :return:
    */
    LookNFeelPreviewer(tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    void activate();

    bool deactivate();

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;
};

} // namespace hierarchy_tree_view
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
