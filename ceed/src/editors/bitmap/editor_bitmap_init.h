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

#ifndef CEED_editors_bitmap___init___
#define CEED_editors_bitmap___init___

#include "CEEDBase.h"

#include "editors/editors_init.h"

#include "ui_BitmapEditor.h"

namespace CEED {
namespace editors {
namespace bitmap {

/**A simple external bitmap editor starter/image viewer
*/
class BitmapTabbedEditor : public TabbedEditor, QWidget
{
public:
    Ui_BitmapEditor* m_ui;
    QLabel* m_preview;

    BitmapTabbedEditor(const QString& filePath)
        : TabbedEditor(nullptr, filePath)
        , QWidget()
    {
        m_ui = new Ui_BitmapEditor();
        m_ui->setupUi(this);

        m_tabWidget = this;
        m_preview = m_ui->preview;
    }

    void initialise(mainwindow::MainWindow* mainWindow) override
    {
        TabbedEditor::initialise(mainWindow);

        m_preview->setPixmap(QPixmap(m_filePath));
    }

    void finalise() override
    {
        TabbedEditor::finalise();
    }

    bool hasChanges() override
    {
        return false;
    }
};

class BitmapTabbedEditorFactory : public TabbedEditorFactory
{
public:
    QString getName() override
    {
        return "Bitmap";
    }

    QSet<QString> getFileExtensions() override
    {
        QStringList extensions =  { "png", "jpg", "jpeg", "tga", "dds" };
        return extensions.toSet();
    }


    bool canEditFile(const QString& filePath) override
    {
        QSet<QString> extensions = getFileExtensions();

        for (QString extension : extensions) {
            if (filePath.endsWith("." + extension)) {
                return true;
            }
        }

        return false;
    }

    TabbedEditor* create(const QString& filePath)
    {
        return new BitmapTabbedEditor(filePath);
    }
};

} // namespace bitmap
} // namespace editors
} // namespace CEED

#endif
