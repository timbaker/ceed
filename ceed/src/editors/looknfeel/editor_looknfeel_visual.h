/*
   created:    25th June 2014
   author:     Lukas E Meindl
*/

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

#ifndef CEED_editors_looknfeel_visual_
#define CEED_editors_looknfeel_visual_

#include "CEEDBase.h"

#include "resizable.h"

#include "cegui/cegui_widgethelpers.h"
#include "editors/editors_multi.h"

class QComboBox;
class QToolBar;

class Ui_LookNFeelEditorWidgetLookSelectorWidget;

namespace CEED {
namespace editors {
namespace looknfeel {
namespace visual {

class EditingScene;
class LookNFeelWidgetLookSelectorWidget;

/**This is the default visual editing mode

see ceed.editors.multi.EditMode
*/
class LookNFeelVisualEditing : public QWidget, multi::EditMode
{
public:
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    CEGUI::Window* m_rootWindow;
    hierarchy_dock_widget::LookNFeelHierarchyDockWidget *m_lookNFeelHierarchyDockWidget;
    LookNFeelWidgetLookSelectorWidget *m_lookNFeelWidgetLookSelectorWidget;
    falagard_element_editor::LookNFeelFalagardElementEditorDockWidget* m_falagardElementEditorDockWidget;
    EditingScene *m_scene;
    action::ConnectionGroup* m_connectionGroup;
    action::declaration::Action* m_focusElementEditorFilterBoxAction;
    QToolBar* m_toolBar;

    /**
    :param tabbedEditor: LookNFeelTabbedEditor
    :return:
    */
    LookNFeelVisualEditing(tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    void initialise();

    void destroy();

    void setupActions();

    void setupToolBar();

    /**Adds actions to the editor menu*/
    void rebuildEditorMenu(QMenu* editorMenu);

    /**
    Destroys all child windows of the root, which means that all preview windows of the selected WidgetLookFeel should be destroyed
    :return:
    */
    void destroyCurrentPreviewWidget();

    void updateWidgetLookPreview();

    void updateToNewTargetWidgetLook();

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    /**Focuses into element editor filter

    This potentially allows the user to just press a shortcut to find properties to edit,
    instead of having to reach for a mouse.
    */
    void focusElementEditorFilterBox();

    bool performCut();
};


/*!
\brief LookNFeelWidgetLookSelectorWidget

This dock widget allows to select a WidgetLook from a combobox and start editing it

*/
class LookNFeelWidgetLookSelectorWidget : public QDockWidget
{
public:
    visual::LookNFeelVisualEditing* m_visual;
    Ui_LookNFeelEditorWidgetLookSelectorWidget* m_ui;
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    QLabel *m_fileNameLabel;
    QComboBox* m_widgetLookNameBox;
    QPushButton* m_editWidgetLookButton;

    /**
    :param visual: LookNFeelVisualEditing
    :param tabbedEditor: LookNFeelTabbedEditor
    :return:
    */
    LookNFeelWidgetLookSelectorWidget(visual::LookNFeelVisualEditing* visual_, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);

    void resizeEvent(QResizeEvent *event) override;

    void slot_editWidgetLookButtonPressed();

    void populateWidgetLookComboBox(const QList<QPair<QString, QString> > &widgetLookNameTuples);

    void setFileNameLabel();
};


/*!
\brief EditingScene

This scene contains all the manipulators users want to interact it. You can visualise it as the
    visual editing centre screen where CEGUI is rendered.

    It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

*/
class EditingScene : public cegui::widgethelpers::GraphicsScene
{
public:
    LookNFeelVisualEditing* m_visual;
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    CEGUI::Window* m_rootWindow;
    hierarchy_dock_widget::LookNFeelHierarchyDockWidget* m_lookNFeelHierarchyDockWidget;
    LookNFeelWidgetLookSelectorWidget* m_lookNFeelWidgetLookSelectorWidget;
    falagard_element_editor::LookNFeelFalagardElementEditorDockWidget* m_falagardElementEditorDockWidget;
    bool m_ignoreSelectionChanges;

    EditingScene(visual::LookNFeelVisualEditing* visual_);

    void setCEGUIDisplaySize(int width, int height, bool lazyUpdate = true) override
    {
        // overridden to keep the manipulators in sync

        cegui::widgethelpers::GraphicsScene::setCEGUIDisplaySize(width, height, lazyUpdate);
    }

    void slot_selectionChanged();

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;
};

} // namespace visual
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
