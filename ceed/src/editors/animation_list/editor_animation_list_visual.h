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

#ifndef CEED_editors_animation_list_visual_
#define CEED_editors_animation_list_visual_

#include "CEEDBase.h"

#include "editors/animation_list/editor_animation_list_timeline.h" // for KeyFrameMoved

#include "elementtree.h"

#include "cegui/cegui_widgethelpers.h"
#include "editors/editors_multi.h"

#include <QDockWidget>
#include <QGraphicsView>

class Ui_AnimationListEditorDockWidget;
class Ui_AnimationListEditorKeyFramePropertiesDockWidget;
class Ui_TimelineDockWidget;
class Ui_AnimationListEditorVisualEditing;

class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QRadioButton;

namespace CEED {
namespace editors {
namespace animation_list {
namespace visual {

class AnimationDefinitionWrapper;
class TimelineDockWidget;

/*!
\brief AnimationListDockWidget

Lists animations in the currently opened animation list XML

*/
class AnimationListDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    Ui_AnimationListEditorDockWidget* m_ui;
    QListWidget* m_list;

    AnimationListDockWidget(VisualEditing* visual_);

    void fillWithAnimations(const QList<AnimationDefinitionWrapper *> &animationWrappers);

private slots:
    void slot_currentItemChanged(QListWidgetItem* newItem, QListWidgetItem* oldItem);
};

/*!
\brief PropertiesDockWidget

Lists and allows editing of properties at current position of the timeline

*/
class PropertiesDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    propertysetinspector::PropertyInspectorWidget* m_inspector;

    PropertiesDockWidget(VisualEditing* visual_);
};

/*!
\brief KeyFramePropertiesDockWidget

Lists and allows editing of properties at current position of the timeline

*/
class KeyFramePropertiesDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    Ui_AnimationListEditorKeyFramePropertiesDockWidget* m_ui;
    QDoubleSpinBox* m_timePositionSpinBox;
    QComboBox* m_progressionComboBox;
    QRadioButton* m_fixedValueRadioButton;
    QRadioButton* m_propertyRadioButton;
    QLineEdit* m_fixedValueLineEdit;
    QComboBox* m_sourcePropertyComboBox;
    CEGUI::KeyFrame* m_inspectedKeyFrame;

    KeyFramePropertiesDockWidget(VisualEditing* visual_);

    void setInspectedKeyFrame(CEGUI::KeyFrame *keyFrame);

    void syncSourcePropertyComboBox();

    void syncWithKeyFrame();
};


class TimelineGraphicsView : public QGraphicsView
{
public:
    TimelineDockWidget* m_dockWidget;

    TimelineGraphicsView(QWidget* parent = nullptr):
        QGraphicsView(parent)
    {
        m_dockWidget = nullptr;
    }

    void mouseReleaseEvent(QMouseEvent *event) override;
};


/*!
\brief TimelineDockWidget

Shows a timeline of currently selected animation (from the animation list dock widget)

*/
class TimelineDockWidget : public QDockWidget
{
public:
    VisualEditing* m_visual;
    Ui_TimelineDockWidget* m_ui;
    qreal m_zoomLevel;
    TimelineGraphicsView* m_view;
    QGraphicsScene* m_scene;
    timeline::AnimationTimeline* m_timeline;
    QPushButton* m_playButton;
    QPushButton* m_pauseButton;
    QPushButton* m_stopButton;

    TimelineDockWidget(VisualEditing* visual_);

    bool zoomIn();

    bool zoomOut();

    bool zoomReset();

    void slot_keyFramesMoved(const QList<timeline::KeyFrameMoved>& movedKeyFrames);
};

/*!
\brief EditingScene

This scene is used just to preview the animation in the state user selects.

*/
class EditingScene : public cegui::widgethelpers::GraphicsScene
{
public:
    VisualEditing* m_visual;

    EditingScene(VisualEditing* visual_);
};

/*!
\brief AnimationDefinitionWrapper

Represents one of the animations in the list, takes care of loading and saving it from/to XML element

*/
class AnimationDefinitionWrapper
{
public:
    VisualEditing* m_visual;
    CEGUI::Animation* m_animationDefinition;
    QString m_realDefinitionName;
    QString m_fakeDefinitionName;

    AnimationDefinitionWrapper(VisualEditing* visual_);

    void loadFromElement(ElementTree::Element* element);

    ElementTree::Element* saveToElement();
};

/**This is the default visual editing mode for animation lists

see editors.multi.EditMode
*/
class VisualEditing : public QWidget, public multi::EditMode
{
public:
    Ui_AnimationListEditorVisualEditing* m_ui;
    AnimationListTabbedEditor* m_tabbedEditor;
    AnimationListDockWidget* m_animationListDockWidget;
    PropertiesDockWidget* m_propertiesDockWidget;
    KeyFramePropertiesDockWidget* m_keyFramePropertiesDockWidget;
    TimelineDockWidget* m_timelineDockWidget;
    static int m_fakeAnimationDefinitionNameSuffix;
    CEGUI::Animation* m_currentAnimation;
    CEGUI::AnimationInstance* m_currentAnimationInstance;
    CEGUI::Window* m_currentPreviewWidget;
    CEGUI::Window* m_rootPreviewWidget;
    QComboBox* m_previewWidgetSelector;
    QWidget* m_ceguiPreview;
    EditingScene* m_scene;
    QMap<QString, AnimationDefinitionWrapper*> m_animationWrappers;

    VisualEditing(AnimationListTabbedEditor* tabbedEditor);

    QString generateFakeAnimationDefinitionName()
    {
        m_fakeAnimationDefinitionNameSuffix += 1;

        return QString("CEED_InternalAnimationDefinition_%1").arg(m_fakeAnimationDefinitionNameSuffix);
    }

    void showEvent(QShowEvent *event) override;

    void hideEvent(QHideEvent *event) override;

    void synchInstanceAndWidget();

    void loadFromElement(ElementTree::Element* rootElement);

    ElementTree::Element* saveToElement();

    QString generateNativeData();

    /**Set animation we want to edit*/
    void setCurrentAnimation(CEGUI::Animation *animation);

    void setCurrentAnimationWrapper(AnimationDefinitionWrapper* animationWrapper)
    {
        setCurrentAnimation(animationWrapper->m_animationDefinition);
    }

    AnimationDefinitionWrapper* getAnimationWrapper(const QString& name)
    {
        return m_animationWrappers[name];
    }

    CEGUI::Affector *getAffectorOfCurrentAnimation(int affectorIdx);

    CEGUI::KeyFrame *getKeyFrameOfCurrentAnimation(int affectorIdx, int keyFrameIdx);

    void setPreviewWidget(const QString& widgetType);

    void populateWidgetSelector();

    void slot_previewWidgetSelectorChanged(int index);

    void slot_timePositionChanged(float oldPosition, float newPosition);

    void slot_keyFrameSelectionChanged();

    bool zoomIn()
    {
        return m_timelineDockWidget->zoomIn();
    }

    bool zoomOut()
    {
        return m_timelineDockWidget->zoomOut();
    }

    bool zoomReset()
    {
        return m_timelineDockWidget->zoomReset();
    }
};

} // namespace undo
} // namespace animation_list
} // namespace editors
} // namespace CEED

#endif
