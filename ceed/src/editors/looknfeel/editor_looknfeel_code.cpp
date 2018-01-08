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

#include "editors/looknfeel/tabbed_editor.h"
#include "editors/looknfeel/editor_looknfeel_visual.h"

#include "cegui/cegui_container.h"

#include "editor_looknfeel_code.h"
#include "mainwindow.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace code {

CodeEditing::CodeEditing(tabbed_editor::LookNFeelTabbedEditor *tabbedEditor):
    multi::CodeEditMode()
{
    m_tabbedEditor = tabbedEditor;
    m_highlighter = new WidgetLookHighlighter(this);
}

QString CodeEditing::getNativeCode()
{
    // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
    auto nameSet = m_tabbedEditor->getStringSetOfWidgetLookFeelNames();
    // We parse all WidgetLookFeels as XML to a string
    auto lookAndFeelString = CEGUI::WidgetLookManager::getSingleton().getWidgetLookSetAsString(nameSet);

    QString lookAndFeelStringQ = m_tabbedEditor->unmapWidgetLookReferences(TO_QSTR(lookAndFeelString));

    return lookAndFeelStringQ;
}

bool CodeEditing::propagateNativeCode(const QString &code)
{
    // we have to make the context the current context to ensure textures are fine
    mainwindow::MainWindow::instance->ceguiContainerWidget->makeGLContextCurrent();

    bool loadingSuccessful = m_tabbedEditor->tryUpdatingWidgetLookFeel(code);
    m_tabbedEditor->m_visual->updateToNewTargetWidgetLook();

    return loadingSuccessful;
}

void CodeEditing::moveToAndSelectWidgetLookFeel(const QString &widgetLookFeelName)
{
    QString wlfTagStartText = QString("<WidgetLook name=\"%1\"").arg(widgetLookFeelName);
    QString wlfTagEndText = "</WidgetLook>";

    // Move cursor to the start of the entire text
    moveCursor(QTextCursor::Start);

    // Find the beginning of the WidgetLookFeel element in the text
    bool textWasFound = find(wlfTagStartText);
    if (!textWasFound)
        return;

    // Retrieve the position of the cursor which points to the found text
    QTextCursor textCursor = this->textCursor();
    int startPos = textCursor.selectionStart();

    // Find the end of the WidgetLookFeel element in the text
    textWasFound = find(wlfTagEndText);
    if (!textWasFound)
        return;

    textCursor.setPosition(startPos);
    setTextCursor(textCursor);
}

void CodeEditing::refreshFromVisual()
{
    if (!m_tabbedEditor->m_targetWidgetLook.isEmpty()) {
        // Refresh the WidgetLookFeel Highlighter based on the new name of the WidgetLook
        QString originalWidgetLookName = m_tabbedEditor->unmapMappedNameIntoOriginalParts(m_tabbedEditor->m_targetWidgetLook).first;
        m_highlighter->updateWidgetLookRule(originalWidgetLookName);
    }

    multi::CodeEditMode::refreshFromVisual();

    if (!m_tabbedEditor->m_targetWidgetLook.isEmpty()) {
        QString originalWidgetLookName = m_tabbedEditor->unmapMappedNameIntoOriginalParts(m_tabbedEditor->m_targetWidgetLook).first;
        moveToAndSelectWidgetLookFeel(originalWidgetLookName);
    }
}


} // namespace code
} // namespace looknfeel
} // namespace editors
} // namespace CEED
