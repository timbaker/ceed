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

#include "falagard_element_editor.h"

#include "propertysetinspector.h"
#include "propertytree/propertytree_init.h"

#include "editors/looknfeel/falagard_element_inspector.h"
#include "editors/looknfeel/undoable_commands.h"
#include "editors/looknfeel/editor_looknfeel_visual.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_editor {

LookNFeelFalagardElementEditorDockWidget::LookNFeelFalagardElementEditorDockWidget(visual::LookNFeelVisualEditing *visual_,
                                                                                   tabbed_editor::LookNFeelTabbedEditor *tabbedEditor):
    QDockWidget()
{
    setObjectName("FalagardElementEditorDockWidget");
    m_visual = visual_;
    m_tabbedEditor = tabbedEditor;

    setWindowTitle("Falagard Element Editor");
    // Make the dock take as much space as it can vertically
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    m_inspector = new falagard_element_inspector::PropertyInspectorWidget();
    m_inspector->m_ptree->setupRegistry(new propertytree::editors::PropertyEditorRegistry(true));

    setWidget(m_inspector);
}

/////

bool FalagardElementEditorProperty::tryUpdateInner(const CEED::Variant &newValue, propertytree::properties::ChangeValueReason reason)
{
    if (super::tryUpdateInner(newValue, reason)) {
        CEED::Variant ceguiValue;
        if (newValue.type() == CEED::VariantType::CEGUI_String)
            ceguiValue = newValue.toString();
        else
            ceguiValue = newValue;

        // Create the undoable command for editing the attribute,
        // but tell it not to trigger the change-callback
        // on the first run because our editor value has already changed,
        // we just want to sync the Falagard element's attribute value now.
        auto cmd = new undoable_commands::FalagardElementAttributeEdit(this, m_visual, m_falagardElement, m_attributeName, ceguiValue, true);
        m_visual->m_tabbedEditor->m_undoStack->push(cmd);

        // make sure to redraw the scene to preview the property
        m_visual->m_scene->update();

        return true;
    }

    return false;
}


} // namespace falagard_element_editor
} // namespace looknfeel
} // namespace editors
} // namespace CEED
