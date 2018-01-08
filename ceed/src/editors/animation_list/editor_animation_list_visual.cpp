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

#include "editor_animation_list_visual.h"

#include "cegui/cegui_init.h"
#include "cegui/cegui_container.h"

#include "editors/animation_list/editor_animation_list_init.h"
#include "editors/animation_list/editor_animation_list_timeline.h"
#include "editors/animation_list/editor_animation_list_undo.h"
#include "editors/animation_list/editor_animation_list_visual.h"

#include "ui_AnimationListEditorDockWidget.h"
#include "ui_AnimationListEditorKeyFramePropertiesDockWidget.h"
#include "ui_AnimationListEditorVisualEditing.h"
#include "ui_AnimationListEditorTimelineDockWidget.h"

#include "propertysetinspector.h"
#include "propertytree/propertytree_init.h"
#include "propertytree/propertytree_editors.h"
#include "propertytree/propertytree_properties.h"

#include "elementtree.h"
#include "mainwindow.h"
#include "project.h"
#include "xmledit.h"

namespace CEED {
namespace editors {
namespace animation_list {
namespace visual {

AnimationListDockWidget::AnimationListDockWidget(VisualEditing *visual_)
    : QDockWidget()
{
    m_visual = visual_;

    m_ui = new Ui_AnimationListEditorDockWidget();
    m_ui->setupUi(this);

    m_list = m_ui->list;
    connect(m_list, &QListWidget::currentItemChanged,
            this, &AnimationListDockWidget::slot_currentItemChanged);
}

void AnimationListDockWidget::fillWithAnimations(const QList<AnimationDefinitionWrapper *>& animationWrappers)
{
    m_list->clear();

    for (auto wrapper : animationWrappers)
        m_list->addItem(wrapper->m_realDefinitionName);
}

void AnimationListDockWidget::slot_currentItemChanged(QListWidgetItem *newItem, QListWidgetItem *oldItem)
{
    QString newName = newItem ? newItem->text() : "";
    QString oldName = oldItem ? oldItem->text() : "";

    auto cmd = new undo::ChangeCurrentAnimationDefinitionCommand(m_visual, newName, oldName);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

/////

PropertiesDockWidget::PropertiesDockWidget(VisualEditing *visual_)
    : QDockWidget()
{
    setObjectName("PropertiesDockWidget");
    m_visual = visual_;

    setWindowTitle("Property State");
    // Make the dock take as much space as it can vertically
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_inspector = new propertysetinspector::PropertyInspectorWidget();
    m_inspector->m_ptree->setupRegistry(
                new propertytree::editors::PropertyEditorRegistry(true));

    auto pmap = mainwindow::MainWindow::instance->m_project->m_propertyMap;
    m_inspector->setPropertyManager(new propertysetinspector::CEGUIPropertyManager(pmap));

    setWidget(m_inspector);
}

/////

KeyFramePropertiesDockWidget::KeyFramePropertiesDockWidget(VisualEditing *visual_)
    : QDockWidget()
{
    setObjectName("KeyFramePropertiesDockWidget");
    m_visual = visual_;

    m_ui = new Ui_AnimationListEditorKeyFramePropertiesDockWidget();
    m_ui->setupUi(this);

    m_timePositionSpinBox = m_ui->timePositionSpinBox;
    m_progressionComboBox = m_ui->progressionComboBox;

    m_fixedValueRadioButton = m_ui->fixedValueRadioButton;
    m_propertyRadioButton = m_ui->propertyRadioButton;

    m_fixedValueLineEdit = m_ui->fixedValueLineEdit;
    m_sourcePropertyComboBox = m_ui->sourcePropertyComboBox;

    m_inspectedKeyFrame = nullptr;
    setInspectedKeyFrame(nullptr);
}

void KeyFramePropertiesDockWidget::setInspectedKeyFrame(CEGUI::KeyFrame *keyFrame)
{
    setDisabled(keyFrame == nullptr);
    m_inspectedKeyFrame = keyFrame;

    syncWithKeyFrame();
}

void KeyFramePropertiesDockWidget::syncSourcePropertyComboBox()
{
    m_sourcePropertyComboBox->clear();

    // TODO
}

void KeyFramePropertiesDockWidget::syncWithKeyFrame()
{
    syncSourcePropertyComboBox();

    if (m_inspectedKeyFrame == nullptr) {
        m_timePositionSpinBox->setRange(0, 1);
        m_timePositionSpinBox->setValue(0);

        m_progressionComboBox->setCurrentIndex(m_progressionComboBox->findText("Linear"));

        m_fixedValueRadioButton->setChecked(true);
        m_propertyRadioButton->setChecked(false);

        m_fixedValueLineEdit->setText("");
        m_sourcePropertyComboBox->setCurrentIndex(-1);

    } else {
        Q_ASSERT(m_visual->m_currentAnimation != nullptr);
        m_timePositionSpinBox->setRange(0, m_visual->m_currentAnimation->getDuration());
        m_timePositionSpinBox->setValue(m_inspectedKeyFrame->getPosition());

        CEGUI::KeyFrame::Progression progression = m_inspectedKeyFrame->getProgression();
        if (progression == CEGUI::KeyFrame::Progression::P_Linear) {
            m_progressionComboBox->setCurrentIndex(m_progressionComboBox->findText("Linear"));
        } else if (progression == CEGUI::KeyFrame::Progression::P_QuadraticAccelerating) {
            m_progressionComboBox->setCurrentIndex(m_progressionComboBox->findText("Quadratic Accelerating"));
        } else if (progression == CEGUI::KeyFrame::Progression::P_QuadraticDecelerating) {
            m_progressionComboBox->setCurrentIndex(m_progressionComboBox->findText("Quadratic Decelerating"));
        } else if (progression == CEGUI::KeyFrame::Progression::P_Discrete) {
            m_progressionComboBox->setCurrentIndex(m_progressionComboBox->findText("Discrete"));
        } else {
            throw RuntimeError("Can't recognise progression of inspected keyframe");
        }

        if (m_inspectedKeyFrame->getSourceProperty() == "") {
            m_fixedValueRadioButton->setChecked(true);
            m_propertyRadioButton->setChecked(false);

            m_fixedValueLineEdit->setText(QString::fromUtf8((const char*)m_inspectedKeyFrame->getValue().data()));

            m_sourcePropertyComboBox->setCurrentIndex(-1);

        } else {
            m_fixedValueRadioButton->setChecked(false);
            m_propertyRadioButton->setChecked(true);

            m_fixedValueLineEdit->setText("");

            m_sourcePropertyComboBox->lineEdit()->setText(QString::fromUtf8((const char*)m_inspectedKeyFrame->getSourceProperty().data()));
        }
    }
}

/////

void TimelineGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    m_dockWidget->m_timeline->notifyMouseReleased();
}

/////

TimelineDockWidget::TimelineDockWidget(VisualEditing *visual_):
    QDockWidget()
{
    m_visual = visual_;

    m_ui = new Ui_TimelineDockWidget();
    m_ui->setupUi(this);

    m_zoomLevel = 1.0;

    m_view = m_ui->view;
    m_view->m_dockWidget = this;

    m_scene = new QGraphicsScene();
    m_timeline = new timeline::AnimationTimeline();
    connect(m_timeline, &timeline::AnimationTimeline::keyFramesMoved, this, &TimelineDockWidget::slot_keyFramesMoved);
    m_scene->addItem(m_timeline);
    m_view->setScene(m_scene);

    m_playButton = m_ui->playButton;
    connect(m_playButton, &QPushButton::clicked, [=](){ m_timeline->play(); });
    m_pauseButton = m_ui->pauseButton;
    connect(m_pauseButton, &QPushButton::clicked, [=](){ m_timeline->pause(); });
    m_stopButton = m_ui->stopButton;
    connect(m_stopButton, &QPushButton::clicked, [=](){ m_timeline->stop(); });
}

bool TimelineDockWidget::zoomIn()
{
    m_view->scale(2, 1);
    m_zoomLevel *= 2.0;

    m_timeline->notifyZoomChanged(m_zoomLevel);

    return true;
}

bool TimelineDockWidget::zoomOut()
{
    m_view->scale(0.5, 1);
    m_zoomLevel /= 2.0;

    m_timeline->notifyZoomChanged(m_zoomLevel);

    return true;
}

bool TimelineDockWidget::zoomReset()
{
    m_view->scale(1.0 / m_zoomLevel, 1);
    m_zoomLevel = 1.0;

    m_timeline->notifyZoomChanged(m_zoomLevel);

    return true;
}

void TimelineDockWidget::slot_keyFramesMoved(const QList<timeline::KeyFrameMoved> &movedKeyFrames)
{
    auto cmd = new undo::MoveKeyFramesCommand(m_visual, movedKeyFrames);
    m_visual->m_tabbedEditor->m_undoStack->push(cmd);
}

/////

EditingScene::EditingScene(VisualEditing *visual_):
    cegui::widgethelpers::GraphicsScene(mainwindow::MainWindow::instance->ceguiInstance)
{
    m_visual = visual_;
}

/////

AnimationDefinitionWrapper::AnimationDefinitionWrapper(VisualEditing *visual_)
{
    m_visual = visual_;
    m_animationDefinition = nullptr;
    // the real name that should be saved when saving to a file
    m_realDefinitionName = "";
    // the fake name that will never clash with anything
    m_fakeDefinitionName = "";
}

void AnimationDefinitionWrapper::loadFromElement(ElementTree::Element *element)
{
    m_realDefinitionName = element->get("name", "");
    m_fakeDefinitionName = m_visual->generateFakeAnimationDefinitionName();

    element->set("name", m_fakeDefinitionName);

    auto wrapperElement = new ElementTree::Element("Animations");
    wrapperElement->append(element);

    QString fakeWrapperCode = ElementTree::tostring(wrapperElement, "utf-8");
    // tidy up what we abused
    element->set("name", m_realDefinitionName);

    CEGUI::AnimationManager::getSingleton().loadAnimationsFromString(FROM_QSTR(fakeWrapperCode));

    m_animationDefinition = CEGUI::AnimationManager::getSingleton().getAnimation(FROM_QSTR(m_fakeDefinitionName));
}

/////

ElementTree::Element *AnimationDefinitionWrapper::saveToElement()
{
    CEGUI::String ceguiCode = CEGUI::AnimationManager::getSingleton().getAnimationDefinitionAsString(*m_animationDefinition);

    auto ret = ElementTree::fromstring(QString::fromUtf8((const char*)ceguiCode.data()));
    ret->set("name", m_realDefinitionName);

    return ret;
}

/////

int VisualEditing::m_fakeAnimationDefinitionNameSuffix = 1;

VisualEditing::VisualEditing(AnimationListTabbedEditor *tabbedEditor):
    multi::EditMode()
{
    m_ui = new Ui_AnimationListEditorVisualEditing();
    m_ui->setupUi(this);

    m_tabbedEditor = tabbedEditor;

    // the check and return is there because we require a project but are
    // constructed before the "project is opened" check is performed
    // if rootPreviewWidget == nullptr we will fail later, however that
    // won't happen since it will be checked after construction
    if (mainwindow::MainWindow::instance->m_project == nullptr) {
        return;
    }

    m_animationListDockWidget = new AnimationListDockWidget(this);
    m_propertiesDockWidget = new PropertiesDockWidget(this);
    m_keyFramePropertiesDockWidget = new KeyFramePropertiesDockWidget(this);

    m_timelineDockWidget = new TimelineDockWidget(this);
    connect(m_timelineDockWidget->m_timeline, &timeline::AnimationTimeline::timePositionChanged, this, &VisualEditing::slot_timePositionChanged);
    connect(m_timelineDockWidget->m_scene, &QGraphicsScene::selectionChanged, this, &VisualEditing::slot_keyFrameSelectionChanged);

//    m_fakeAnimationDefinitionNameSuffix = 1;
    m_currentAnimation = nullptr;
    m_currentAnimationInstance = nullptr;
    m_currentPreviewWidget = nullptr;

    setCurrentAnimation(nullptr);

    m_rootPreviewWidget = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "RootPreviewWidget");

    m_previewWidgetSelector = m_ui->previewWidgetSelector;
    connect(m_previewWidgetSelector, (void(QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &VisualEditing::slot_previewWidgetSelectorChanged);
    populateWidgetSelector();
    m_ceguiPreview = m_ui->ceguiPreview;

    auto layout = new QVBoxLayout(m_ceguiPreview);
    layout->setContentsMargins(0, 0, 0, 0);
    m_ceguiPreview->setLayout(layout);

    m_scene = new EditingScene(this);
}

void VisualEditing::showEvent(QShowEvent *event)
{
    mainwindow::MainWindow::instance->ceguiContainerWidget->activate(m_ceguiPreview, m_scene);
    mainwindow::MainWindow::instance->ceguiContainerWidget->setViewFeatures(true,
                                                                            true,
                                                                            true);

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootPreviewWidget);

    m_animationListDockWidget->setEnabled(true);
    m_timelineDockWidget->setEnabled(true);

    QWidget::showEvent(event);
}

void VisualEditing::hideEvent(QHideEvent *event)
{
    m_animationListDockWidget->setEnabled(false);
    m_timelineDockWidget->setEnabled(false);

    mainwindow::MainWindow::instance->ceguiContainerWidget->deactivate(m_ceguiPreview);

    QWidget::hideEvent(event);
}

void VisualEditing::synchInstanceAndWidget()
{
    if (m_currentAnimationInstance == nullptr) {
        m_propertiesDockWidget->m_inspector->setSource(QList<CEGUI::PropertySet*>());
        return;
    }

    m_currentAnimationInstance->setTargetWindow(nullptr);

    if (m_currentPreviewWidget == nullptr) {
        m_propertiesDockWidget->m_inspector->setSource(QList<CEGUI::PropertySet*>());
        return;
    }

    m_currentAnimationInstance->setTargetWindow(m_currentPreviewWidget);
    m_currentAnimationInstance->apply();

    QList<CEGUI::PropertySet*> sources = { m_currentPreviewWidget };
    if (m_propertiesDockWidget->m_inspector->getSources().toSet() != sources.toSet()) {
        m_propertiesDockWidget->m_inspector->setSource(sources);
    } else {
        m_propertiesDockWidget->m_inspector->m_propertyManager->updateAllValues(sources);
    }
}

void VisualEditing::loadFromElement(ElementTree::Element *rootElement)
{
    m_animationWrappers = {};

    for (auto animation : rootElement->findall("AnimationDefinition")) {
        auto animationWrapper = new AnimationDefinitionWrapper(this);
        animationWrapper->loadFromElement(animation);
        m_animationWrappers[animationWrapper->m_realDefinitionName] = animationWrapper;
    }

    m_animationListDockWidget->fillWithAnimations(m_animationWrappers.values());
}

ElementTree::Element *VisualEditing::saveToElement()
{
    auto root = new ElementTree::Element("Animations");

    for (auto animationWrapper : m_animationWrappers.values()) {
        auto element = animationWrapper->saveToElement();
        root->append(element);
    }

    return root;
}

QString VisualEditing::generateNativeData()
{
    auto element = saveToElement();
    xmledit::indent(element);

    return ElementTree::tostring(element, "utf-8");
}

void VisualEditing::setCurrentAnimation(CEGUI::Animation* animation)
{
    m_currentAnimation = animation;

    if (m_currentAnimationInstance != nullptr) {
        CEGUI::AnimationManager::getSingleton().destroyAnimationInstance(m_currentAnimationInstance);
        m_currentAnimationInstance = nullptr;
    }

    if (m_currentAnimation != nullptr) {
        m_currentAnimationInstance = CEGUI::AnimationManager::getSingleton().instantiateAnimation(m_currentAnimation);
    }

    m_timelineDockWidget->m_timeline->setAnimation(m_currentAnimation);

    synchInstanceAndWidget();
}

CEGUI::Affector* VisualEditing::getAffectorOfCurrentAnimation(int affectorIdx)
{
    return m_currentAnimation->getAffectorAtIdx(affectorIdx);
}

CEGUI::KeyFrame *VisualEditing::getKeyFrameOfCurrentAnimation(int affectorIdx, int keyFrameIdx)
{
    return getAffectorOfCurrentAnimation(affectorIdx)->getKeyFrameAtIdx(keyFrameIdx);
}

void VisualEditing::setPreviewWidget(const QString &widgetType)
{
    if (m_currentPreviewWidget != nullptr) {
        m_rootPreviewWidget->removeChild(m_currentPreviewWidget);
        CEGUI::WindowManager::getSingleton().destroyWindow(m_currentPreviewWidget);
        m_currentPreviewWidget = nullptr;
    }

    if (widgetType != "") {
        try {
            m_currentPreviewWidget = CEGUI::WindowManager::getSingleton().createWindow(widgetType.toStdString(), "PreviewWidget");

        } catch (Exception ex) {
            QMessageBox::warning(this, "Unable to comply!",
                                 QString("Your selected preview widget of type '%1' can't be used as a preview widget, error occured ('%2').")
                                 .arg(widgetType)
                                 .arg(QString::fromUtf8(ex.what())));
            m_currentPreviewWidget = nullptr;
            synchInstanceAndWidget();
            return;
        }

        m_currentPreviewWidget->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25, 0), CEGUI::UDim(0.25, 0)));
        m_currentPreviewWidget->setSize(CEGUI::USize(CEGUI::UDim(0.5, 0), CEGUI::UDim(0.5, 0)));
        m_rootPreviewWidget->addChild(m_currentPreviewWidget);
    }

    synchInstanceAndWidget();
}

void VisualEditing::populateWidgetSelector()
{
    m_previewWidgetSelector->clear();
    m_previewWidgetSelector->addItem(""); // no preview
    m_previewWidgetSelector->setCurrentIndex(0); // select no preview

    OrderedMap<QString, QStringList> widgetsBySkin = mainwindow::MainWindow::instance->ceguiInstance->getAvailableWidgetsBySkin();
    for (auto it = widgetsBySkin.begin(); it != widgetsBySkin.end(); it++) {
        QString skin = it.key();
        QStringList widgets = it.value();

        if (skin == "__no_skin__") {
            // pointless to preview animations with invisible widgets
            continue;
        }

        for (QString widget : widgets) {
            QString widgetType = skin + "/" + widget;
            m_previewWidgetSelector->addItem(widgetType);
        }
    }
}

void VisualEditing::slot_previewWidgetSelectorChanged(int index)
{
    QString widgetType = m_previewWidgetSelector->itemText(index);

    setPreviewWidget(widgetType);
}

void VisualEditing::slot_timePositionChanged(float oldPosition, float newPosition)
{
    // there is intentionally no undo/redo for this (it doesn't change content or context)

    if (m_currentAnimationInstance != nullptr) {
        m_currentAnimationInstance->setPosition(newPosition);
    }

    synchInstanceAndWidget();
}

void VisualEditing::slot_keyFrameSelectionChanged()
{
    QList<CEGUI::KeyFrame*> selectedKeyFrames;

    for (auto item_ : m_timelineDockWidget->m_scene->selectedItems()) {
        if (auto item = dynamic_cast<timeline::AffectorTimelineKeyFrame*>(item_)) {
            selectedKeyFrames.append(item->m_keyFrame);
        }
    }

    m_keyFramePropertiesDockWidget->setInspectedKeyFrame((selectedKeyFrames.length() == 1) ? selectedKeyFrames[0] : nullptr);
}


} // namespace undo
} // namespace animation_list
} // namespace editors
} // namespace CEED
