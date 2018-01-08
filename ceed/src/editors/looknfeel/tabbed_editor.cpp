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

#include "tabbed_editor.h"

#include "editors/looknfeel/falagard_element_editor.h"
#include "editors/looknfeel/hierarchy_dock_widget.h"
#include "editors/looknfeel/editor_looknfeel_visual.h"
#include "editors/looknfeel/editor_looknfeel_code.h"
#include "editors/looknfeel/editor_looknfeel_preview.h"

#include "settings/settings_init.h"

#include "cegui/cegui_container.h"
#include "mainwindow.h"
#include "messages.h"

#include "compatibility/looknfeel/compat_looknfeel_init.h"

#include <QMenu>
#include <QToolBar>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace tabbed_editor {

LookNFeelTabbedEditor::LookNFeelTabbedEditor(const QString &filePath):
    multi::MultiModeTabbedEditor(compatibility::looknfeel::manager, filePath)
{
    messages::warning(nullptr, this, "LookNFeel Editor is experimental!",
                      "This part of CEED is not considered to be ready for "
                      "production. You have been warned. If everything "
                      "breaks you get to keep the pieces!",
                      "looknfeel_editor_experimental");

    m_editorIDString = getEditorIDStringPrefix() + QString::number((qulonglong)this); // was id(self) in Python

    m_requiresProject = true;

    // The name of the widget we are targeting for editing
    m_targetWidgetLook = "";

    m_visual = new visual::LookNFeelVisualEditing(this);
    addTab(m_visual, "Visual");

    m_code = new code::CodeEditing(this);
    addTab(m_code, "Code");

//    m_nameMappingsOfOwnedWidgetLooks = [];

    // Look n' Feel Previewer is not actually an edit mode, you can't edit the Look n' Feel from it,
    // however for everything to work smoothly we do push edit mode changes to it to the
    // undo stack.
    //
    // TODO: This could be improved at least a little bit if 2 consecutive edit mode changes
    //       looked like this: A->Preview, Preview->C.  We could simply turn this into A->C,
    //       and if A = C it would eat the undo command entirely.
    m_previewer = new preview::LookNFeelPreviewer(this);
    addTab(m_previewer, "Live Preview");

    m_tabWidget = this;

    // set the toolbar icon size according to the setting and subscribe to it
    m_tbIconSizeEntry = settings::getEntry("global/ui/toolbar_icon_size");
    updateToolbarSize(m_tbIconSizeEntry->m_value.toInt());
    m_subscribeID = m_tbIconSizeEntry->subscribe([=](const QVariant& v){ updateToolbarSize(v.toInt()); });
}

void LookNFeelTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    super::initialise(mainWindow);

    // we have to make the context the current context to ensure textures are fine
    m_mainWindow->ceguiContainerWidget->makeGLContextCurrent();

    mapAndLoadLookNFeelFileString(m_nativeData);

    m_visual->initialise();
}

bool LookNFeelTabbedEditor::tryUpdatingWidgetLookFeel(const QString &sourceCode)
{
    bool loadingSuccessful = true;
    try {
        mapAndLoadLookNFeelFileString(sourceCode);
    } catch (...) {
        mapAndLoadLookNFeelFileString(m_nativeData);
        loadingSuccessful = false;
    }

    return loadingSuccessful;
}

void LookNFeelTabbedEditor::mapAndLoadLookNFeelFileString(const QString& lookNFeelAsXMLString)
{
    // When we are loading a Look n' Feel file we want to load it into CEED in a way it doesn't collide with other LNF definitions stored into CEGUI.
    // To prevent name collisions and also to prevent live-editing of WidgetLooks that are used somewhere in a layout editor or other look n' feel editor simultaneously,
    // we will map the names that we load from a Look n' Feel file in a way that they are unique. We achieve this by editing the WidgetLook names inside the string we loaded
    // from the .looknfeel file, so that the LNF Editor instance's python ID will be prepended to the name. ( e.g.: Vanilla/Button will turn into 18273822/Vanilla/Button )
    // Each Editor is associated with only one LNF file so that this will in effect also guarantee that the WidgetLooks inside the CEGUI system will be uniquely named
    // for each file

    if (lookNFeelAsXMLString.isEmpty())
        return;

    // Mapping all occuring references
    QString modifiedLookNFeelString = mapWidgetLookReferences(lookNFeelAsXMLString);

    // We remove all WidgetLook mappings
    removeOwnedWidgetLookFalagardMappings();

    // We erase all widgetLooks
    destroyOwnedWidgetLooks();

    try {
        // Parsing the resulting Look n' Feel
        CEGUI::WidgetLookManager::getSingleton().parseLookNFeelSpecificationFromString(FROM_QSTR(modifiedLookNFeelString));
    } catch (...) {
        // We retrieve a list of all newly loaded WidgetLook names (as tuples of original and new name) that we just mapped for this editor
        refreshWidgetLookNameMappingTuples();
        // We erase all widgetLooks
        destroyOwnedWidgetLooks();
        // Refreshing the combobox
        m_visual->m_lookNFeelWidgetLookSelectorWidget->populateWidgetLookComboBox(m_nameMappingsOfOwnedWidgetLooks);
        // We raise the error again so we can process it in the try-block outside the function
        throw;
    }

    // We retrieve a list of all WidgetLook names (as tuples of original and new name) that we just mapped for this editor
    refreshWidgetLookNameMappingTuples();
    // We look for falagard mappings and add them
    addMappedWidgetLookFalagardMappings();
    // Refreshing the combobox
    m_visual->m_lookNFeelWidgetLookSelectorWidget->populateWidgetLookComboBox(m_nameMappingsOfOwnedWidgetLooks);
}

QString LookNFeelTabbedEditor::mapWidgetLookReferences(const QString &lookNFeelString_)
{
    QString lookNFeelString = lookNFeelString_;

    // Modifying the string using regex
    QString regexPattern = "<\\s*WidgetLook\\sname\\s*=\\s*\"";
    QString replaceString = "<WidgetLook name=\"" + m_editorIDString + "/";
    QString modifiedLookNFeelString = lookNFeelString.replace(QRegExp(regexPattern), replaceString);

    regexPattern = "look\\s*=\\s*\"";
    replaceString = "look=\"" + m_editorIDString + "/";
    modifiedLookNFeelString = modifiedLookNFeelString.replace(QRegExp(regexPattern), replaceString);

    return modifiedLookNFeelString;
}

QString LookNFeelTabbedEditor::unmapWidgetLookReferences(const QString &lookNFeelString_)
{
    QString lookNFeelString = lookNFeelString_;

    // Modifying the string using regex
    QString regexPattern = "name=\"" + m_editorIDString + "/";
    QString replaceString = "name=\"";
    QString modifiedLookNFeelString = lookNFeelString.replace(QRegExp(regexPattern), replaceString);

    regexPattern = "look=\"" + m_editorIDString + "/";
    replaceString = "look=\"";
    modifiedLookNFeelString = modifiedLookNFeelString.replace(QRegExp(regexPattern), replaceString);

    return modifiedLookNFeelString;
}

void LookNFeelTabbedEditor::refreshWidgetLookNameMappingTuples()
{
    // Delete all previous entries
    // Returns an array containing tuples of the original WidgetLook name and the mapped one
    m_nameMappingsOfOwnedWidgetLooks.clear();

    CEGUI::WidgetLookManager::WidgetLookPointerMap widgetLookMap = CEGUI::WidgetLookManager::getSingleton().getWidgetLookPointerMap();

    for (auto it = widgetLookMap.begin(); it != widgetLookMap.end(); it++) {
        CEGUI::String widgetLookEditModeName = it->first;
        auto p1 = unmapMappedNameIntoOriginalParts(TO_QSTR(widgetLookEditModeName));
        QString widgetLookOriginalName = p1.first;
        QString widgetLookEditorID = p1.second;

        if (widgetLookEditorID == m_editorIDString) {
            QPair<QString, QString> widgetLookNameTuple = { widgetLookOriginalName, TO_QSTR(widgetLookEditModeName) };
            m_nameMappingsOfOwnedWidgetLooks.append(widgetLookNameTuple);
        }
    }

    std::sort(m_nameMappingsOfOwnedWidgetLooks.begin(), m_nameMappingsOfOwnedWidgetLooks.end());
}

void LookNFeelTabbedEditor::addMappedWidgetLookFalagardMappings()
{
    // We have to "guess" at least one FalagardWindowMapping - we have to keep in mind that there could theoretically be multiple window mappings for one WidgetLook -  ( which
    // contains a targetType and a renderer ) for our WidgetLook so we can display it.
    // If the user has already loaded .scheme files then we can use the WindowFactoryManager for this purpose:
    for (auto nameTuple : m_nameMappingsOfOwnedWidgetLooks) {

        auto falagardMappingIter = CEGUI::WindowFactoryManager::getSingleton().getFalagardMappingIterator();
        while (!falagardMappingIter.isAtEnd()) {
            auto falagardMapping = falagardMappingIter.getCurrentValue();
            if (falagardMapping.d_lookName == FROM_QSTR(nameTuple.first)) {
                CEGUI::WindowFactoryManager::getSingleton().addFalagardWindowMapping(FROM_QSTR(nameTuple.second), falagardMapping.d_baseType,
                                                                                     FROM_QSTR(nameTuple.second), falagardMapping.d_rendererType);
            }
            falagardMappingIter++;
        }
    }
}

void LookNFeelTabbedEditor::destroy()
{
    m_visual->destroy();

    // Remove all FalagardMappings we added
    removeOwnedWidgetLookFalagardMappings();

    // Erase all mapped WidgetLooks we added
    destroyOwnedWidgetLooks();

    // unsubscribe from the toolbar icon size setting
    m_tbIconSizeEntry->unsubscribe(m_subscribeID);

    TabbedEditor::destroy(); // or QWidget::destroy() ?
}

void LookNFeelTabbedEditor::rebuildEditorMenu(QMenu *editorMenu, bool &visible, bool &enabled)
{
    editorMenu->setTitle("&Look and Feel");
    m_visual->rebuildEditorMenu(editorMenu);

    visible = true;
    enabled = currentWidget() == m_visual;
}

void LookNFeelTabbedEditor::activate()
{
    super::activate();

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_lookNFeelWidgetLookSelectorWidget);
    m_visual->m_lookNFeelWidgetLookSelectorWidget->setVisible(true);

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_lookNFeelHierarchyDockWidget);
    m_visual->m_lookNFeelHierarchyDockWidget->setVisible(true);

    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_visual->m_falagardElementEditorDockWidget);
    m_visual->m_falagardElementEditorDockWidget->setVisible(true);

    m_mainWindow->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_visual->m_toolBar);
    m_visual->m_toolBar->show();

    m_visual->m_lookNFeelWidgetLookSelectorWidget->setFileNameLabel();
}

void LookNFeelTabbedEditor::updateToolbarSize(int size)
{
    if (size < 16)
        size = 16;
    m_visual->m_toolBar->setIconSize(QSize(size, size));
}

void LookNFeelTabbedEditor::deactivate()
{
    m_mainWindow->removeDockWidget(m_visual->m_lookNFeelHierarchyDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_lookNFeelWidgetLookSelectorWidget);
    m_mainWindow->removeDockWidget(m_visual->m_falagardElementEditorDockWidget);

    m_mainWindow->removeToolBar(m_visual->m_toolBar);

    super::deactivate();
}

CEGUI::WidgetLookFeel::StringSet LookNFeelTabbedEditor::getStringSetOfWidgetLookFeelNames()
{
    // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
    CEGUI::WidgetLookFeel::StringSet nameSet;
    for (auto nameTuple : m_nameMappingsOfOwnedWidgetLooks)
        nameSet.insert(FROM_QSTR(nameTuple.second));

    return nameSet;
}

bool LookNFeelTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    bool codeMode = currentWidget() == m_code;

    // if user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)

    if (codeMode)
        m_code->propagateToVisual();

    // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
    auto nameSet = getStringSetOfWidgetLookFeelNames();
    // We parse all WidgetLookFeels as XML to a string
    CEGUI::String lookAndFeelString = CEGUI::WidgetLookManager::getSingleton().getWidgetLookSetAsString(nameSet);
    m_nativeData = unmapWidgetLookReferences(TO_QSTR(lookAndFeelString));

    return super::saveAs(targetPath, updateCurrentPath);
}

bool LookNFeelTabbedEditor::performCut()
{
    if (currentWidget() == m_visual)
        return m_visual->performCut();

    return false;
}

bool LookNFeelTabbedEditor::performCopy()
{
#if 0
    if (currentWidget() == m_visual)
        return m_visual->performCopy();
#endif
    return false;
}

bool LookNFeelTabbedEditor::performPaste()
{
#if 0
    if (currentWidget() == m_visual)
        return m_visual->performPaste();
#endif
    return false;
}

bool LookNFeelTabbedEditor::performDelete()
{
#if 0
    if (currentWidget() == m_visual)
        return m_visual->performDelete();
#endif
    return false;
}

bool LookNFeelTabbedEditor::zoomIn()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomIn();
    return false;
}

bool LookNFeelTabbedEditor::zoomOut()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomOut();
    return false;
}

bool LookNFeelTabbedEditor::zoomReset()
{
    if (currentWidget() == m_visual)
        dynamic_cast<cegui::qtgraphics::GraphicsView*>(m_visual->m_scene->views()[0])->zoomOriginal();
    return false;
}

} // namespace tabbed_editor
} // namespace looknfeel
} // namespace editors
} // namespace CEED
