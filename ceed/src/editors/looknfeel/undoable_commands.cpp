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

#include "undoable_commands.h"

#include "editors/looknfeel/editor_looknfeel_visual.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace undoable_commands {

using namespace falagard_element_interface;

TargetWidgetChangeCommand::TargetWidgetChangeCommand(visual::LookNFeelVisualEditing *visual_, tabbed_editor::LookNFeelTabbedEditor *tabbedEditor, const QString& newTargetWidgetLook):
    commands::UndoCommand()
{
    m_visual = visual_;
    m_tabbedEditor = tabbedEditor;
    m_oldTargetWidgetLook = tabbedEditor->m_targetWidgetLook;
    m_newTargetWidgetLook = newTargetWidgetLook;

    refreshText();
}

void TargetWidgetChangeCommand::refreshText()
{
    QString originalOldTargetName;
    QString originalNewTargetName;

    if (!m_oldTargetWidgetLook.isEmpty()) {
        originalOldTargetName = tabbed_editor::LookNFeelTabbedEditor::unmapMappedNameIntoOriginalParts(m_oldTargetWidgetLook).first;
        originalOldTargetName = "\"" + originalOldTargetName + "\"";
    } else {
        originalOldTargetName = "no selection";
    }

    if (!m_newTargetWidgetLook.isEmpty()) {
        originalNewTargetName = tabbed_editor::LookNFeelTabbedEditor::unmapMappedNameIntoOriginalParts(m_newTargetWidgetLook).first;
        originalNewTargetName =  "\"" + originalNewTargetName + "\"";
    } else {
        originalNewTargetName = "no selection";
    }

    setText("Changed editing target from " + originalOldTargetName + " to " + originalNewTargetName);
}

void TargetWidgetChangeCommand::undo()
{
    commands::UndoCommand::undo();

    m_tabbedEditor->m_targetWidgetLook = m_oldTargetWidgetLook;
    m_visual->updateToNewTargetWidgetLook();
}

void TargetWidgetChangeCommand::redo()
{
    m_tabbedEditor->m_targetWidgetLook = m_newTargetWidgetLook;
    m_visual->updateToNewTargetWidgetLook();

    commands::UndoCommand::redo();
}

/////

FalagardElementAttributeEdit::FalagardElementAttributeEdit(CEED::propertytree::properties::Property *falagardProperty,
                                                           visual::LookNFeelVisualEditing *visual_, FalagardElement falagardElement,
                                                           const QString &attributeName, const CEED::Variant &newValue, bool ignoreNextCallback):
    commands::UndoCommand()
{
    m_visual = visual_;
    /** :type : LookNFeelVisualEditing */

    m_falagardElement = falagardElement;
    m_attributeName = attributeName;

    m_falagardProperty = falagardProperty;
    /** :type : FalagardElementEditorProperty*/

    // We retrieve the momentary value using the getter callback and store it as old value
    m_oldValue = FalagardElementInterface::getAttributeValue(falagardElement, attributeName, m_visual->m_tabbedEditor).first;
    m_oldValueAsString = m_oldValue.toString();

#if 1
    m_newValueAsString = newValue.toString();
    m_newValue = FalagardElementInterface::getCeguiTypeValueFromString(newValue.type(), m_newValueAsString);
#elif 0
    m_newValueAsString = newValue.toString();
    CEED::VariantType newValueType = newValue.type();
    switch (newValueType) {
    case VariantType::CEED_UDim: m_newValue = cegui::ceguitypes::UDim::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;
    case VariantType::CEED_USize: m_newValue = cegui::ceguitypes::USize::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;
    case VariantType::CEED_UVector2: m_newValue = cegui::ceguitypes::UVector2::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;
    case VariantType::CEED_URect: m_newValue = cegui::ceguitypes::URect::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;
    case VariantType::CEED_Colour: m_newValue = cegui::ceguitypes::Colour::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;
    case VariantType::CEED_ColourRect: m_newValue = cegui::ceguitypes::ColourRect::tryToCeguiType(FROM_QSTR(m_newValueAsString)); break;

    case VariantType::QString: m_newValue = CEGUI::String(FROM_QSTR(newValue.toString())); break;
    default:
        Q_ASSERT(false);
        break;
    }

    CEED::Variant converted = newValue;
#else
    CEED::VariantType newValueType = newValue.type();
    // If the value is a subtype of
    if (issubclass(newValueType, ceguitypes.Base)) {
        // if it is a subclass of our ceguitypes, do some special handling
        m_newValueAsString = newValue.toString();
        m_newValue = newValueType.toCeguiType(m_newValueAsString);
    } else if (newValueType == QVariant::Bool || newValueType == QVariant::String) {
        m_newValue = newValue;
        m_newValueAsString = newValue.toString();
    } else {
        throw Exception("Unexpected type encountered");
    }
#endif

    // Get the Falagard element's type as string so we can display better info
    m_falagardElementName = FalagardElementInterface::getFalagardElementTypeAsString(falagardElement);

    refreshText();

    m_ignoreNextCall = ignoreNextCallback; // unused?
}

void FalagardElementAttributeEdit::undo()
{
    commands::UndoCommand::undo();

    // We set the value using the setter callback
    FalagardElementInterface::setAttributeValue(m_falagardElement, m_attributeName, m_oldValue);

    //TODO Ident: Refresh the property view afterwards instead
    // m_falagardProperty.setValue(m_oldValue, reason=Property.ChangeValueReason.InnerValueChanged)

    m_visual->updateWidgetLookPreview();
}

void FalagardElementAttributeEdit::redo()
{
    // We set the value using the setter callback
    FalagardElementInterface::setAttributeValue(m_falagardElement, m_attributeName, m_newValue);

    //TODO Ident: Refresh the property view afterwards instead
    //m_falagardProperty.setValue(m_newValue, reason=Property.ChangeValueReason.InnerValueChanged)

    m_visual->destroyCurrentPreviewWidget();

    m_visual->m_tabbedEditor->removeOwnedWidgetLookFalagardMappings();
    m_visual->m_tabbedEditor->addMappedWidgetLookFalagardMappings();

    m_visual->updateWidgetLookPreview();

    /**
        // We add every WidgetLookFeel name of this Look N' Feel to a StringSet
        nameSet = m_visual->tabbedEditor.getStringSetOfWidgetLookFeelNames()
        // We parse all WidgetLookFeels as XML to a string
        import PyCEGUI
        lookAndFeelString = CEGUI::WidgetLookManager::getSingleton().getWidgetLookSetAsString(nameSet)

        lookAndFeelString = m_visual->tabbedEditor.unmapWidgetLookReferences(lookAndFeelString)

        m_visual->tabbedEditor.tryUpdatingWidgetLookFeel(lookAndFeelString)
        m_visual->updateToNewTargetWidgetLook()
        */

    commands::UndoCommand::redo();
}


} // namespace undoable_commands
} // namespace looknfeel
} // namespace editors
} // namespace CEED
