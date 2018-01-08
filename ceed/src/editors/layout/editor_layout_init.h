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

#ifndef CEED_editors_layout___init___
#define CEED_editors_layout___init___

#include "CEEDBase.h"

#include "editors/editors_init.h"
#include "settings/settings_init.h"

#include "compatibility/layout/compat_layout_init.h"

#include "editor_layout_code.h"
#include "editor_layout_preview.h"
#include "editor_layout_visual.h"

namespace CEED {
namespace editors {
namespace layout {

/*!
\brief LayoutTabbedEditor

Binds all layout editing functionality together

*/
class LayoutTabbedEditor : public multi::MultiModeTabbedEditor
{
    typedef multi::MultiModeTabbedEditor super;
public:
    visual::VisualEditing* m_visual;
    code::CodeEditing* m_code;
    preview::LayoutPreviewer* m_previewer;
    settings::declaration::Entry* m_tbIconSizeEntry;
    int m_subscribeID;

    LayoutTabbedEditor(const QString& filePath);

    void initialise(mainwindow::MainWindow* mainWindow);

    void finalise() override
    {
        super::finalise();
    }

    void destroy() override;

    void rebuildEditorMenu(QMenu* editorMenu, bool& visible, bool& enabled) override;

    void activate() override;

    void updateToolbarSize(int size);

    void deactivate() override;

    bool saveAs(const QString& targetPath, bool updateCurrentPath = true);

    bool performCut() override;

    bool performCopy() override;

    bool performPaste() override;

    bool performDelete() override;

    bool zoomIn() override;

    bool zoomOut() override;

    bool zoomReset() override;
};

class LayoutTabbedEditorFactory : public editors::TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Layout";
    }

    QSet<QString> getFileExtensions() override;

    bool canEditFile(const QString& filePath) override;

    TabbedEditor* create(const QString& filePath) override
    {
        return new LayoutTabbedEditor(filePath);
    }
};

} // namespace layout
} // namespace editors
} // namespace CEED

#endif
