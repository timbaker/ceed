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

#ifndef CEED_editors_looknfeel_tabbed_editor_
#define CEED_editors_looknfeel_tabbed_editor_

#include "CEEDBase.h"

#include "cegui/ceguitypes.h"
#include "editors/editors_multi.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace tabbed_editor {

/*!
\brief LookNFeelTabbedEditor

Binds all Look n' Feel editing functionality together

*/
class LookNFeelTabbedEditor : public multi::MultiModeTabbedEditor
{
    typedef multi::MultiModeTabbedEditor super;
public:
    QString m_editorIDString;
    QString m_targetWidgetLook;
    visual::LookNFeelVisualEditing* m_visual;
    code::CodeEditing* m_code;
    QList<QPair<QString, QString>> m_nameMappingsOfOwnedWidgetLooks;
    preview::LookNFeelPreviewer* m_previewer;
    settings::declaration::Entry* m_tbIconSizeEntry;
    int m_subscribeID;

    LookNFeelTabbedEditor(const QString& filePath);

    void initialise(mainwindow::MainWindow* mainWindow);

    static
    QString getEditorIDStringPrefix()
    {
        return "ceed_internal-";
    }

    /**
    Returns the original WidgetLookFeel name and the editorID, based on a mapped name
    :param mappedName: str
    :return: str, str
    */
    static
    QPair<QString, QString> unmapMappedNameIntoOriginalParts(const QString& mappedName)
    {
        if (!mappedName.contains('/'))
            throw Exception("Failed to split the mapped name");

        QString mappedNameSplitResult0 = mappedName.section('/', 0, 0);
        QString mappedNameSplitResult1 = mappedName.section('/', 1);

        return { mappedNameSplitResult1, mappedNameSplitResult0 };
    }

    /**
    Tries to parse a LNF source code content
    :param sourceCode:
    :return:
    */
    bool tryUpdatingWidgetLookFeel(const QString& sourceCode);

    void mapAndLoadLookNFeelFileString(const QString &lookNFeelAsXMLString);

    /**
    Maps all occurances of WidgetLookFeel name references in an XML string to a new name based by prepending the editor's ID number
    :type lookNFeelString: str
    :return: str
    */
    QString mapWidgetLookReferences(const QString& lookNFeelString);

    /**
    Unmaps all occurances of mapped WidgetLookFeel name references in an XML string by removing the prepended editor ID number
    :type lookNFeelString: str
    :return: str
    */
    QString unmapWidgetLookReferences(const QString& lookNFeelString);

    void destroyOwnedWidgetLooks()
    {
        for (auto nameTuple : m_nameMappingsOfOwnedWidgetLooks)
            CEGUI::WidgetLookManager::getSingleton().eraseWidgetLook(FROM_QSTR(nameTuple.second));

        // We refresh the WidgetLook names
        refreshWidgetLookNameMappingTuples();
    }

    void refreshWidgetLookNameMappingTuples();

    void addMappedWidgetLookFalagardMappings();

    void removeOwnedWidgetLookFalagardMappings()
    {
        // Removes all FalagardMappings we previously added
        for (auto nameTuple : m_nameMappingsOfOwnedWidgetLooks)
            CEGUI::WindowFactoryManager::getSingleton().removeFalagardWindowMapping(FROM_QSTR(nameTuple.second));
    }

    void finalise()
    {
        super::finalise();
    }

    void destroy();

    void rebuildEditorMenu(QMenu *editorMenu, bool &visible, bool &enabled) override;

    void activate();

    void updateToolbarSize(int size);

    void deactivate();

    /**
    Returns a CEGUI::StringSet containing all (mapped) names of WidgetLookFeels that the file is associated with according to the editor
    :return: CEGUI::StringSet
    */
    CEGUI::WidgetLookFeel::StringSet getStringSetOfWidgetLookFeelNames();

    bool saveAs(const QString &targetPath, bool updateCurrentPath = true) override;

    bool performCut() override;

    bool performCopy();

    bool performPaste();

    bool performDelete();

    bool zoomIn();

    bool zoomOut();

    bool zoomReset();
};

} // namespace tabbed_editor
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
