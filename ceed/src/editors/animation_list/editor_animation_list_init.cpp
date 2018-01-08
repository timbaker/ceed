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

#include "editors/animation_list/editor_animation_list_init.h"

#include "editors/animation_list/editor_animation_list_code.h"

#include "ceed_paths.h"
#include "mainwindow.h"

namespace CEED {
namespace editors {
namespace animation_list {

AnimationListTabbedEditor::AnimationListTabbedEditor(const QString &filePath)
    : super(compatibility::animation_list::manager, filePath)
{
    messages::warning(nullptr, this, "Animation List Editor is experimental!",
                      "This part of CEED is not considered to be ready for "
                      "production. You have been warned. If everything "
                      "breaks you get to keep the pieces!",
                      "animation_list_editor_experimental");

    m_requiresProject = true;

    m_visual = new visual::VisualEditing(this);
    addTab(m_visual, "Visual");

    m_code = new code::CodeEditing(this);
    addTab(m_code, "Code");

    m_tabWidget = this;
}

void AnimationListTabbedEditor::initialise(mainwindow::MainWindow *mainWindow)
{
    super::initialise(mainWindow);

    // We do something most people would not expect here,
    // instead of asking CEGUI to load the animation list as it is,
    // we parse it ourself, mine each and every animation from it,
    // and use these chunks of code to load every animation one at a time

    // the reason we do this is more control (CEGUI just adds the animation
    // list to the pool of existing animations, we don't want to pollute that
    // pool and we want to group all loaded animations)

    ElementTree::Element* root = nullptr;
    try {
        root = ElementTree::fromstring(m_nativeData);

    } catch (...) {
        // things didn't go smooth
        // 2 reasons for that
        //  * the file is empty
        //  * the contents of the file are invalid
        //
        // In the first case we will silently move along (it is probably just a new file),
        // in the latter we will output a message box informing about the situation

        // the file should exist at this point, so we are not checking and letting exceptions
        // fly out of this method
        if (os.path.getsize(m_filePath) > 2) {
            // the file contains more than just CR LF
            QMessageBox::question(this,
                                  "Can't parse given animation list!",
                                  QString("Parsing '%1' failed, it's most likely not a valid XML file. "
                                          "Constructing empty animation list instead (if you save you will override the invalid data!). "
                                          "Exception details follow:\n%2").arg(m_filePath).arg("sys.exc_info()[1]"),
                                  QMessageBox::Ok);
        }

        // we construct the minimal empty imageset
        root = new ElementTree::Element("Animations");
    }

    m_visual->loadFromElement(root);
}

void AnimationListTabbedEditor::finalise()
{
    // this takes care of destroying the temporary animation instance, if any
    m_visual->setCurrentAnimation(nullptr);

    super::finalise();
}

void AnimationListTabbedEditor::activate()
{
    super::activate();

    m_mainWindow->addDockWidget(Qt::LeftDockWidgetArea, m_visual->m_animationListDockWidget);
    m_visual->m_animationListDockWidget->setVisible(true);
    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_visual->m_propertiesDockWidget);
    m_visual->m_propertiesDockWidget->setVisible(true);
    m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, m_visual->m_keyFramePropertiesDockWidget);
    m_visual->m_keyFramePropertiesDockWidget->setVisible(true);
    m_mainWindow->addDockWidget(Qt::BottomDockWidgetArea, m_visual->m_timelineDockWidget);
    m_visual->m_timelineDockWidget->setVisible(true);
}

void AnimationListTabbedEditor::deactivate()
{
    m_mainWindow->removeDockWidget(m_visual->m_animationListDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_propertiesDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_keyFramePropertiesDockWidget);
    m_mainWindow->removeDockWidget(m_visual->m_timelineDockWidget);

    super::deactivate();
}

bool AnimationListTabbedEditor::saveAs(const QString &targetPath, bool updateCurrentPath)
{
    bool codeMode = currentWidget() == m_code;

    // if user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)

    if (codeMode)
        m_code->propagateToVisual();

    m_nativeData = m_visual->generateNativeData();

    return super::saveAs(targetPath, updateCurrentPath);
}

} // namespace animation_list
} // namespace editors
} // namespace CEED

