/*
   created:    2nd July 2014
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

#ifndef CEED_editors_looknfeel_falagard_element_editor_
#define CEED_editors_looknfeel_falagard_element_editor_

#include "CEEDBase.h"

#include "propertytree/propertytree_properties.h"

#include <QDockWidget>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_editor {

/*!
\brief LookNFeelFalagardElementEditorDockWidget

This dock widget allows to add, remove or edit the Property, PropertyDefinition and PropertyLinkDefinition elements of a WidgetLook

*/
class LookNFeelFalagardElementEditorDockWidget : public QDockWidget
{
public:
    visual::LookNFeelVisualEditing* m_visual;
    tabbed_editor::LookNFeelTabbedEditor* m_tabbedEditor;
    falagard_element_inspector::PropertyInspectorWidget* m_inspector;

    /**
    :param visual: LookNFeelVisualEditing
    :param tabbedEditor: LookNFeelTabbedEditor
    :return:
    */
    LookNFeelFalagardElementEditorDockWidget(visual::LookNFeelVisualEditing* visual_, tabbed_editor::LookNFeelTabbedEditor* tabbedEditor);
};


/*!
\brief FalagardElementEditorProperty

Overrides the default Property to update the 'inner properties'
    and to create undoable commands that update the WidgetLookFeel element.

*/
class FalagardElementEditorProperty : public propertytree::properties::SinglePropertyWrapper
{
    typedef propertytree::properties::SinglePropertyWrapper super;
public:
    visual::LookNFeelVisualEditing* m_visual;
    FalagardElement m_falagardElement;
    QString m_attributeName;

    FalagardElementEditorProperty(propertytree::properties::Property* ownedProperty, FalagardElement falagardElement,
                                  const QString& attributeName, visual::LookNFeelVisualEditing* visual_):
        propertytree::properties::SinglePropertyWrapper(ownedProperty)
    {
        m_visual = visual_;
        m_falagardElement = falagardElement;
        m_attributeName = attributeName;
    }

    bool tryUpdateInner(const CEED::Variant &newValue, propertytree::properties::ChangeValueReason reason = propertytree::properties::ChangeValueReason::Unknown);
};

} // namespace falagard_element_editor
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
