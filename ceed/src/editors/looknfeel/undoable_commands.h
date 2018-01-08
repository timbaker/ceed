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

#ifndef CEED_editors_looknfeel_undoable_commands_
#define CEED_editors_looknfeel_undoable_commands_

#include "CEEDBase.h"

#include "cegui/ceguitypes.h"
#include "editors/looknfeel/falagard_element_interface.h"
#include "editors/looknfeel/tabbed_editor.h"

#include "commands.h"
#include "propertytree/propertytree_properties.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace undoable_commands {

const int idbase = 1300;


/*!
\brief TargetWidgetChangeCommand

This command changes the Look n' Feel widget targeted for editing to another one

*/
class TargetWidgetChangeCommand : public commands::UndoCommand
{
public:
    visual::LookNFeelVisualEditing* m_visual;
    tabbed_editor::LookNFeelTabbedEditor *m_tabbedEditor;
    QString m_oldTargetWidgetLook;
    QString m_newTargetWidgetLook;

    /**
    :param visual: LookNFeelVisualEditing
    :param tabbedEditor: LookNFeelTabbedEditor
    :param newTargetWidgetLook: string
    :return:
    */
    TargetWidgetChangeCommand(visual::LookNFeelVisualEditing* visual_, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor, const QString &newTargetWidgetLook);

    void refreshText();

    int id() const override
    {
        return idbase + 1;
    }

    bool mergeWith(QUndoCommand* cmd_)
    {
        auto cmd = static_cast<TargetWidgetChangeCommand*>(cmd_);
        if (m_newTargetWidgetLook == cmd->m_newTargetWidgetLook) {
            return true;
        }

        return false;
    }

    void undo();

    void redo();
};


/*!
\brief FalagardElementAttributeEdit

This command resizes given widgets from old positions and old sizes to new

*/
class FalagardElementAttributeEdit : public commands::UndoCommand
{
public:
    visual::LookNFeelVisualEditing* m_visual;
    FalagardElement m_falagardElement;
    QString m_attributeName;
    propertytree::properties::Property* m_falagardProperty;
    CEED::Variant m_oldValue;
    QString m_oldValueAsString;
    QString m_newValueAsString;
    CEED::Variant m_newValue;
    QString m_falagardElementName;
    bool m_ignoreNextCall;

    FalagardElementAttributeEdit(propertytree::properties::Property* falagardProperty, visual::LookNFeelVisualEditing* visual_,
                                 FalagardElement falagardElement, const QString& attributeName, const CEED::Variant& newValue,
                                 bool ignoreNextCallback=false);

    void refreshText()
    {
        setText(QString("Changing '%1' in '%2' to value '%3'").arg(m_attributeName, m_falagardElementName, m_newValueAsString));
    }

    int id() const override
    {
        return idbase + 2;
    }

    bool mergeWith(const QUndoCommand *other) override
    {
        auto cmd = static_cast<const FalagardElementAttributeEdit*>(other);

        if (m_falagardElement == cmd->m_falagardElement && m_attributeName == cmd->m_attributeName) {
            m_newValue = cmd->m_newValue;
            m_newValueAsString = cmd->m_newValueAsString;

            refreshText();

            return true;
        }

        return false;
    }

    void undo();

    void redo();
};

} // namespace undoable_commands
} // namespace looknfeel
} // namespace editors
} // namespace CEED

  #endif
