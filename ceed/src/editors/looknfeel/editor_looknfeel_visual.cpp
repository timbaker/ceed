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

#include "editor_looknfeel_visual.h"

#include "cegui/cegui_container.h"

#include "editors/looknfeel/tabbed_editor.h"
#include "editors/looknfeel/undoable_commands.h"

#include "editors/looknfeel/hierarchy_dock_widget.h"
#include "editors/looknfeel/falagard_element_editor.h"
#include "editors/looknfeel/falagard_element_inspector.h"

#include "mainwindow.h"
#include "project.h"

#include "ui_LookNFeelEditorHierarchyDockWidget.h"
#include "ui_LookNFeelEditorWidgetLookSelectorWidget.h"
#include "ui_LookNFeelEditorPropertyEditorDockWidget.h"

#include <QMenu>
#include <QToolBar>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace visual {

LookNFeelVisualEditing::LookNFeelVisualEditing(tabbed_editor::LookNFeelTabbedEditor *tabbedEditor)
    : QWidget()
{
    m_tabbedEditor = tabbedEditor;
    m_tabbedEditor->m_visual = this;
    m_rootWindow = nullptr;

    m_lookNFeelHierarchyDockWidget = new hierarchy_dock_widget::LookNFeelHierarchyDockWidget(this, tabbedEditor);
    m_lookNFeelWidgetLookSelectorWidget = new LookNFeelWidgetLookSelectorWidget(this, tabbedEditor);
    m_falagardElementEditorDockWidget = new falagard_element_editor::LookNFeelFalagardElementEditorDockWidget(this, tabbedEditor);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_scene = new EditingScene(this);

    setupActions();
    setupToolBar();

    m_lookNFeelHierarchyDockWidget->m_treeView->setupContextMenu();
#if 1 // moved from LookNFeelHierarchyDockWidget because setModel() calls LookNFeelHierarchyTreeView::selectionChanged()
    m_lookNFeelHierarchyDockWidget->m_treeView->setModel(m_lookNFeelHierarchyDockWidget->m_model);
    m_lookNFeelHierarchyDockWidget->updateToNewWidgetLook(m_tabbedEditor->m_targetWidgetLook);
#endif
}

void LookNFeelVisualEditing::initialise()
{
    auto propertyMap = mainwindow::MainWindow::instance->m_project->m_propertyMap;
    auto widgetLookPropertyManager = new falagard_element_inspector::FalagardElementAttributesManager(propertyMap, this);
    m_falagardElementEditorDockWidget->m_inspector->setPropertyManager(widgetLookPropertyManager);
    m_rootWindow = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "LookNFeelEditorRoot");
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootWindow);
}

void LookNFeelVisualEditing::destroy()
{
    // Remove the widget with the previous WidgetLook from the scene
    destroyCurrentPreviewWidget();
}

void LookNFeelVisualEditing::setupActions()
{
    m_connectionGroup = new action::ConnectionGroup(action::ActionManager::instance);

    m_focusElementEditorFilterBoxAction = action::getAction("looknfeel/focus_element_editor_filter_box");
    m_connectionGroup->add(m_focusElementEditorFilterBoxAction, [=](){ focusElementEditorFilterBox(); });
}

void LookNFeelVisualEditing::setupToolBar()
{
    m_toolBar = new QToolBar("looknfeel");
    m_toolBar->setObjectName("looknfeelToolbar");
    m_toolBar->setIconSize(QSize(32, 32));
}

void LookNFeelVisualEditing::rebuildEditorMenu(QMenu *editorMenu)
{
    editorMenu->addAction(m_focusElementEditorFilterBoxAction);
}

void LookNFeelVisualEditing::destroyCurrentPreviewWidget()
{
    if (m_rootWindow == nullptr)
        return;

    // Remove the widget with the previous WidgetLook from the scene
    while (m_rootWindow->getChildCount() != 0)
        CEGUI::WindowManager::getSingleton().destroyWindow(m_rootWindow->getChildAtIdx(0));

    // TODO (Ident) :
    // Fix the window pool cleanup issues in CEGUI default and remove this later in the CEED default branch
    // for more info see: http://cegui.org.uk/wiki/The_Lederhosen_project_-_The_Second_Coming
    CEGUI::WindowManager::getSingleton().cleanDeadPool();
}

void LookNFeelVisualEditing::updateWidgetLookPreview()
{
    destroyCurrentPreviewWidget();

    if (!m_tabbedEditor->m_targetWidgetLook.isEmpty()) {
        // Add new widget representing the new WidgetLook to the scene, if the factory is registered
        bool factoryPresent = CEGUI::WindowFactoryManager::getSingleton().isFactoryPresent(FROM_QSTR(m_tabbedEditor->m_targetWidgetLook));
        if (factoryPresent) {
            auto widgetLookWindow = CEGUI::WindowManager::getSingleton().createWindow(FROM_QSTR(m_tabbedEditor->m_targetWidgetLook), "WidgetLookWindow");
            m_rootWindow->addChild(widgetLookWindow);
        } else {
            m_tabbedEditor->m_targetWidgetLook = "";
        }
    }

    //Refresh the drawing of the preview
    m_scene->update();
}

void LookNFeelVisualEditing::updateToNewTargetWidgetLook()
{
    updateWidgetLookPreview();

    m_falagardElementEditorDockWidget->m_inspector->setSource(FalagardElement());

    m_lookNFeelHierarchyDockWidget->updateToNewWidgetLook(m_tabbedEditor->m_targetWidgetLook);
}

void LookNFeelVisualEditing::showEvent(QShowEvent *event)
{
    mainwindow::MainWindow::instance->ceguiContainerWidget->activate(this, m_scene);
    mainwindow::MainWindow::instance->ceguiContainerWidget->setViewFeatures(true,
                                                                           true,
                                                                           settings::getEntry("looknfeel/visual/continuous_rendering")->m_value.toBool());

    m_lookNFeelHierarchyDockWidget->setEnabled(true);
    m_lookNFeelWidgetLookSelectorWidget->setEnabled(true);
    m_falagardElementEditorDockWidget->setEnabled(true);

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootWindow);

    m_toolBar->setEnabled(true);
    if (m_tabbedEditor->editorMenu() != nullptr)
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(true);

    // connect all our actions
    m_connectionGroup->connectAll();

    QWidget::showEvent(event);
}

void LookNFeelVisualEditing::hideEvent(QHideEvent *event)
{
    // disconnected all our actions
    m_connectionGroup->disconnectAll();

    m_lookNFeelHierarchyDockWidget->setEnabled(false);
    m_lookNFeelWidgetLookSelectorWidget->setEnabled(false);
    m_falagardElementEditorDockWidget->setEnabled(false);

    m_toolBar->setEnabled(false);
    if (m_tabbedEditor->editorMenu() != nullptr)
        m_tabbedEditor->editorMenu()->menuAction()->setEnabled(false);

    mainwindow::MainWindow::instance->ceguiContainerWidget->deactivate(this);

    QWidget::hideEvent(event);
}

void LookNFeelVisualEditing::focusElementEditorFilterBox()
{
    auto filterBox = m_falagardElementEditorDockWidget->m_inspector->m_filterBox;
    // selects all contents of the filter so that user can replace that with their search phrase
    filterBox->selectAll();
    // sets focus so that typing puts text into the filter box without clicking
    filterBox->setFocus();
}

bool LookNFeelVisualEditing::performCut()
{
#if 1 // TODO
    return false;
#else
    bool ret = performCopy();
    m_scene->deleteSelectedWidgets();

    return ret;
#endif
}

/////

LookNFeelWidgetLookSelectorWidget::LookNFeelWidgetLookSelectorWidget(LookNFeelVisualEditing *visual_, tabbed_editor::LookNFeelTabbedEditor *tabbedEditor)
    : QDockWidget()
{
    m_tabbedEditor = tabbedEditor;

    m_visual = visual_;

    m_ui = new Ui_LookNFeelEditorWidgetLookSelectorWidget();
    m_ui->setupUi(this);

    m_fileNameLabel = m_ui->fileNameLabel;

    m_widgetLookNameBox = m_ui->widgetLookNameBox;

    m_editWidgetLookButton = m_ui->editWidgetLookButton;
    connect(m_editWidgetLookButton, &QPushButton::pressed, this, &LookNFeelWidgetLookSelectorWidget::slot_editWidgetLookButtonPressed);
}

void LookNFeelWidgetLookSelectorWidget::resizeEvent(QResizeEvent *event)
{
    setFileNameLabel();

    QDockWidget::resizeEvent(event);
}

void LookNFeelWidgetLookSelectorWidget::slot_editWidgetLookButtonPressed()
{
    // Handles the actions necessary after a user selects a new WidgetLook to edit
    int selectedItemIndex = m_widgetLookNameBox->currentIndex();
    QString selectedWidgetLookName = m_widgetLookNameBox->itemData(selectedItemIndex).toString();

    auto command = new undoable_commands::TargetWidgetChangeCommand(m_visual, m_tabbedEditor, selectedWidgetLookName);
    m_tabbedEditor->m_undoStack->push(command);
}

void LookNFeelWidgetLookSelectorWidget::populateWidgetLookComboBox(const QList<QPair<QString, QString> > &widgetLookNameTuples)
{
    m_widgetLookNameBox->clear();
    // We populate the combobox with items that use the original name as display text but have the name of the live-editable WidgetLook stored as a QVariant

    for (auto nameTuple : widgetLookNameTuples)
        m_widgetLookNameBox->addItem(nameTuple.first, nameTuple.second);
}

void LookNFeelWidgetLookSelectorWidget::setFileNameLabel()
{
    // Shortens the file name so that it fits into the label and ends with "...", the full path is set as a tooltip
    QString fileNameStr = m_tabbedEditor->m_filePath;
    QFontMetrics fontMetrics = m_fileNameLabel->fontMetrics();
    int labelWidth = m_fileNameLabel->size().width();
    int fontMetricsWidth = fontMetrics.width(fileNameStr);
    int rightMargin = 6;
    if (labelWidth < fontMetricsWidth)
        m_fileNameLabel->setText(fontMetrics.elidedText(fileNameStr, Qt::ElideRight, labelWidth - rightMargin));
    else
        m_fileNameLabel->setText(fileNameStr);

    m_fileNameLabel->setToolTip(fileNameStr);
}

/////

EditingScene::EditingScene(LookNFeelVisualEditing *visual_)
    : cegui::widgethelpers::GraphicsScene(mainwindow::MainWindow::instance->ceguiInstance)
{
    m_visual = visual_;
    m_ignoreSelectionChanges = false;
    connect(this, &EditingScene::selectionChanged, this, &EditingScene::slot_selectionChanged);
}

void EditingScene::slot_selectionChanged()
{
#if 0 // FIXME: this does nothing
    auto selection = selectedItems();

    QList<cegui::widgethelpers::Manipulator*> sets;
    for (auto item_ : selection) {
        cegui::widgethelpers::Manipulator* wdt = nullptr;

        if (auto item = dynamic_cast<cegui::widgethelpers::Manipulator*>())
            wdt = item->m_widget;

        else if (auto resizable_ = dynamic_cast<resizable::ResizingHandle*>(item)) {
            if (auto item = dynamic_cast<cegui::widgethelpers::Manipulator*>(resizable_->m_parentResizable)) {
                wdt = item->m_widget;
            }
        }

        if (wdt != nullptr && !sets.contains(wdt))
            sets.append(wdt);
    }
#endif
}

void EditingScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#if 0 // TODO
    cegui::widgethelpers::GraphicsScene::mouseReleaseEvent(event);

    QStringList movedWidgetPaths;
    QMap<QString, QPointF> movedOldPositions;
    QMap<QString, QPointF> movedNewPositions;

    QStringList resizedWidgetPaths;
    QMap<QString, QPointF> resizedOldPositions;
    QMap<QString, QSizeF> resizedOldSizes;
    QMap<QString, QPointF> resizedNewPositions;
    QMap<QString, QSizeF> resizedNewSizes;

    // we have to "expand" the items, adding parents of resizing handles
    // instead of the handles themselves
    QList<cegui::widgethelpers::Manipulator*> expandedSelectedItems;
    for (auto selectedItem : selectedItems()) {
        if (auto manipulator = dynamic_cast<cegui::widgethelpers::Manipulator*>(selectedItem))
            expandedSelectedItems.append(manipulator);
        else if (auto resizingHandle = dynamic_cast<resizable::ResizingHandle*>(selectedItem)) {
            if (auto manipulator = dynamic_cast<cegui::widgethelpers::Manipulator*>(selectedItem->parentItem()))
                expandedSelectedItems.append(manipulator);
        }
    }

    for (auto item : expandedSelectedItems) {
        if (isinstance(item, widgethelpers.Manipulator)) {
            if (item.preMovePos != nullptr) {
                widgetPath = item.widget.getNamePath();
                movedWidgetPaths.append(widgetPath);
                movedOldPositions[widgetPath] = item.preMovePos;
                movedNewPositions[widgetPath] = item.widget.getPosition();

                // it won't be needed anymore so we use this to mark we picked this item up
                item.preMovePos = None;
            }

            if (item.preResizePos != nullptr and item.preResizeSize != nullptr) {
                widgetPath = item.widget.getNamePath();
                resizedWidgetPaths.append(widgetPath);
                resizedOldPositions[widgetPath] = item.preResizePos;
                resizedOldSizes[widgetPath] = item.preResizeSize;
                resizedNewPositions[widgetPath] = item.widget.getPosition();
                resizedNewSizes[widgetPath] = item.widget.getSize();

                // it won't be needed anymore so we use this to mark we picked this item up
                item.preResizePos = None;
                item.preResizeSize = None;
            }
        }
    }

    if (!movedWidgetPaths.isEmpty()) {
        auto cmd = new undoable_commands::MoveCommand(m_visual, movedWidgetPaths, movedOldPositions, movedNewPositions);
        m_visual.tabbedEditor.undoStack.push(cmd);
    }

    if (resizedWidgetPaths.isEmpty()) {
        auto cmd = new undoable_commands::ResizeCommand(m_visual, resizedWidgetPaths, resizedOldPositions, resizedOldSizes, resizedNewPositions, resizedNewSizes);
        m_visual.tabbedEditor.undoStack.push(cmd);
    }
#endif
}

void EditingScene::keyReleaseEvent(QKeyEvent *event)
{
#if 0
    bool handled = false;

    if (event.key() == Qt::Key_Delete)
        handled = deleteSelectedWidgets();

    if (!handled)
        cegui::widgethelpers::GraphicsScene::keyReleaseEvent(event);

    else
        event.accept();
#endif
}


} // namespace visual
} // namespace looknfeel
} // namespace editors
} // namespace CEED
