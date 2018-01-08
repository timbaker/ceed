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

#include "editor_layout_undo.h"

#include "cegui/cegui_widgethelpers.h"

#include "editors/layout/editor_layout_widgethelpers.h"
#include "editors/layout/editor_layout_visual.h"

namespace CEED {
namespace editors {
namespace layout {
namespace undo {

bool MoveCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const MoveCommand*>(cmd_);

    if (m_widgetPaths.toSet() == cmd->m_widgetPaths.toSet()) {
        // it is nearly impossible to do the delta guesswork right, the parent might get resized
        // etc, it might be possible in this exact scenario (no resizes) but in the generic
        // one it's a pain and can't be done consistently, so I don't even try and just merge if
        // the paths match
        m_newPositions = cmd->m_newPositions;

        return true;

        //for widgetPath : m_widgetPaths:
        //    delta = m_newPositions[widgetPath] - m_oldPositions[widgetPath]
    }
    return false;
}

void MoveCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setPosition(m_oldPositions[widgetPath]);
        widgetManipulator->updateFromWidget(false, true);
        // in case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        widgetManipulator->update();

        widgetManipulator->triggerPropertyManagerCallback({"Position", "Area"});
    }
}

void MoveCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setPosition(m_newPositions[widgetPath]);
        widgetManipulator->updateFromWidget(false, true);
        // in case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        widgetManipulator->update();

        widgetManipulator->triggerPropertyManagerCallback({"Position", "Area"});
    }

    commands::UndoCommand::redo();
}

/////

ResizeCommand::ResizeCommand(visual::VisualEditing *visual, const QStringList &widgetPaths, const QMap<QString, CEGUI::UVector2> &oldPositions, const QMap<QString, CEGUI::USize> &oldSizes, const QMap<QString, CEGUI::UVector2> &newPositions, const QMap<QString, CEGUI::USize> &newSizes):
    commands::UndoCommand()
{
    m_visual = visual;

    m_widgetPaths = widgetPaths;
    m_oldPositions = oldPositions;
    m_oldSizes = oldSizes;
    m_newPositions = newPositions;
    m_newSizes = newSizes;
}

void ResizeCommand::postConstruct()
{
    refreshText();
}

void ResizeCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Resize '%1'").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Resize %1 widgets").arg(m_widgetPaths.length()));
    }
}

bool ResizeCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const ResizeCommand*>(cmd_);

    if (m_widgetPaths == cmd->m_widgetPaths) {
        // it is nearly impossible to do the delta guesswork right, the parent might get resized
        // etc, so I don't even try and just merge if the paths match
        m_newPositions = cmd->m_newPositions;
        m_newSizes = cmd->m_newSizes;

        return true;
    }

    return false;
}

void ResizeCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setPosition(m_oldPositions[widgetPath]);
        widgetManipulator->m_widget->setSize(m_oldSizes[widgetPath]);
        widgetManipulator->updateFromWidget(false, true);
        // in case the pixel size didn't change but the absolute and negative sizes changed and canceled each other out
        widgetManipulator->update();

        widgetManipulator->triggerPropertyManagerCallback({"Size", "Position", "Area"});
    }
}

void ResizeCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setPosition(m_newPositions[widgetPath]);
        widgetManipulator->m_widget->setSize(m_newSizes[widgetPath]);
        widgetManipulator->updateFromWidget(false, true);
        // in case the pixel size didn't change but the absolute and negative sizes changed and canceled each other out
        widgetManipulator->update();

        widgetManipulator->triggerPropertyManagerCallback({"Size", "Position", "Area"});
    }

    commands::UndoCommand::redo();
}

/////

DeleteCommand::DeleteCommand(visual::VisualEditing *visual, const QStringList &widgetPaths):
    commands::UndoCommand()
{
    m_visual = visual;

    m_widgetPaths = widgetPaths;
    //        m_widgetData = {};

    // we have to add all the child widgets of all widgets we are deleting
    for (QString widgetPath : m_widgetPaths) {
        cegui::widgethelpers::Manipulator* manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        auto dependencies = manipulator->getAllDescendantManipulators();

        for (cegui::widgethelpers::Manipulator* dependency : dependencies) {
            QString dependencyNamePath = TO_QSTR(dependency->m_widget->getNamePath());
            if (!m_widgetPaths.contains(dependencyNamePath)) {
                m_widgetPaths.append(dependencyNamePath);
            }
        }
    }

    // now we have to sort them in a way that ensures the most depending widgets come first
    // (the most deeply nested widgets get deleted first before their ancestors get deleted)
    class ManipulatorDependencyKey
    {
    public:
        visual::VisualEditing* m_visual;
        QString m_path;
        widgethelpers::Manipulator* m_manipulator;

        ManipulatorDependencyKey(visual::VisualEditing* visual_, const QString& path)
        {
            m_visual = visual_;

            m_path = path;
            m_manipulator = m_visual->m_scene->getManipulatorByPath(path);
        }

        bool operator<(const ManipulatorDependencyKey& otherKey) const
        {
            // if this is the ancestor of other manipulator, it comes after it
            if (m_manipulator->m_widget->isAncestor(otherKey.m_manipulator->m_widget)) {
                return true;
            }
            // vice versa
            if (otherKey.m_manipulator->m_widget->isAncestor(m_manipulator->m_widget)) {
                return false;
            }
            // otherwise, we don't care but lets define a precise order
            return m_path < otherKey.m_path;
        }
    };

#if 1
    std::sort(m_widgetPaths.begin(), m_widgetPaths.end(), [=](const QString& a, const QString&b)
    {
        return ManipulatorDependencyKey(m_visual, a) < ManipulatorDependencyKey(m_visual, b);
    });
#else
    m_widgetPaths = sorted(m_widgetPaths, key = lambda path: ManipulatorDependencyKey(m_visual, path));
#endif

    // we have to store everything about these widgets before we destroy them,
    // we want to be able to restore if user decides to undo
    for (QString widgetPath : m_widgetPaths) {
        // serialiseChildren is false because we have already included all the children and they are handled separately
        m_widgetData[widgetPath] = new widgethelpers::SerialisationData(m_visual, m_visual->m_scene->getManipulatorByPath(widgetPath)->m_widget,
                                                                        false);
    }

    refreshText();
}

void DeleteCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Delete '%1'").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Delete %1 widgets").arg(m_widgetPaths.length()));
    }
}

void DeleteCommand::undo()
{
    commands::UndoCommand::undo();

    QList<widgethelpers::Manipulator*> manipulators;

    // we have to undo in reverse to ensure widgets have their (potential) dependencies in place when they
    // are constructed
    for (auto it = m_widgetPaths.rbegin(); it != m_widgetPaths.rend(); it++) {
        widgethelpers::SerialisationData* data = m_widgetData[*it];
        widgethelpers::Manipulator* result = data->reconstruct(m_visual->m_scene->m_rootManipulator);
        manipulators.append(result);
    }

    m_visual->notifyWidgetManipulatorsAdded(manipulators);
}

void DeleteCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        widgethelpers::Manipulator* manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        manipulator->detach(true, true, true);
    }

    m_visual->notifyWidgetManipulatorsRemoved(m_widgetPaths);

    commands::UndoCommand::redo();
}

/////

void CreateCommand::undo()
{
    commands::UndoCommand::undo();

    QString path = m_parentWidgetPath.isEmpty() ? m_widgetName : (m_parentWidgetPath + "/" + m_widgetName);
    widgethelpers::Manipulator* manipulator = m_visual->m_scene->getManipulatorByPath(path);
    manipulator->detach(true, true);

    m_visual->m_hierarchyDockWidget->refresh();
}

void CreateCommand::redo()
{
    auto* data = new widgethelpers::SerialisationData(m_visual);

    data->m_name = FROM_QSTR(m_widgetName);
    data->m_type = FROM_QSTR(m_widgetType);
    data->m_parentPath = FROM_QSTR(m_parentWidgetPath);

    widgethelpers::Manipulator* result = data->reconstruct(m_visual->m_scene->m_rootManipulator);
    // if the size is 0x0, the widget will be hard to deal with, lets fix that in that case
    if (result->m_widget->getSize() == CEGUI::USize(CEGUI::UDim(0, 0), CEGUI::UDim(0, 0))) {
        result->m_widget->setSize(CEGUI::USize(CEGUI::UDim(0, 50), CEGUI::UDim(0, 50)));
    }

    result->updateFromWidget(true, true);

    // ensure this isn't obscured by it's parent
    result->moveToFront();

    m_visual->m_hierarchyDockWidget->refresh();

    commands::UndoCommand::redo();
}

/////

PropertyEditCommand::PropertyEditCommand(visual::VisualEditing *visual, const QString &propertyName, const QStringList &widgetPaths,
                                         const QMap<QString, CEED::Variant> &oldValues, const CEED::Variant &newValue, bool ignoreNextPropertyManagerCallback):
    commands::UndoCommand()
{
    m_visual = visual;

    m_propertyName = propertyName;
    m_widgetPaths = widgetPaths;
    m_oldValues = oldValues;
    m_newValue = newValue;

    refreshText();

    m_ignoreNextPropertyManagerCallback = ignoreNextPropertyManagerCallback;
}

void PropertyEditCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Change '%1' in '%2'").arg(m_propertyName).arg(m_widgetPaths[0]));
    } else {
        setText(QString("Change '%1' in %2 widgets").arg(m_propertyName).arg(m_widgetPaths.length()));
    }
}

bool PropertyEditCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const PropertyEditCommand*>(cmd_);

    if ((m_widgetPaths == cmd->m_widgetPaths) && (m_propertyName == cmd->m_propertyName)) {
        m_newValue = cmd->m_newValue;
        return true;
    }

    return false;
}

void PropertyEditCommand::notifyPropertyManager(widgethelpers::Manipulator *widgetManipulator, bool ignoreTarget)
{
    if (!ignoreTarget) {
        widgetManipulator->triggerPropertyManagerCallback({m_propertyName});
    }
    // some properties are related to others so that
    // when one changes, the others change too.
    // the following ensures that notifications are sent
    // about the related properties as well.
    QSet<QString> related;
    if (m_propertyName == "Size") {
        related = { "Area" };
    } else if (m_propertyName == "Area") {
        related = {"Position", "Size"};
    } else if (m_propertyName == "Position") {
        related = { "Area" };
    }

    if (!related.isEmpty()) {
        widgetManipulator->triggerPropertyManagerCallback(related);
    }
}

void PropertyEditCommand::undo()
{
    commands::UndoCommand::undo();

    for (QString widgetPath : m_widgetPaths) {
        widgethelpers::Manipulator* widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setProperty(FROM_QSTR(m_propertyName), FROM_QSTR(m_oldValues[widgetPath].toString()));
        widgetManipulator->updateFromWidget(false, true);
#if 1
        widgetManipulator->m_widget->invalidate(); // in case Rotation changed
#endif

        notifyPropertyManager(widgetManipulator, m_ignoreNextPropertyManagerCallback);
    }
    m_ignoreNextPropertyManagerCallback = false;

    // make sure to redraw the scene so the changes are visible
    m_visual->m_scene->update();
}

void PropertyEditCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        widgethelpers::Manipulator* widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setProperty(FROM_QSTR(m_propertyName), FROM_QSTR(m_newValue.toString()));
        widgetManipulator->updateFromWidget(false, true);
#if 1
        widgetManipulator->m_widget->invalidate(); // in case Rotation changed
#endif
        notifyPropertyManager(widgetManipulator, m_ignoreNextPropertyManagerCallback);
    }

    m_ignoreNextPropertyManagerCallback = false;

    // make sure to redraw the scene so the changes are visible
    m_visual->m_scene->update();

    commands::UndoCommand::redo();
}

/////

void HorizontalAlignCommand::refreshText()
{
    QString alignStr = "";
    if (m_newAlignment == CEGUI::HorizontalAlignment::HA_LEFT) {
        alignStr = "left";
    } else if (m_newAlignment == CEGUI::HorizontalAlignment::HA_CENTRE) {
        alignStr = "centre";
    } else if (m_newAlignment == CEGUI::HorizontalAlignment::HA_RIGHT) {
        alignStr = "right";
    } else {
        throw RuntimeError("Unknown horizontal alignment");
    }

    if (m_widgetPaths.length() == 1) {
        setText(QString("Horizontally align '%1' %2").arg(m_widgetPaths[0]).arg(alignStr));
    } else {
        setText(QString("Horizontally align %1 widgets %2").arg(m_widgetPaths.length()).arg(alignStr));
    }
}

void HorizontalAlignCommand::undo()
{
    commands::UndoCommand::undo();

    for (int i = 0; i < m_widgetPaths.size(); i++) {
        auto* widgetManipulator = m_visual->m_scene->getManipulatorByPath(m_widgetPaths[i]);
        widgetManipulator->m_widget->setHorizontalAlignment(m_oldAlignments[i]);
        widgetManipulator->updateFromWidget();

        widgetManipulator->triggerPropertyManagerCallback({ "HorizontalAlignment" });
    }
}

void HorizontalAlignCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setHorizontalAlignment(m_newAlignment);
        widgetManipulator->updateFromWidget();

        widgetManipulator->triggerPropertyManagerCallback({ "HorizontalAlignment" });
    }

    commands::UndoCommand::redo();
}

/////

void VerticalAlignCommand::refreshText()
{
    QString alignStr = "";
    if (m_newAlignment == CEGUI::VerticalAlignment::VA_TOP) {
        alignStr = "top";
    } else if (m_newAlignment == CEGUI::VerticalAlignment::VA_CENTRE) {
        alignStr = "centre";
    } else if (m_newAlignment == CEGUI::VerticalAlignment::VA_BOTTOM) {
        alignStr = "bottom";
    } else {
        throw RuntimeError("Unknown vertical alignment");
    }

    if (m_widgetPaths.length() == 1) {
        setText(QString("Vertically align '%1' %2").arg(m_widgetPaths[0]).arg(alignStr));
    } else {
        setText(QString("Vertically align %1 widgets %2").arg(m_widgetPaths.length()).arg(alignStr));
    }
}

void VerticalAlignCommand::undo()
{
    commands::UndoCommand::undo();

    for (int i = 0; i < m_widgetPaths.size(); i++) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(m_widgetPaths[i]);
        widgetManipulator->m_widget->setVerticalAlignment(m_oldAlignments[i]);
        widgetManipulator->updateFromWidget();

        widgetManipulator->triggerPropertyManagerCallback({ "VerticalAlignment" });
    }
}

void VerticalAlignCommand::redo()
{
    for (QString widgetPath : m_widgetPaths) {
        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        widgetManipulator->m_widget->setVerticalAlignment(m_newAlignment);
        widgetManipulator->updateFromWidget();

        widgetManipulator->triggerPropertyManagerCallback({ "VerticalAlignment" });
    }

    commands::UndoCommand::redo();
}

/////

void ReparentCommand::refreshText()
{
    if (m_oldWidgetPaths.length() == 1) {
        setText(QString("Reparent '%1' to '%2'").arg(m_oldWidgetPaths[0]).arg(m_newWidgetPaths[0]));
    } else {
        setText(QString("Reparent %1 widgets").arg(m_oldWidgetPaths.length()));
    }
}

bool ReparentCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const ReparentCommand*>(cmd_);

    if (m_newWidgetPaths == cmd->m_oldWidgetPaths) {
        m_newWidgetPaths = cmd->m_newWidgetPaths;
        refreshText();

        return true;
    }

    return false;
}

void ReparentCommand::undo()
{
    commands::UndoCommand::undo();

    m_visual->m_scene->clearSelection();
    m_visual->m_hierarchyDockWidget->m_treeView->clearSelection();

    int i = 0;
    while (i < m_newWidgetPaths.length()) {
        QString widgetPath = m_newWidgetPaths[i];
        QString oldWidgetPath = m_oldWidgetPaths[i];
        QString newWidgetName = widgetPath.section('/', -1);
        QString oldWidgetName = oldWidgetPath.section('/', -1);

        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        QString oldParentPath = oldWidgetPath.section('/', 0, -2); // all but last
        auto oldParentManipulator = m_visual->m_scene->getManipulatorByPath(oldParentPath);

        // remove it from the current CEGUI parent widget
        CEGUI::Window* ceguiParentWidget = widgetManipulator->m_widget->getParent();
        if (ceguiParentWidget != nullptr) {
            ceguiParentWidget->removeChild(widgetManipulator->m_widget);
        }

        // rename it if necessary
        if (oldWidgetName != newWidgetName) {
            widgetManipulator->m_widget->setProperty("Name", FROM_QSTR(oldWidgetName));
        }

        // add it to the old CEGUI parent widget
        CEGUI::Window* ceguiOldParentWidget = oldParentManipulator->m_widget;
        ceguiOldParentWidget->addChild(widgetManipulator->m_widget);

        // and sort out the manipulators
        widgetManipulator->setParentItem(oldParentManipulator);

        widgetManipulator->updateFromWidget(true, true);

        i += 1;
    }

    m_visual->m_hierarchyDockWidget->refresh();
}

void ReparentCommand::redo()
{
    m_visual->m_scene->clearSelection();
    m_visual->m_hierarchyDockWidget->m_treeView->clearSelection();

    int i = 0;
    while (i < m_oldWidgetPaths.length()) {
        QString widgetPath = m_oldWidgetPaths[i];
        QString newWidgetPath = m_newWidgetPaths[i];
        QString oldWidgetName = widgetPath.section('/', -1);
        QString newWidgetName = newWidgetPath.section('/', -1);

        auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        QString newParentPath = newWidgetPath.section('/', 0, -2); // all but last
        auto newParentManipulator = m_visual->m_scene->getManipulatorByPath(newParentPath);

        // remove it from the current CEGUI parent widget
        CEGUI::Window* ceguiParentWidget = widgetManipulator->m_widget->getParent();
        if (ceguiParentWidget != nullptr) {
            ceguiParentWidget->removeChild(widgetManipulator->m_widget);
        }

        // rename it if necessary
        if (oldWidgetName != newWidgetName) {
            widgetManipulator->m_widget->setProperty("Name", FROM_QSTR(newWidgetName));
        }

        // add it to the new CEGUI parent widget
        CEGUI::Window* ceguiNewParentWidget = newParentManipulator->m_widget;
        ceguiNewParentWidget->addChild(widgetManipulator->m_widget);

        // and sort out the manipulators
        widgetManipulator->setParentItem(newParentManipulator);

        widgetManipulator->updateFromWidget(true, true);

        i += 1;
    }

    m_visual->m_hierarchyDockWidget->refresh();
    commands::UndoCommand::redo();
}

/////

void PasteCommand::refreshText()
{
    if (m_clipboardData.length() == 1) {
        setText(QString("Paste '%1' hierarchy to '%2'").arg(TO_QSTR(m_clipboardData[0]->m_name)).arg(m_targetWidgetPath));
    } else {
        setText(QString("Paste %1 hierarchies to '%2'").arg(m_clipboardData.length()).arg(m_targetWidgetPath));
    }
}

void PasteCommand::undo()
{
    commands::UndoCommand::undo();

    QStringList widgetPaths;
    for (auto serialisationData : m_clipboardData) {
        QString widgetPath = TO_QSTR(serialisationData->m_parentPath + "/" + serialisationData->m_name);
        widgetPaths.append(widgetPath);

        auto manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
        bool wasRootWidget = manipulator->m_widget->getParent()== nullptr;

        manipulator->detach(true, true);

        if (wasRootWidget) {
            // this was a root widget being deleted, handle this accordingly
            m_visual->setRootWidgetManipulator(nullptr);
        }
    }

    m_visual->notifyWidgetManipulatorsRemoved(widgetPaths);
}

void PasteCommand::redo()
{
    auto targetManipulator = m_visual->m_scene->getManipulatorByPath(m_targetWidgetPath);

    for (auto serialisationData : m_clipboardData) {
        // make sure the name is unique and we will be able to paste smoothly
        serialisationData->m_name = FROM_QSTR(targetManipulator->getUniqueChildWidgetName(TO_QSTR(serialisationData->m_name)));
        serialisationData->setParentPath(m_targetWidgetPath);

        serialisationData->reconstruct(m_visual->m_scene->m_rootManipulator);
    }

    // Update the topmost parent widget recursively to get possible resize or
    // repositions of the pasted widgets into the manipulator data.
    targetManipulator->updateFromWidget(true, true);

    m_visual->m_hierarchyDockWidget->refresh();

    commands::UndoCommand::redo();
}

/////

NormaliseSizeCommand::NormaliseSizeCommand(visual::VisualEditing *visual, const QStringList &widgetPaths,
                                           const QMap<QString, CEGUI::UVector2> &oldPositions,
                                           const QMap<QString, CEGUI::USize> &oldSizes)
    // we use oldPositions as newPositions because this command never changes positions of anything
    : ResizeCommand(visual, widgetPaths, oldPositions, oldSizes, oldPositions, QMap<QString, CEGUI::USize>())
{
}

void NormaliseSizeCommand::postConstruct()
{
    for (QString widgetPath : m_widgetPaths) {
        m_newSizes[widgetPath] = normaliseSize(widgetPath);
    }

    ResizeCommand::postConstruct();
}

/////

NormaliseSizeToRelativeCommand::NormaliseSizeToRelativeCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths,
                                                               const QMap<QString, CEGUI::UVector2> &oldPositions,
                                                               const QMap<QString, CEGUI::USize> &oldSizes)
    : NormaliseSizeCommand(visual_, widgetPaths, oldPositions, oldSizes)
{
}

CEGUI::USize NormaliseSizeToRelativeCommand::normaliseSize(const QString &widgetPath)
{
    auto manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
    CEGUI::Sizef pixelSize = manipulator->m_widget->getPixelSize();
    CEGUI::Sizef baseSize = manipulator->getBaseSize();

    return CEGUI::USize(CEGUI::UDim(pixelSize.d_width / baseSize.d_width, 0),
                        CEGUI::UDim(pixelSize.d_height / baseSize.d_height, 0));
}

void NormaliseSizeToRelativeCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Normalise size of '%1' to relative").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Normalise size of %1 widgets to relative").arg(m_widgetPaths.length()));
    }
}

/////

NormaliseSizeToAbsoluteCommand::NormaliseSizeToAbsoluteCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths,
                                                               const QMap<QString, CEGUI::UVector2> &oldPositions,
                                                               const QMap<QString, CEGUI::USize> &oldSizes)
    : NormaliseSizeCommand(visual_, widgetPaths, oldPositions, oldSizes)
{
}

CEGUI::USize NormaliseSizeToAbsoluteCommand::normaliseSize(const QString& widgetPath)
{
    auto manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
    CEGUI::Sizef pixelSize = manipulator->m_widget->getPixelSize();

    return CEGUI::USize(CEGUI::UDim(0, pixelSize.d_width),
                        CEGUI::UDim(0, pixelSize.d_height));
}

void NormaliseSizeToAbsoluteCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Normalise size of '%1' to absolute").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Normalise size of %1 widgets to absolute").arg(m_widgetPaths.length()));
    }
}

/////

NormalisePositionCommand::NormalisePositionCommand(visual::VisualEditing *visual, const QStringList &widgetPaths,
                                                   const QMap<QString, CEGUI::UVector2> &oldPositions)
    : MoveCommand(visual, widgetPaths, oldPositions, QMap<QString, CEGUI::UVector2>())
{
}

void NormalisePositionCommand::postConstruct()
{
    for (auto it = m_oldPositions.begin(); it != m_oldPositions.end(); it++) {
        QString widgetPath = it.key();
        CEGUI::UVector2 oldPosition = it.value();
        m_newPositions[widgetPath] = normalisePosition(widgetPath, oldPosition);
    }

    MoveCommand::postConstruct();
}

/////

NormalisePositionToRelativeCommand::NormalisePositionToRelativeCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths,
                                                                       const QMap<QString, CEGUI::UVector2> &oldPositions)
    : NormalisePositionCommand(visual_, widgetPaths, oldPositions)
{
}

CEGUI::UVector2 NormalisePositionToRelativeCommand::normalisePosition(const QString &widgetPath, const CEGUI::UVector2 &position)
{
    auto manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
    CEGUI::Sizef baseSize = manipulator->getBaseSize();

    return CEGUI::UVector2(CEGUI::UDim((position.d_x.d_offset + position.d_x.d_scale * baseSize.d_width) / baseSize.d_width, 0),
                           CEGUI::UDim((position.d_y.d_offset + position.d_y.d_scale * baseSize.d_height) / baseSize.d_height, 0));
}

void NormalisePositionToRelativeCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Normalise position of '%1' to relative").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Normalise position of %1 widgets to relative").arg(m_widgetPaths.length()));
    }
}

/////

NormalisePositionToAbsoluteCommand::NormalisePositionToAbsoluteCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths,
                                                                       const QMap<QString, CEGUI::UVector2> &oldPositions)
    : NormalisePositionCommand(visual_, widgetPaths, oldPositions)
{
}

CEGUI::UVector2 NormalisePositionToAbsoluteCommand::normalisePosition(const QString &widgetPath, const CEGUI::UVector2 &position)
{
    auto manipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
    CEGUI::Sizef baseSize = manipulator->getBaseSize();

    return CEGUI::UVector2(CEGUI::UDim(0, position.d_x.d_offset + position.d_x.d_scale * baseSize.d_width),
                           CEGUI::UDim(0, position.d_y.d_offset + position.d_y.d_scale * baseSize.d_height));
}

void NormalisePositionToAbsoluteCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Normalise position of '%1' to absolute").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Normalise position of %1 widgets to absolute").arg(m_widgetPaths.length()));
    }
}

/////

RenameCommand::RenameCommand(visual::VisualEditing *visual_, const QString &oldWidgetPath, const QString &newWidgetName):
    commands::UndoCommand()
{
    m_visual = visual_;

    // NOTE: rfind returns -1 when '/' can't be found, so in case the widget
    //       is the root widget, -1 + 1 = 0 and the slice just returns
    //       full name of the widget

    m_oldWidgetPath = oldWidgetPath;
    m_oldWidgetName = oldWidgetPath.section('/', -1);

    m_newWidgetPath = oldWidgetPath.section('/', 0, -2) + "/" + newWidgetName;
    m_newWidgetName = newWidgetName;

    refreshText();
}

void RenameCommand::refreshText()
{
    setText(QString("Rename '%1' to '%2'").arg(m_oldWidgetName, m_newWidgetName));
}

bool RenameCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const RenameCommand*>(cmd_);

    // don't merge if the new rename command will simply revert to previous commands old name
    if (m_newWidgetPath == cmd->m_oldWidgetPath && m_oldWidgetName != cmd->m_newWidgetName) {
        m_newWidgetName = cmd->m_newWidgetName;
        m_newWidgetPath = m_oldWidgetPath.section('/', 0, -2) + "/" + m_newWidgetName;

        refreshText();
        return true;
    }

    return false;
}

void RenameCommand::undo()
{
    commands::UndoCommand::undo();

    auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(m_newWidgetPath);
    //        Q_ASSERT(hasattr(widgetManipulator, "treeItem"));
    Q_ASSERT(widgetManipulator->m_treeItem != nullptr);

    widgetManipulator->m_widget->setName(FROM_QSTR(m_oldWidgetName));
    widgetManipulator->m_treeItem->setText(m_oldWidgetName);
    widgetManipulator->m_treeItem->refreshPathData();

    widgetManipulator->triggerPropertyManagerCallback({"Name", "NamePath"});
}

void RenameCommand::redo()
{
    auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(m_oldWidgetPath);
    //        Q_ASSERT(hasattr(widgetManipulator, "treeItem"));
    Q_ASSERT(widgetManipulator->m_treeItem != nullptr);

    widgetManipulator->m_widget->setName(FROM_QSTR(m_newWidgetName));
    widgetManipulator->m_treeItem->setText(m_newWidgetName);
    widgetManipulator->m_treeItem->refreshPathData();

    widgetManipulator->triggerPropertyManagerCallback({"Name", "NamePath"});

    commands::UndoCommand::redo();
}

/////

RoundPositionCommand::RoundPositionCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths,
                                           const QMap<QString, CEGUI::UVector2> &oldPositions)
    : MoveCommand(visual_, widgetPaths, oldPositions, QMap<QString, CEGUI::UVector2>())
{
}

CEGUI::UVector2 RoundPositionCommand::roundAbsolutePosition(const CEGUI::UVector2 &oldPosition)
{
    return CEGUI::UVector2(CEGUI::UDim(oldPosition.d_x.d_scale, CEGUI::CoordConverter::alignToPixels(oldPosition.d_x.d_offset)),
                           CEGUI::UDim(oldPosition.d_y.d_scale, CEGUI::CoordConverter::alignToPixels(oldPosition.d_y.d_offset)));
}

void RoundPositionCommand::postConstruct()
{
    // calculate the new, rounded positions for the widget(s)
    for (auto it = m_oldPositions.begin(); it != m_oldPositions.end(); it++) {
        QString widgetPath = it.key();
        CEGUI::UVector2 oldPosition = it.value();
        m_newPositions[widgetPath] = roundAbsolutePosition(oldPosition);
    }

    MoveCommand::postConstruct();
}

void RoundPositionCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Round absolute position of '%1' to nearest integer").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Round absolute positions of %1 widgets to nearest integers").arg(m_widgetPaths.length()));
    }
}

/////

RoundSizeCommand::RoundSizeCommand(visual::VisualEditing *visual, const QStringList &widgetPaths,
                                   const QMap<QString, CEGUI::UVector2> &oldPositions,
                                   QMap<QString, CEGUI::USize> &oldSizes)
    // we use oldPositions as newPositions because this command never changes positions of anything
    : ResizeCommand(visual, widgetPaths, oldPositions, oldSizes, oldPositions, QMap<QString, CEGUI::USize>())
{
}

CEGUI::USize RoundSizeCommand::roundAbsoluteSize(const CEGUI::USize &oldSize)
{
    return CEGUI::USize(CEGUI::UDim(oldSize.d_width.d_scale, CEGUI::CoordConverter::alignToPixels(oldSize.d_width.d_offset)),
                        CEGUI::UDim(oldSize.d_height.d_scale, CEGUI::CoordConverter::alignToPixels(oldSize.d_height.d_offset)));
}

void RoundSizeCommand::postConstruct()
{
    // calculate the new, rounded sizes for the widget(s)
    for (auto it = m_oldSizes.begin(); it != m_oldSizes.end(); it++) {
        QString widgetPath = it.key();
        CEGUI::USize oldSize = it.value();
        m_newSizes[widgetPath] = roundAbsoluteSize(oldSize);
    }

    ResizeCommand::postConstruct();
}

void RoundSizeCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Round absolute size of '%1' to nearest integer").arg(m_widgetPaths[0]));
    } else {
        setText(QString("Round absolute sizes of %1 widgets to nearest integers").arg(m_widgetPaths.length()));
    }
}

bool RoundSizeCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const RoundSizeCommand*>(cmd_);

    // merge if the new round size command will apply to the same widget
    if (m_widgetPaths.toSet() == cmd->m_widgetPaths.toSet()) {
        return true;
    } else {
        return false;
    }
}

/////

MoveInParentWidgetListCommand::MoveInParentWidgetListCommand(visual::VisualEditing *visual_, const QStringList &widgetPaths, int delta):
    commands::UndoCommand()
{
    m_visual = visual_;
    m_widgetPaths = widgetPaths;
    m_delta = delta;

    refreshText();
}

void MoveInParentWidgetListCommand::refreshText()
{
    if (m_widgetPaths.length() == 1) {
        setText(QString("Move '%1' by %2 in parent widget list").arg(m_widgetPaths[0]).arg(m_delta));
    } else {
        setText(QString("Move %1 widgets by %2 in parent widget list").arg(m_widgetPaths.length(), m_delta));
    }
}

bool MoveInParentWidgetListCommand::mergeWith(const QUndoCommand *cmd_)
{
    auto cmd = static_cast<const MoveInParentWidgetListCommand*>(cmd_);

    if (m_widgetPaths == cmd->m_widgetPaths) {
        m_delta += cmd->m_delta;
        refreshText();
        return true;
    }

    return false;
}

void MoveInParentWidgetListCommand::undo()
{
    commands::UndoCommand::undo();

    if (m_delta != 0) {
        for (auto it = m_widgetPaths.rbegin(); it != m_widgetPaths.rend(); it++) {
            QString widgetPath = *it;
            auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
            auto parentManipulator = dynamic_cast<widgethelpers::Manipulator*>(widgetManipulator->parentItem());
            Q_ASSERT(parentManipulator != nullptr);
            auto parentWidget = dynamic_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->m_widget);
            Q_ASSERT(parentWidget != nullptr);

            size_t oldPosition = parentWidget->getPositionOfChild(widgetManipulator->m_widget);
            size_t newPosition = oldPosition - m_delta;
            parentWidget->swapChildPositions(oldPosition, newPosition);
            Q_ASSERT(newPosition == parentWidget->getPositionOfChild(widgetManipulator->m_widget));

            parentManipulator->updateFromWidget(true, true);
            parentManipulator->m_treeItem->refreshOrderingData(true, true);
        }
    }
}

void MoveInParentWidgetListCommand::redo()
{
    if (m_delta != 0) {
        for (QString widgetPath : m_widgetPaths) {
            auto widgetManipulator = m_visual->m_scene->getManipulatorByPath(widgetPath);
            auto parentManipulator = dynamic_cast<widgethelpers::Manipulator*>(widgetManipulator->parentItem());
            Q_ASSERT(parentManipulator != nullptr);
            auto parentWidget = dynamic_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->m_widget);
            Q_ASSERT(parentWidget != nullptr);

            size_t oldPosition = parentWidget->getPositionOfChild(widgetManipulator->m_widget);
            size_t newPosition = oldPosition + m_delta;
            parentWidget->swapChildPositions(oldPosition, newPosition);
            Q_ASSERT(newPosition == parentWidget->getPositionOfChild(widgetManipulator->m_widget));

            parentManipulator->updateFromWidget(true, true);
            parentManipulator->m_treeItem->refreshOrderingData(true, true);
        }
    }

    commands::UndoCommand::redo();
}


} // namespace undo
} // namespace layout
} // namespace editors
} // namespace CEED
