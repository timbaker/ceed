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

#include "editors/text/editor_text_init.h"

#include "action/action__init__.h"

#include "mainwindow.h"

#include <QTextDocument>
#include <QTextEdit>

namespace CEED {
namespace editors {
namespace text {

TextTabbedEditor::TextTabbedEditor(const QString &filePath)
    : editors::TabbedEditor(nullptr, filePath)
{
    m_tabWidget = new QTextEdit();
    m_textDocument = nullptr;
}

void TextTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    editors::TabbedEditor::initialise(mainWindow);

    m_textDocument = new QTextDocument();

    QFile file(m_filePath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QByteArray data = file.readAll();
        m_textDocument->setPlainText(QString::fromUtf8(data));
    }

    dynamic_cast<QTextEdit*>(m_tabWidget)->setDocument(m_textDocument);
    m_textDocument->setModified(false);

    m_textDocument->setUndoRedoEnabled(true);
    QObject::connect(m_textDocument, &QTextDocument::undoAvailable, [=](bool available){ slot_undoAvailable(available); });
    QObject::connect(m_textDocument, &QTextDocument::redoAvailable, [=](bool available){ slot_redoAvailable(available); });

    QObject::connect(m_textDocument, &QTextDocument::contentsChanged, [=](){ slot_contentsChanged(); });
}

void TextTabbedEditor::finalise()
{
    editors::TabbedEditor::finalise();

    m_textDocument = nullptr;
}

bool TextTabbedEditor::hasChanges()
{
    return m_textDocument->isModified();
}

void TextTabbedEditor::undo()
{
    m_textDocument->undo();
}

void TextTabbedEditor::redo()
{
    m_textDocument->redo();
}

void TextTabbedEditor::slot_undoAvailable(bool available)
{
    m_mainWindow->m_undoAction->setEnabled(available);
}

void TextTabbedEditor::slot_redoAvailable(bool available)
{
    m_mainWindow->m_redoAction->setEnabled(available);
}

void TextTabbedEditor::slot_contentsChanged()
{
    markHasChanges(hasChanges());
}

bool TextTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    m_nativeData = m_textDocument->toPlainText();

    return TabbedEditor::saveAs(targetPath, updateCurrentPath);
}

/////

QSet<QString> TextTabbedEditorFactory::getFileExtensions()
{
    QSet<QString> extensions = {"py", "lua", "txt", "xml"};
    // this is just temporary, will go away when scheme, looknfeel and font editors are in place
    QSet<QString> temporaryExtensions = {"scheme", "font"};
    extensions += temporaryExtensions;
    return extensions;
}

bool TextTabbedEditorFactory::canEditFile(const QString &filePath)
{
    auto extensions = getFileExtensions();

    for (QString extension : extensions) {
        if (filePath.endsWith("." + extension))
            return true;
    }

    return false;
}


} // namespace text
} // namespace editors
} // namespace CEED
