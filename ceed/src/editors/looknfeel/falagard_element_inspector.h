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

#ifndef CEED_editors_looknfeel_falagard_element_inspector_
#define CEED_editors_looknfeel_falagard_element_inspector_

#include "CEEDBase.h"

#include "propertytree/propertytree_properties.h"
#include "propertytree/propertytree_utility.h"
#include "cegui/ceguitypes.h"

#include "editors/looknfeel/falagard_element_editor.h"
#include "editors/looknfeel/falagard_element_interface.h"
#include "editors/looknfeel/tabbed_editor.h"

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_inspector {

class FalagardElementSettingCategory;

/*!
\brief FalagardElementAttributesManager

Builds propertytree settings from a CEGUI Falagard element, allowing to edit its attributes in the editor.

*/
class FalagardElementAttributesManager
{
public:
    // Maps CEGUI data types (in string form) to Python types

    static QMap<CEED::VariantType, CEED::VariantType> _typeMap;

    propertymapping::PropertyMap* m_propertyMap;
    visual::LookNFeelVisualEditing* m_visual;

    FalagardElementAttributesManager(propertymapping::PropertyMap* propertyMap, visual::LookNFeelVisualEditing* visual_)
    {
        m_visual = visual_;
        m_propertyMap = propertyMap;
    }

    // Returns a corresponding Python type for a given CEGUI type
    static
    CEED::VariantType getPythonTypeFromCeguiType(CEED::VariantType ceguiType);

    // Returns a corresponding CEGUI type for a given Python type
    static
    CEED::VariantType getCeguiTypeTypeFromPythonType(CEED::VariantType pythonType);

    /**Create all available Properties, PropertyDefinitions and PropertyLinkDefinition options for this WidgetLook
    and categorise them.

    Return the categories, ready to be loaded into an Inspector Widget.
    */
    OrderedMap<QString, propertytree::properties::BasePropertyCategory *> buildCategories(FalagardElement falagardElement);

    /**
    Returns the pythonised cegui type and the editoroptions, using a propertyMap.
    :param propertyMap:
    :param category:
    :param propertyName:
    :param dataType:
    :return: value and propertyType
    */
    static
    QPair<CEED::VariantType, propertytree::properties::EditorOptions> getPythonCeguiTypeAndEditorOptions(propertymapping::PropertyMap* propertyMap, const QString& category,
            const QString& propertyName, CEED::VariantType dataType);

    /**
    Gets the editor options and the pythonised value based on the python data type and the native CEGUI type value.
    Converts the value to the right internal python class for the given cegui type.
    :param pythonDataType:
    :param currentValue:
    :return:
    */
    static
    QPair<Variant, propertysetinspector::PropertyFactoryBase*> getEditorPropertyTypeAndValue(CEED::VariantType pythonDataType, const CEED::Variant &currentValue);

    using FalagardElementInterface = falagard_element_interface::FalagardElementInterface;

    /**
    Creates a list of settings for any type of Falagard Element (except the WidgetLookFeel itself)
    :param falagardElement:
    :return:
    */
    QList<falagard_element_editor::FalagardElementEditorProperty*> createSettingsForFalagardElement(FalagardElement falagardElement);

    /**
    Create a FalagardElementEditorProperty based on a type-specific property for the FalagardElement's attribute
    */
    falagard_element_editor::FalagardElementEditorProperty* createPropertyForFalagardElement(FalagardElement falagardElement, const QString &attributeName,
          const CEED::Variant &attributeValue, CEED::VariantType attributeCeguiType, const QString &helpText);
};


/*!
\brief FalagardElementSettingCategory

 A category for Falagard element settings.
    Categories have a name and hold a list of settings.

*/
class FalagardElementSettingCategory : public propertytree::properties::BasePropertyCategory
{
public:
//    QString m_name;
//    OrderedMap<QString, falagard_element_editor::FalagardElementEditorProperty*> m_properties;

    /**Initialise the instance with the specified name.*/
    FalagardElementSettingCategory(const QString& name)
    {
        m_name = name;
//        m_properties = OrderedDict();
    }

    /**Given a list of settings, creates categories and adds the
    settings to them based on their 'category' field.

    The unknownCategoryName is used for a category that holds all
    properties that have no 'category' specified.
    */
    static
    OrderedMap<QString, FalagardElementSettingCategory*> categorisePropertyList(const QList<falagard_element_editor::FalagardElementEditorProperty*>& propertyList,
                                                                                const QString& unknownCategoryName = "Unknown");

    /** We want to maintain the order used by the FalagardElement interface
    :param reverse:
    :return:
    */
    void sortProperties(bool reverse = false)
    {
//        m_properties = OrderedDict(m_properties.items());
    }
};

/*!
\brief PropertyInspectorWidget

Full blown inspector widget for CEGUI PropertySet(s).

    Requires a call to 'setPropertyManager()' before
    it can show properties via 'setPropertySets'.

*/
class PropertyInspectorWidget : public QWidget
{
public:
    QString m_modifiedFilterPrefix;
    qtwidgets::LineEditWithClearButton* m_filterBox;
    QLabel* m_selectionLabel;
    QString m_selectionObjectPath;
    QString m_selectionObjectDescription;
    QString m_selectionLabelTooltip;
    propertytree::ui::PropertyTreeWidget* m_ptree;
    FalagardElementAttributesManager* m_propertyManager;
    FalagardElement m_currentSource;

    PropertyInspectorWidget(QWidget* parent = nullptr);

    QSize sizeHint() const override
    {
        // we'd rather have this size
        return QSize(400, 600);
    }

private slots:
    void filterChanged(const QString& filterText);

public:
    void setPropertyManager(FalagardElementAttributesManager* propertyManager)
    {
        m_propertyManager = propertyManager;
    }

    void resizeEvent(QResizeEvent *event) override
    {
        updateSelectionLabelElidedText();

        QWidget::resizeEvent(event);
    }

    static
    QPair<QString, QString> generateLabelForSet(CEGUI::PropertySet* ceguiPropertySet);

    static
    QPair<QString, QString> generateLabelForSet(void* ceguiPropertySet);

    void setSource(FalagardElement source);

    FalagardElement getSource()
    {
        return m_currentSource;
    }

    /**
    Shortens the window/widget path so that the whole text will fit into the label. The beginning of the, if necessary, cut-off path text will be "...".
    */
    void updateSelectionLabelElidedText();
};

} // namespace falagard_element_inspector
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
