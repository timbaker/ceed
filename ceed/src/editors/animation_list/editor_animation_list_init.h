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

#ifndef CEED_editors_animation_list___init___
#define CEED_editors_animation_list___init___

#include "CEEDBase.h"

#include "editors/editors_init.h"

#include "compatibility/animation_list/compat_animation_list_init.h"
#include "editors/animation_list/editor_animation_list_visual.h"
#include "editors/animation_list/editor_animation_list_code.h"

#include "messages.h"

#include "elementtree.h"

namespace CEED {
namespace editors {
namespace animation_list {

/*!
\brief AnimationListTabbedEditor

Animation list file editor (XML file containing list of animations)

*/
class AnimationListTabbedEditor : public editors::multi::MultiModeTabbedEditor
{
    typedef editors::multi::MultiModeTabbedEditor super;
public:
    visual::VisualEditing* m_visual;
    code::CodeEditing* m_code;

    AnimationListTabbedEditor(const QString& filePath);

    void initialise(mainwindow::MainWindow* mainWindow);

    void finalise() override;

    void activate() override;

    void deactivate() override;

    bool saveAs(const QString& targetPath, bool updateCurrentPath = true) override;

    bool zoomIn() override
    {
        if (currentWidget() == m_visual)
            return m_visual->zoomIn();

        return false;
    }

    bool zoomOut() override
    {
        if (currentWidget() == m_visual)
            return m_visual->zoomOut();

        return false;
    }

    bool zoomReset() override
    {
        if (currentWidget() == m_visual)
            return m_visual->zoomReset();

        return false;
    }
};


class AnimationListTabbedEditorFactory : public editors::TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Animation List";
    }

    QSet<QString> getFileExtensions() override
    {
        QSet<QString> extensions = { "anims" };
        return extensions;
    }

    bool canEditFile(const QString& filePath)
    {
        QSet<QString> extensions = getFileExtensions();

        for (QString extension : extensions) {
            if (filePath.endsWith("." + extension))
                return true;
        }

        return false;
    }

    TabbedEditor* create(const QString& filePath) override
    {
        return new AnimationListTabbedEditor(filePath);
    }
};

} // namespace animation_list
} // namespace editors
} // namespace CEED

#endif
