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

#include "falagard_element_inspector.h"

#include "editors/looknfeel/hierarchy_tree_item.h"
#include "editors/looknfeel/editor_looknfeel_visual.h"

#include "propertymapping.h"
#include "propertysetinspector.h"

#include <QLabel>
#include <QVBoxLayout>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace falagard_element_inspector {

QMap<CEED::VariantType, CEED::VariantType> FalagardElementAttributesManager::_typeMap = {
        { CEED::VariantType::Int, CEED::VariantType::Int },
        { CEED::VariantType::Float, CEED::VariantType::Float },
        { CEED::VariantType::Bool, CEED::VariantType::Bool },
        { CEED::VariantType::CEGUI_String, CEED::VariantType::QString },
        { CEED::VariantType::CEGUI_USize,  CEED::VariantType::CEED_USize },
        { CEED::VariantType::CEGUI_UVector2, CEED::VariantType::CEED_UVector2 },
        { CEED::VariantType::CEGUI_URect, CEED::VariantType::CEED_URect },
        { CEED::VariantType::CEGUI_AspectMode, CEED::VariantType::CEED_AspectMode },
        { CEED::VariantType::CEGUI_HorizontalAlignment, CEED::VariantType::CEED_HorizontalAlignment },
        { CEED::VariantType::CEGUI_VerticalAlignment, CEED::VariantType::CEED_VerticalAlignment },
        { CEED::VariantType::CEGUI_WindowUpdateMode, CEED::VariantType::CEED_WindowUpdateMode },
        { CEED::VariantType::CEGUI_Quaternion, CEED::VariantType::CEED_Quaternion },
        { CEED::VariantType::CEGUI_HorizontalFormatting, CEED::VariantType::CEED_HorizontalFormatting },
        { CEED::VariantType::CEGUI_VerticalFormatting, CEED::VariantType::CEED_VerticalFormatting },
        { CEED::VariantType::CEGUI_HorizontalTextFormatting, CEED::VariantType::CEED_HorizontalTextFormatting },
        { CEED::VariantType::CEGUI_VerticalTextFormatting, CEED::VariantType::CEED_VerticalTextFormatting },
        { CEED::VariantType::CEGUI_SortMode, CEED::VariantType::CEED_SortMode },
        { CEED::VariantType::CEGUI_Colour, CEED::VariantType::CEED_Colour },
        { CEED::VariantType::CEGUI_ColourRect, CEED::VariantType::CEED_ColourRect },
        { CEED::VariantType::CEGUI_Font, CEED::VariantType::CEED_FontRef },
        { CEED::VariantType::CEGUI_Image, CEED::VariantType::CEED_ImageRef },
        { CEED::VariantType::CEGUI_BasicImage, CEED::VariantType::CEED_ImageRef }
    };

CEED::VariantType FalagardElementAttributesManager::getPythonTypeFromCeguiType(CEED::VariantType ceguiType)
{
    return FalagardElementAttributesManager::_typeMap.value(ceguiType, CEED::VariantType::QString);
}

CEED::VariantType FalagardElementAttributesManager::getCeguiTypeTypeFromPythonType(CEED::VariantType pythonType)
{
    for (auto it = _typeMap.begin(); it != _typeMap.end(); it++) {
        if (it.value() == pythonType)
            return it.key();
    }

    throw NotImplementedError("No conversion for this python type to a CEGUI type is known");
}

OrderedMap<QString, propertytree::properties::BasePropertyCategory *> FalagardElementAttributesManager::buildCategories(
        FalagardElement falagardElement)
{
    auto settingsList = createSettingsForFalagardElement(falagardElement);

    auto categories = FalagardElementSettingCategory::categorisePropertyList(settingsList);

    // sort properties in categories
    for (auto cat : categories.values())
        cat->sortProperties();

    // sort categories by name
    QList<QString> sorted = categories.keys();
    std::sort(sorted.begin(), sorted.end());
    OrderedMap<QString, propertytree::properties::BasePropertyCategory*> ret;
    for (QString catName : sorted) {
        ret[catName] = categories[catName];
    }

    return ret;
}

QPair<CEED::VariantType, propertytree::properties::EditorOptions> FalagardElementAttributesManager::getPythonCeguiTypeAndEditorOptions(
        propertymapping::PropertyMap *propertyMap, const QString &category, const QString &propertyName, CEED::VariantType dataType)
{
    propertytree::properties::EditorOptions editorOptions;
#if 0 // TODO
    QString dataType;

    // if the current property map specifies a different type, use that one instead
    auto pmEntry = propertyMap->getEntry(category, propertyName);
    if (pmEntry && !pmEntry->m_typeName.isEmpty())
        dataType = pmEntry->m_typeName;

    // Retrieve the EditorSettings if available
    if (pmEntry && !pmEntry->m_editorSettings.isEmpty())
        editorOptions = pmEntry->m_editorSettings;
#endif // TODO
    // get a native data type for the CEGUI data type, falling back to string
    CEED::VariantType pythonDataType = FalagardElementAttributesManager::getPythonTypeFromCeguiType(dataType);

    return { pythonDataType, editorOptions };
}

QPair<CEED::Variant, propertysetinspector::PropertyFactoryBase *> FalagardElementAttributesManager::getEditorPropertyTypeAndValue(
        CEED::VariantType pythonDataType, const CEED::Variant &currentValue)
{
    CEED::Variant value;
    propertysetinspector::PropertyFactoryBase* propertyType;

#if 1
    value = propertysetinspector::getCeedValueFromString(pythonDataType, currentValue.toString());
    propertyType = propertysetinspector::getPropertyFactory(pythonDataType);
#elif 0
    static propertysetinspector::PropertyFactory<propertytree::properties::Property> defaultFactory;
    QString strValue = currentValue.toString();

    switch (pythonDataType) {
    case CEED::VariantType::NONE:
        value = CEED::Variant();
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::Bool:
        value = propertytree::utility::boolFromString(strValue);
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::Int:
        value = propertytree::utility::intFromString(strValue);
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::UInt:
        value = propertytree::utility::uintFromString(strValue);
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::Float:
        value = propertytree::utility::floatFromString(strValue);
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::QString:
        value = strValue;
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_USize:
        value = cegui::ceguitypes::USize::fromString(strValue);
        propertyType = cegui::ceguitypes::USize::getPropertyType();
        break;
    case CEED::VariantType::CEED_UVector2:
        value = cegui::ceguitypes::UVector2::fromString(strValue);
        propertyType = cegui::ceguitypes::UVector2::getPropertyType();
        break;
    case CEED::VariantType::CEED_URect:
        value = cegui::ceguitypes::URect::fromString(strValue);
        propertyType = cegui::ceguitypes::URect::getPropertyType();
        break;
    case CEED::VariantType::CEED_ColourRect:
        value = cegui::ceguitypes::ColourRect::fromString(strValue);
        propertyType = cegui::ceguitypes::ColourRect::getPropertyType();
        break;
    case CEED::VariantType::CEED_FontRef:
        value = cegui::ceguitypes::FontRef::fromString(strValue);
        propertyType = cegui::ceguitypes::FontRef::getPropertyType();
        break;
    case CEED::VariantType::CEED_ImageRef:
        value = cegui::ceguitypes::ImageRef::fromString(strValue);
        propertyType = cegui::ceguitypes::ImageRef::getPropertyType();
        break;
    default:
        Q_ASSERT(false);
        break;
    }
#else
    // get the callable that creates this data type
    // and the Property type to use.
    if (issubclass(pythonDataType, ct.Base)) {
        // if it is a subclass of our ceguitypes, do some special handling
        value = pythonDataType.fromString(pythonDataType.toString(currentValue));
        propertyType = pythonDataType.getPropertyType();
    } else {
        if (!currentValue.isValid())
            value = QVariant();
        else if (pythonDataType == QVariant::Bool)
            // The built-in bool parses "false" as true
            // so we replace the default value creator.
            value = propertytree::utility::boolFromString(currentValue);
        else
            value = pythonDataType(currentValue);

        propertyType = properties.Property;
    }
#endif

    return { value, propertyType };
}

/////

QList<falagard_element_editor::FalagardElementEditorProperty *> FalagardElementAttributesManager::createSettingsForFalagardElement(FalagardElement falagardElement)
{
    QList<falagard_element_editor::FalagardElementEditorProperty*> settings;

    if (falagardElement.isType(FalagardElement::Type::NONE))
        return settings;

    //        from falagard_element_interface import FalagardElementInterface

    QStringList attributeList = FalagardElementInterface::getListOfAttributes(falagardElement);

    for (QString attributeName : attributeList) {
        auto attributePair = FalagardElementInterface::getAttributeValue(falagardElement, attributeName, m_visual->m_tabbedEditor);
        CEED::Variant attributeValue = attributePair.first;
        CEED::VariantType attributeCeguiType = attributePair.second;
        auto newSetting = createPropertyForFalagardElement(falagardElement, attributeName, attributeValue, attributeCeguiType, "");
        settings.append(newSetting);
    }

    return settings;
}

falagard_element_editor::FalagardElementEditorProperty *FalagardElementAttributesManager::createPropertyForFalagardElement(
        FalagardElement falagardElement, const QString& attributeName, const CEED::Variant &attributeValue,
        CEED::VariantType attributeCeguiType, const QString& helpText)
{
    QString falagardElementTypeStr = FalagardElementInterface::getFalagardElementTypeAsString(falagardElement);

    // Get the python type representing the cegui type and also the editor options
    auto p1 = getPythonCeguiTypeAndEditorOptions(m_propertyMap, falagardElementTypeStr, attributeName, attributeCeguiType);
    CEED::VariantType pythonDataType = p1.first;
    propertytree::properties::EditorOptions editorOptions = p1.second;

    // Get the pythonised type of the value and also its editor-propertytype
    auto p2 = getEditorPropertyTypeAndValue(pythonDataType, attributeValue);
    CEED::Variant pythonTypeValue = p2.first;
    propertysetinspector::PropertyFactoryBase* propertyType = p2.second;

    // Unmap the reference in case we reference to the WidgetLookFeel
    if (attributeName == "look" && falagardElementTypeStr == "SectionSpecification" && pythonTypeValue.isValid()) {
        pythonTypeValue = tabbed_editor::LookNFeelTabbedEditor::unmapMappedNameIntoOriginalParts(pythonTypeValue.toString()).first;
    }

    auto typedProperty = propertyType->createInstance(/*name=*/attributeName,
                                                      /*value=*/pythonTypeValue,
                                                      /*defaultValue=*/pythonTypeValue,
                                                      /*category=*/falagardElementTypeStr,
                                                      /*helpText=*/helpText,
                                                      /*readOnly=*/false,
                                                      /*editorOptions=*/editorOptions,
                                                      /*createComponents=*/true
                                                      );
    typedProperty->createComponents(); // Moved from constructor in C++ version

    // create and return the multi wrapper
    return new falagard_element_editor::FalagardElementEditorProperty(typedProperty, falagardElement, attributeName, m_visual);
}

/////

OrderedMap<QString, FalagardElementSettingCategory *> FalagardElementSettingCategory::categorisePropertyList(
        const QList<falagard_element_editor::FalagardElementEditorProperty*> &propertyList,
        const QString &unknownCategoryName)
{
    OrderedMap<QString, FalagardElementSettingCategory*> categories;
    for (auto prop : propertyList) {
        QString catName = prop->m_category.isEmpty() ? unknownCategoryName : prop->m_category;
        if (!categories.contains(catName))
            categories[catName] = new FalagardElementSettingCategory(catName);
        auto category = categories[catName];
        category->m_properties[prop->m_name] = prop;
    }

    return categories;
}

/////

PropertyInspectorWidget::PropertyInspectorWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // if the filter starts with this, we only show modified properties
    m_modifiedFilterPrefix = "*";
    m_filterBox = new qtwidgets::LineEditWithClearButton();
    m_filterBox->setPlaceholderText(QString("Filter (prefix with '%1' to show modified)").arg(m_modifiedFilterPrefix));
    connect(m_filterBox, &qtwidgets::LineEditWithClearButton::textChanged, this, &PropertyInspectorWidget::filterChanged);

    m_selectionLabel = new QLabel();
    m_selectionLabel->setAlignment(Qt::AlignCenter);
    m_selectionLabel->setFrameStyle(QFrame::StyledPanel);
    m_selectionLabel->setFrameShadow(QFrame::Sunken);

    m_selectionObjectPath = "";
    m_selectionObjectDescription = "Nothing is selected.";
    m_selectionLabelTooltip = "";

    m_ptree = new propertytree::ui::PropertyTreeWidget();
    //        """ :type : PropertyTreeWidget"""

    layout->addWidget(m_selectionLabel);
    layout->addWidget(m_filterBox);
    layout->addWidget(m_ptree);

    // set the minimum size to a reasonable value for this widget
    setMinimumSize(200, 200);

    m_propertyManager = nullptr;

//    m_currentSource = [];
}

void PropertyInspectorWidget::filterChanged(const QString &filterText)
{
    if (!filterText.isEmpty() && filterText.startsWith(m_modifiedFilterPrefix)) {
        m_ptree->setFilter(filterText.mid(m_modifiedFilterPrefix.length()), true);
    } else {
        m_ptree->setFilter(filterText);
    }
}

QPair<QString, QString> PropertyInspectorWidget::generateLabelForSet(CEGUI::PropertySet *ceguiPropertySet)
{
    // We do not know what the property set is but we can take a few informed
    // guesses. Most likely it will be a CEGUI::Window.
    if (auto window = dynamic_cast<CEGUI::Window*>(ceguiPropertySet)) {
        return { TO_QSTR(window->getType()), TO_QSTR(window->getNamePath()) };
    }

    return { "", "Unknown PropertySet" };
}

QPair<QString, QString> PropertyInspectorWidget::generateLabelForSet(void *ceguiPropertySet)
{
    return { "", "Unknown PropertySet" };
}

void PropertyInspectorWidget::setSource(FalagardElement source)
{
    auto p = hierarchy_tree_item::LookNFeelHierarchyItem::getNameAndToolTip(source, "");
    QString falagardEleName = p.first;
    QString falagardEleTooltip = p.second;

    m_selectionObjectPath = "";
    m_selectionObjectDescription = falagardEleName;
    m_selectionLabelTooltip = falagardEleTooltip;

    updateSelectionLabelElidedText();

    auto categories = m_propertyManager->buildCategories(source);

    // load them into the tree
    m_ptree->load(categories);

    m_currentSource = source;
}

void PropertyInspectorWidget::updateSelectionLabelElidedText()
{
    QString adjustedSelectionObjectPath = "";
    if (!m_selectionObjectPath.isEmpty())
        adjustedSelectionObjectPath = m_selectionObjectPath + " : ";

    QFontMetrics fontMetrics = m_selectionLabel->fontMetrics();
    int labelWidth = m_selectionLabel->size().width();
    int objectDescriptionWidth = fontMetrics.width(m_selectionObjectDescription);
    int objectPathWidth = fontMetrics.width(adjustedSelectionObjectPath);
    int margin = 6;
    int minWidthTakenByPath = 20;

    QString finalText;
    if (labelWidth > objectDescriptionWidth + objectPathWidth) {
        finalText = adjustedSelectionObjectPath + m_selectionObjectDescription;
    } else if (labelWidth < minWidthTakenByPath + objectDescriptionWidth)
        finalText = fontMetrics.elidedText(m_selectionObjectDescription, Qt::ElideRight, labelWidth - margin);
    else {
        QString alteredPathText = fontMetrics.elidedText(adjustedSelectionObjectPath, Qt::ElideLeft, labelWidth - margin - objectDescriptionWidth);
        finalText = alteredPathText + m_selectionObjectDescription;
    }

    m_selectionLabel->setText(finalText);
    m_selectionLabel->setToolTip(m_selectionLabelTooltip);
}

} // namespace falagard_element_inspector
} // namespace looknfeel
} // namespace editors
} // namespace CEED
