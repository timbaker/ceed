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

#include "propertysetinspector.h"

#include "editors/looknfeel/hierarchy_tree_item.h"

#include <QLabel>
#include <QVBoxLayout>

namespace CEED {
namespace propertysetinspector {

QMap<QString, CEED::VariantType> CEGUIPropertyManager::_typeMap;

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

void PropertyInspectorWidget::setSource(const QList<CEGUI::PropertySet *> &source)
{
    if (source.isEmpty()) {
        m_selectionObjectPath = "";
        m_selectionObjectDescription = "Nothing is selected.";
        m_selectionLabelTooltip = "";

    } else if (source.length() == 1) {
        auto p = generateLabelForSet(source[0]);
        m_selectionObjectPath = p.first;
        m_selectionObjectDescription  = p.second;
        m_selectionLabelTooltip = m_selectionObjectPath + " : " + m_selectionObjectDescription;

    } else {
        QString tooltip = "";
        for (auto ceguiPropertySet : source) {
            auto p = generateLabelForSet(ceguiPropertySet);
            QString path = p.first;
            QString typeName = p.second;
            tooltip += path + " : " + typeName + "\n";
        }

        m_selectionObjectPath = "";
        m_selectionObjectDescription = "Multiple selections...";
        m_selectionLabelTooltip = tooltip;
        m_selectionLabelTooltip.chop(1); // remove trailing newline
    }

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

/////

CEGUIPropertyManager::CEGUIPropertyManager(propertymapping::PropertyMap *propertyMap)
{
    // TODO: Font*, Image*, UBox?
    _typeMap = {
        { "int", CEED::VariantType::Int },
        { "uint", CEED::VariantType::UInt },
        { "float", CEED::VariantType::Float },
        { "bool", CEED::VariantType::Bool },
        { "String", CEED::VariantType::QString },
        { "USize", CEED::VariantType::CEED_USize },
        { "UVector2", CEED::VariantType::CEED_UVector2 },
        { "URect", CEED::VariantType::CEED_URect },
        { "UBox", CEED::VariantType::CEED_UBox },
        { "AspectMode", CEED::VariantType::CEED_AspectMode },
        { "HorizontalAlignment", CEED::VariantType::CEED_HorizontalAlignment },
        { "VerticalAlignment", CEED::VariantType::CEED_VerticalAlignment },
        { "WindowUpdateMode", CEED::VariantType::CEED_WindowUpdateMode },
        { "Quaternion", CEED::VariantType::CEED_Quaternion },
        { "HorizontalFormatting", CEED::VariantType::CEED_HorizontalFormatting },
        { "VerticalFormatting", CEED::VariantType::CEED_VerticalFormatting },
        { "HorizontalTextFormatting", CEED::VariantType::CEED_HorizontalTextFormatting },
        { "VerticalTextFormatting", CEED::VariantType::CEED_VerticalTextFormatting },
        { "SortMode", CEED::VariantType::CEED_SortMode },
        { "Colour", CEED::VariantType::CEED_Colour },
        { "ColourRect", CEED::VariantType::CEED_ColourRect },
        { "Font", CEED::VariantType::CEED_FontRef },
        { "Image", CEED::VariantType::CEED_ImageRef }
    };

    m_propertyMap = propertyMap;
}

VariantType CEGUIPropertyManager::getPythonTypeFromStringifiedCeguiType(const QString &ceguiStrType)
{
    //if not ceguiStrType in CEGUIPropertyManager._typeMap:
    //    print("TODO: " + ceguiStrType)
    return CEGUIPropertyManager::_typeMap.value(ceguiStrType, CEED::VariantType::QString);
}

OrderedMap<QString, propertytree::properties::BasePropertyCategory*> CEGUIPropertyManager::buildCategories(const QList<CEGUI::PropertySet *> &ceguiPropertySets)
{
    auto propertyList = buildProperties(ceguiPropertySets);
    QMap<QString, propertytree::properties::PropertyCategory*> categories = propertytree::properties::PropertyCategory::categorisePropertyList(propertyList);

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

QList<propertytree::properties::Property *> CEGUIPropertyManager::buildProperties(const QList<CEGUI::PropertySet *> &ceguiPropertySets)
{
    // short name
    auto cgSets = ceguiPropertySets;

    m_propertyManagerCallbacks.clear();

    if (cgSets.isEmpty())
        return QList<propertytree::properties::Property *>();

    // * A CEGUI property does not have a value, it's similar to a definition
    //   and we need an object that has that property to be able to get a value.
    // * Each CEGUI PropertySet (widget, font, others) has it's own list of properties.
    // * Some properties may be shared across PropertSets but some are not.
    //
    // It's pretty simple to map the properties 1:1 when we have only one
    // set to process. When we have more, however, we need to group the
    // properties that are shared across sets so we display them as one
    // property that affects all the sets that have it.
    // We use getCEGUIPropertyGUID() to determine if two CEGUI properties
    // are the same.

    QMap<QString, QPair<CEGUI::Property*, QList<CEGUI::PropertySet*>>> cgProps;

    for (auto cgSet : cgSets) {

        // add a custom attribute to the PropertySet.
        // this will be filled later on with callbacks (see
        // 'createProperty'), one for each property that
        // will be called when the properties of the set change.
        // it's OK to clear any previous value because we only
        // use this internally and we only need a max of one 'listener'
        // for each property.
        // It's not pretty but it does the job well enough.
#if 1 // TODO
        m_propertyManagerCallbacks[cgSet] = {};
#else
        setattr(cgSet, "propertyManagerCallbacks", dict());
#endif // TODO
        auto propIt = cgSet->getPropertyIterator();

        while (!propIt.isAtEnd()) {
            auto cgProp = propIt.getCurrentValue();
            QString guid = getCEGUIPropertyGUID(cgProp);

            // if we already know this property, add the current set
            // to the list.
            if (cgProps.contains(guid)) {
                cgProps[guid].second.append(cgSet);

            // if it's a new property, check if it can be added
            } else {
                // we don't support unreadable properties
                if (cgProp->isReadable()) {
                    //print("XXX: {}/{}/{}".format(cgProp.getOrigin(), cgProp.getName(), cgProp.getDataType()))
                    // check mapping and ignore hidden properties
                    auto pmEntry = m_propertyMap->getEntry(TO_QSTR(cgProp->getOrigin()), TO_QSTR(cgProp->getName()));
                    if ((pmEntry == nullptr) || (!pmEntry->m_hidden))
                        cgProps[guid] = { cgProp, { cgSet } };
                }
            }

            propIt++;
        }
    }

    // Convert the CEGUI properties with their sets to property tree properties.
    QList<propertytree::properties::Property*> ptProps;
    for (auto propAndSets : cgProps.values()) {
        ptProps += createProperty(propAndSets.first, propAndSets.second);
    }
//        ptProps = [createProperty(ceguiProperty, propertySet) for ceguiProperty, propertySet in cgProps.values()];

    return ptProps;
}

CEGUIPropertyManager::MultiPropertyWrapper *CEGUIPropertyManager::createMultiPropertyWrapper(
        CEGUIPropertyManager::Property *templateProperty,
        const QList<CEGUIPropertyManager::Property *> &innerProperties,
        bool takeOwnership)
{
    return MultiPropertyWrapper::create(templateProperty, innerProperties, takeOwnership,
                                        [](Property*p, const QList<Property*>&i, const QList<CEED::Variant>&a, const QList<CEED::Variant>&d, bool t)
    {
        return new MultiPropertyWrapper(p, i, a, d, t);
    });
}

CEGUIPropertyManager::MultiPropertyWrapper *CEGUIPropertyManager::createProperty(CEGUI::Property *ceguiProperty,
                                                                                 const QList<CEGUI::PropertySet *> &ceguiSets)
{
    // get property information
    CEGUI::String name = ceguiProperty->getName();
    CEGUI::String category = ceguiProperty->getOrigin();
    CEGUI::String helpText = ceguiProperty->getHelp();
    bool readOnly = !ceguiProperty->isWritable();

    // get the CEGUI data type of the property
    CEGUI::String propertyDataType = ceguiProperty->getDataType();

    // if the current property map specifies a different type, use that one instead
    auto pmEntry = m_propertyMap->getEntry(TO_QSTR(category), TO_QSTR(name));
    if (pmEntry && !pmEntry->m_typeName.isEmpty())
        propertyDataType = FROM_QSTR(pmEntry->m_typeName);

    // get a native data type for the CEGUI data type, falling back to string
    CEED::VariantType pythonDataType = CEGUIPropertyManager::getPythonTypeFromStringifiedCeguiType(TO_QSTR(propertyDataType));

    // get the callable that creates this data type
    // and the Property type to use.
    std::function<CEED::Variant(const QString&)> valueCreator;

#if 1
    valueCreator = [=](const QString& strValue){ return getCeedValueFromString(pythonDataType, strValue); };
    PropertyFactoryBase* propertyType = getPropertyFactory(pythonDataType);
#elif 0
    PropertyFactoryBase* propertyType = nullptr;
    static PropertyFactory<propertytree::properties::Property> defaultFactory;

    switch (pythonDataType) {
    case CEED::VariantType::Bool:
        valueCreator = [](const QString& strValue){ return propertytree::utility::boolFromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::Int:
        valueCreator = [](const QString& strValue){ return propertytree::utility::intFromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::UInt:
        valueCreator = [](const QString& strValue){ return propertytree::utility::uintFromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::Float:
        valueCreator = [](const QString& strValue){ return propertytree::utility::floatFromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::QString:
        valueCreator = [](const QString& strValue){ return strValue; };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_UDim:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::UDim::fromString(strValue); };
        propertyType = cegui::ceguitypes::UDim::getPropertyType();
        break;
    case CEED::VariantType::CEED_USize:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::USize::fromString(strValue); };
        propertyType = cegui::ceguitypes::USize::getPropertyType();
        break;
    case CEED::VariantType::CEED_UVector2:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::UVector2::fromString(strValue); };
        propertyType = cegui::ceguitypes::UVector2::getPropertyType();
        break;
    case CEED::VariantType::CEED_URect:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::URect::fromString(strValue); };
        propertyType = cegui::ceguitypes::URect::getPropertyType();
        break;
    case CEED::VariantType::CEED_AspectMode:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::AspectMode::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_HorizontalAlignment:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::HorizontalAlignment::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_VerticalAlignment:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::VerticalAlignment::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_WindowUpdateMode:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::WindowUpdateMode::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_Quaternion:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::Quaternion::fromString(strValue); };
        propertyType = cegui::ceguitypes::Quaternion::getPropertyType();
        break;
    case CEED::VariantType::CEED_HorizontalFormatting:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::HorizontalFormatting::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_VerticalFormatting:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::VerticalFormatting::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_HorizontalTextFormatting:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::HorizontalTextFormatting::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_VerticalTextFormatting:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::VerticalTextFormatting::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_SortMode:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::SortMode::fromString(strValue); };
        propertyType = &defaultFactory;
        break;
    case CEED::VariantType::CEED_Colour:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::Colour::fromString(strValue); };
        propertyType = cegui::ceguitypes::Colour::getPropertyType();
        break;
    case CEED::VariantType::CEED_ColourRect:
        valueCreator = [](const QString& strValue){ return cegui::ceguitypes::ColourRect::fromString(strValue); };
        propertyType = cegui::ceguitypes::ColourRect::getPropertyType();
        break;
    }

#else
    if (issubclass(pythonDataType, ct.Base)) {
        // if it is a subclass of our ceguitypes, do some special handling
        valueCreator = pythonDataType.fromString;
        propertyType = pythonDataType.getPropertyType();
    } else {
        if (pythonDataType == QVariant::Bool) {
            // The built-in bool parses "false" as true
            // so we replace the default value creator.
            valueCreator = ptUtility.boolFromString;
        } else
            valueCreator = pythonDataType;
        propertyType = properties::Property;
    }
#endif
    CEED::Variant value;
    CEED::Variant defaultValue;

    // create the inner properties;
    // one property for each CEGUI PropertySet
    QList<propertytree::properties::Property*> innerProperties;
    for (CEGUI::PropertySet* ceguiSet : ceguiSets) {
//        Q_ASSERT_X(ceguiSet->isPropertyPresent(name), "createProperty", (CEGUI::String("Property '") + name + CEGUI::String("' was not found in PropertySet.")).c_str());
        value = valueCreator(TO_QSTR(ceguiProperty->get(ceguiSet)));
        defaultValue = valueCreator(TO_QSTR(ceguiProperty->getDefault(ceguiSet)));

        auto innerProperty = propertyType->createInstance(/*name=*/TO_QSTR(name),
                                                          /*value=*/value,
                                                          /*defaultValue=*/defaultValue,
                                                          /*category=*/TO_QSTR(category),
                                                          /*helpText=*/TO_QSTR(helpText),
                                                          /*readOnly=*/readOnly,
                                                          propertytree::properties::EditorOptions(),
                                                          /*createComponents=*/false  // no need for components, the template will provide these
                );
        innerProperties.append(innerProperty);

        // hook the inner callback (the 'cb' function) to
        // the value changed notification of the cegui propertyset
        // so that we update our value(s) when the propertyset's
        // property changes because of another editor (i.e. visual, undo, redo)
#if 1
        m_propertyManagerCallbacks[ceguiSet][TO_QSTR(name)] = [=]() {
            innerProperty->setValue( valueCreator( TO_QSTR( ceguiProperty->get( ceguiSet ) ) ) );
        };
#else
        auto makeCallback = [=](CEGUI::PropertySet*cs, CEGUI::Property* cp, propertytree::properties::Property* ip)
        {
            auto cb = [=](CEGUI::PropertySet*cs, CEGUI::Property* cp, propertytree::properties::Property* ip)
            {
                ip->setValue(valueCreator(TO_QSTR(cp->get(cs))));
            };

            return cb;
        };

        ceguiSet->propertyManagerCallbacks[name] = makeCallback(ceguiSet, ceguiProperty, innerProperty);
#endif
    }

    // create the template property;
    // this is the property that will create the components
    // and it will be edited.
    propertytree::properties::EditorOptions editorOptions;
    if (pmEntry && !pmEntry->m_editorSettings.isEmpty())
        editorOptions = pmEntry->m_editorSettings;
    auto templateProperty = propertyType->createInstance(/*name=*/TO_QSTR(name),
                                                         /*value=*/value,
                                                         /*defaultValue=*/defaultValue,
                                                         /*category=*/TO_QSTR(category),
                                                         /*helpText=*/TO_QSTR(helpText),
                                                         /*readOnly=*/readOnly,
                                                         /*editorOptions=*/editorOptions
            );
    templateProperty->createComponents(); // moved from constructor

    // create the multi wrapper
    auto multiProperty = createMultiPropertyWrapper(templateProperty, innerProperties, true);

    return multiProperty;
}

void CEGUIPropertyManager::updateAllValues(const QList<CEGUI::PropertySet*> &ceguiPropertySets)
{
#if 0 // TODO
    for (auto ceguiPropertySet : ceguiPropertySets) {
        if (not hasattr(ceguiPropertySet, "propertyManagerCallbacks"))
            continue;

        for (_, callback in ceguiPropertySet->propertyManagerCallbacks.iteritems())
            callback();
    }
#endif
}

void CEGUIPropertyManager::triggerPropertyManagerCallback(CEGUI::PropertySet *propertySet, const QSet<QString> &propertyNames)
{
    // if the property manager has set callbacks on this widget
    if (m_propertyManagerCallbacks.contains(propertySet)) {
        for (QString propertyName : propertyNames) {
            // if there's a callback for this property
            if (m_propertyManagerCallbacks[propertySet].contains(propertyName)) {
                // call it
                m_propertyManagerCallbacks[propertySet][propertyName]();
            }
        }
    }
}

/////

PropertyFactoryBase *getPropertyFactory(CEED::VariantType type)
{
    static PropertyFactory<propertytree::properties::Property> defaultFactory;

    switch (type) {
    case CEED::VariantType::Bool:
        return &defaultFactory;
    case CEED::VariantType::Int:
        return &defaultFactory;
    case CEED::VariantType::UInt:
        return &defaultFactory;
    case CEED::VariantType::Float:
        return &defaultFactory;
    case CEED::VariantType::QString:
        return &defaultFactory;
    case CEED::VariantType::CEED_UDim:
        return cegui::ceguitypes::UDim::getPropertyType();
    case CEED::VariantType::CEED_USize:
        return cegui::ceguitypes::USize::getPropertyType();
    case CEED::VariantType::CEED_UVector2:
        return cegui::ceguitypes::UVector2::getPropertyType();
    case CEED::VariantType::CEED_URect:
        return cegui::ceguitypes::URect::getPropertyType();
    case CEED::VariantType::CEED_UBox:
        return cegui::ceguitypes::UBox::getPropertyType();
    case CEED::VariantType::CEED_AspectMode:
        return &defaultFactory;
    case CEED::VariantType::CEED_HorizontalAlignment:
        return &defaultFactory;
    case CEED::VariantType::CEED_VerticalAlignment:
        return &defaultFactory;
    case CEED::VariantType::CEED_WindowUpdateMode:
        return &defaultFactory;
    case CEED::VariantType::CEED_Quaternion:
        return cegui::ceguitypes::Quaternion::getPropertyType();
    case CEED::VariantType::CEED_HorizontalFormatting:
        return &defaultFactory;
    case CEED::VariantType::CEED_VerticalFormatting:
        return &defaultFactory;
    case CEED::VariantType::CEED_HorizontalTextFormatting:
        return &defaultFactory;
    case CEED::VariantType::CEED_VerticalTextFormatting:
        return &defaultFactory;
    case CEED::VariantType::CEED_SortMode:
        return &defaultFactory;
    case CEED::VariantType::CEED_Colour:
        return cegui::ceguitypes::Colour::getPropertyType();
    case CEED::VariantType::CEED_ColourRect:
        return cegui::ceguitypes::ColourRect::getPropertyType();
    case CEED::VariantType::CEED_FontRef:
        return cegui::ceguitypes::FontRef::getPropertyType();
    case CEED::VariantType::CEED_ImageRef:
        return cegui::ceguitypes::ImageRef::getPropertyType();
    default:
        Q_ASSERT(false);
        return nullptr;
    }

}

CEED::Variant getCeedValueFromString(CEED::VariantType type, const QString& strValue)
{
    switch (type) {
    case CEED::VariantType::Bool:
        return propertytree::utility::boolFromString(strValue);
    case CEED::VariantType::Int:
        return propertytree::utility::intFromString(strValue);
    case CEED::VariantType::UInt:
        return propertytree::utility::uintFromString(strValue);
    case CEED::VariantType::Float:
        return propertytree::utility::floatFromString(strValue);
    case CEED::VariantType::QString:
        return strValue;
    case CEED::VariantType::CEED_UDim:
        return cegui::ceguitypes::UDim::fromString(strValue);
    case CEED::VariantType::CEED_USize:
        return cegui::ceguitypes::USize::fromString(strValue);
    case CEED::VariantType::CEED_UVector2:
        return cegui::ceguitypes::UVector2::fromString(strValue);
    case CEED::VariantType::CEED_URect:
        return cegui::ceguitypes::URect::fromString(strValue);
    case CEED::VariantType::CEED_UBox:
        return cegui::ceguitypes::UBox::fromString(strValue);
    case CEED::VariantType::CEED_AspectMode:
        return cegui::ceguitypes::AspectMode::fromString(strValue);
    case CEED::VariantType::CEED_HorizontalAlignment:
        return cegui::ceguitypes::HorizontalAlignment::fromString(strValue);
    case CEED::VariantType::CEED_VerticalAlignment:
        return cegui::ceguitypes::VerticalAlignment::fromString(strValue);
    case CEED::VariantType::CEED_WindowUpdateMode:
        return cegui::ceguitypes::WindowUpdateMode::fromString(strValue);
    case CEED::VariantType::CEED_Quaternion:
        return cegui::ceguitypes::Quaternion::fromString(strValue);
    case CEED::VariantType::CEED_HorizontalFormatting:
        return cegui::ceguitypes::HorizontalFormatting::fromString(strValue);
    case CEED::VariantType::CEED_VerticalFormatting:
        return cegui::ceguitypes::VerticalFormatting::fromString(strValue);
    case CEED::VariantType::CEED_HorizontalTextFormatting:
        return cegui::ceguitypes::HorizontalTextFormatting::fromString(strValue);
    case CEED::VariantType::CEED_VerticalTextFormatting:
        return cegui::ceguitypes::VerticalTextFormatting::fromString(strValue);
    case CEED::VariantType::CEED_SortMode:
        return cegui::ceguitypes::SortMode::fromString(strValue);
    case CEED::VariantType::CEED_Colour:
        return cegui::ceguitypes::Colour::fromString(strValue);
    case CEED::VariantType::CEED_ColourRect:
        return cegui::ceguitypes::ColourRect::fromString(strValue);
    case CEED::VariantType::CEED_FontRef:
        return cegui::ceguitypes::FontRef::fromString(strValue);
    case CEED::VariantType::CEED_ImageRef:
        return cegui::ceguitypes::ImageRef::fromString(strValue);
    default:
        Q_ASSERT(false);
        return CEED::Variant();
    }
}

CEED::Variant getCeguiValueFromString(CEED::VariantType type, const QString& strValue)
{
    switch (type) {
    case CEED::VariantType::Bool:
        return propertytree::utility::boolFromString(strValue);
    case CEED::VariantType::Int:
        return propertytree::utility::intFromString(strValue);
    case CEED::VariantType::UInt:
        return propertytree::utility::uintFromString(strValue);
    case CEED::VariantType::Float:
        return propertytree::utility::floatFromString(strValue);
    case CEED::VariantType::QString:
        return strValue;
    case CEED::VariantType::CEED_UDim:
        return cegui::ceguitypes::UDim::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_USize:
        return cegui::ceguitypes::USize::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_UVector2:
        return cegui::ceguitypes::UVector2::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_URect:
        return cegui::ceguitypes::URect::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_UBox:
        return cegui::ceguitypes::UBox::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_AspectMode:
        return cegui::ceguitypes::AspectMode::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_HorizontalAlignment:
        return cegui::ceguitypes::HorizontalAlignment::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_VerticalAlignment:
        return cegui::ceguitypes::VerticalAlignment::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_WindowUpdateMode:
        return cegui::ceguitypes::WindowUpdateMode::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_Quaternion:
        return cegui::ceguitypes::Quaternion::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_HorizontalFormatting:
        return cegui::ceguitypes::HorizontalFormatting::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_VerticalFormatting:
        return cegui::ceguitypes::VerticalFormatting::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_HorizontalTextFormatting:
        return cegui::ceguitypes::HorizontalTextFormatting::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_VerticalTextFormatting:
        return cegui::ceguitypes::VerticalTextFormatting::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_SortMode:
        return cegui::ceguitypes::SortMode::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_Colour:
        return cegui::ceguitypes::Colour::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_ColourRect:
        return cegui::ceguitypes::ColourRect::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_FontRef:
        return cegui::ceguitypes::FontRef::tryToCeguiType(FROM_QSTR(strValue));
    case CEED::VariantType::CEED_ImageRef:
        return cegui::ceguitypes::ImageRef::tryToCeguiType(FROM_QSTR(strValue));
    default:
        Q_ASSERT(false);
        return CEED::Variant();
    }
}

} // namespace propertysetinspector
} // CEED
