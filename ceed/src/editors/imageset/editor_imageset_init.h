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

#ifndef CEED_editors_imageset___init___
#define CEED_editors_imageset___init___

#include "CEEDBase.h"

#include "settings/settings_init.h"
#include "editors/editors_init.h"
#include "xmledit.h"
#include "compatibility/imageset/compat_imageset_init.h"

#include "editors/imageset/editor_imageset_visual.h"
#include "editors/imageset/editor_imageset_code.h"

#include "elementtree.h"

#include "editors/editors_multi.h"

namespace CEED {
namespace editors {
namespace imageset {

/**Special words used in imageset editing code:
Imageset - definition of image rectangles on specified underlying image (~texture atlas)
Image Entry - one rectangle of the imageset
Image Offset - allows you to change the pivot point of the image entry which by default is at the
               top left corner of the image. To make the pivot point be at the centre of the image
               that is 25x25 pixels, set the offset to -12, -12
Underlying image - the image that lies under the image entries/rectangles (bitmap image)
*/

/*!
\brief ImagesetTabbedEditor

Binds all imageset editing functionality together

*/
class ImagesetTabbedEditor : public multi::MultiModeTabbedEditor
{
    typedef multi::MultiModeTabbedEditor super;
public:
    visual::VisualEditing* m_visual;
    code::CodeEditing* m_code;
    settings::declaration::Entry* m_tbIconSizeEntry;
    int m_subscribeID;

    ImagesetTabbedEditor(const QString& filePath);

    void initialise(mainwindow::MainWindow* mainWindow) override;

    void finalise() override;

    void rebuildEditorMenu(QMenu *editorMenu, bool &visible, bool &enabled) override;

    void activate() override;

    void updateToolbarSize(int size);

    void deactivate() override;

    bool saveAs(const QString& targetPath, bool updateCurrentPath = true);

    bool performCut() override;

    bool performCopy();

    bool performPaste();

    bool performDelete();

    bool zoomIn();

    bool zoomOut();

    bool zoomReset();
};


class ImagesetTabbedEditorFactory : public editors::TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Imageset";
    }

    QSet<QString> getFileExtensions() override;

    bool canEditFile(const QString& filePath) override;

    TabbedEditor* create(const QString& filePath);
};


} // namespace imageset
} // namespace editors
} // namespace CEED

#endif
