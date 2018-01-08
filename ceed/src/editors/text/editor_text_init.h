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

#ifndef CEED_editors_text___init___
#define CEED_editors_text___init___

#include "CEEDBase.h"

#include "editors/editors_init.h"

class QTextDocument;

namespace CEED {
namespace editors {
namespace text {

// TODO: This could get replaced by QScintilla once PySide guys get it to work.
//       Scintilla would probably be overkill though, I can't imagine anyone
//       doing any serious text editing in this application

/*!
\brief TextTabbedEditor

Multi purpose text editor

*/
class TextTabbedEditor : public editors::TabbedEditor
{
public:
    QTextDocument* m_textDocument;

    TextTabbedEditor(const QString& filePath);

    void initialise(mainwindow::MainWindow* mainWindow);

    void finalise() override;

    bool hasChanges() override;

    void undo() override;

    void redo() override;

    void slot_undoAvailable(bool available);

    void slot_redoAvailable(bool available);

    void slot_contentsChanged();

    bool saveAs(const QString& targetPath, bool updateCurrentPath = true) override;
};

class TextTabbedEditorFactory : public editors::TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Text";
    }

    QSet<QString> getFileExtensions() override;

    bool canEditFile(const QString& filePath) override;

    TabbedEditor* create(const QString& filePath) override
    {
        return new TextTabbedEditor(filePath);
    }
};

} // namespace text
} // namespace editors
} // namespace CEED

#endif
