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

#ifndef CEED_propertysetinspector_
#define CEED_propertysetinspector_

#include "CEEDBase.h"

#include "qtwidgets.h"
#include "propertymapping.h"
#include "propertytree/propertytree_properties.h"
#include "propertytree/propertytree_ui.h"
#include "propertytree/propertytree_utility.h"

#include "cegui/ceguitypes.h"
//from ceed.cegui import ceguitypes as ct

class QLabel;

namespace CEED {
namespace propertysetinspector {

class CEGUIPropertyManager;

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
    CEGUIPropertyManager* m_propertyManager;

    PropertyInspectorWidget(QWidget* parent = nullptr);

    QSize sizeHint() const override
    {
        // we'd rather have this size
        return QSize(400, 600);
    }

private slots:
    void filterChanged(const QString& filterText);

public:
    void setPropertyManager(CEGUIPropertyManager* propertyManager)
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

    QList<CEGUI::PropertySet*> m_currentSource;

    void setSource(const QList<CEGUI::PropertySet *> &source);

    QList<CEGUI::PropertySet*> getSources()
    {
        return m_currentSource;
    }

    /**
    Shortens the window/widget path so that the whole text will fit into the label. The beginning of the, if necessary, cut-off path text will be "...".
    */
    void updateSelectionLabelElidedText();
};

class PropertyFactoryBase
{
public:
    virtual propertytree::properties::Property* createInstance(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
                                                       const QString& category = "", const QString& helpText = "", bool readOnly = false,
                                                       const propertytree::properties::EditorOptions& editorOptions = propertytree::properties::EditorOptions(),
                                                       bool createComponents = true) = 0;
};

template<typename T>
class PropertyFactory : public PropertyFactoryBase
{
public:
    propertytree::properties::Property* createInstance(const QString& name, const CEED::Variant& value = CEED::Variant(), const CEED::Variant& defaultValue = CEED::Variant(),
                                                       const QString& category = "", const QString& helpText = "", bool readOnly = false,
                                                       const propertytree::properties::EditorOptions& editorOptions = propertytree::properties::EditorOptions(),
                                                       bool createComponents = true) override
    {
        return new T(name, value, defaultValue, category, helpText, readOnly, editorOptions, createComponents);
    }
};

/*!
\brief CEGUIPropertyManager

Builds propertytree properties from CEGUI properties and PropertySets,
    using a PropertyMap.

*/
class CEGUIPropertyManager
{
public:
    // Maps CEGUI data types (in string form) to Python types

    static QMap<QString, CEED::VariantType> _typeMap;

    propertymapping::PropertyMap* m_propertyMap;
    QMap<CEGUI::PropertySet*, QMap<QString, std::function<void()>>> m_propertyManagerCallbacks;

    CEGUIPropertyManager(propertymapping::PropertyMap* propertyMap);

    static
    CEED::VariantType getPythonTypeFromStringifiedCeguiType(const QString& ceguiStrType);

    static
    QString getCEGUIPropertyGUID(CEGUI::Property* ceguiProperty)
    {
        // HACK: The GUID is used as a hash value (to be able
        // to tell if two properties are the same).
        // There's currently no way to get this information, apart
        // from examining the name, datatype, origin etc. of the property
        // and build a string/hash value out of it.
        return TO_QSTR( ceguiProperty->getOrigin() + "/" + ceguiProperty->getName() + "/" + ceguiProperty->getDataType() );
    }

    /**Create all available properties for all CEGUI PropertySets
    and categorise them.

    Return the categories, ready to be loaded into an Inspector Widget.
    */
    virtual OrderedMap<QString, propertytree::properties::BasePropertyCategory *> buildCategories(
            const QList<CEGUI::PropertySet*>& ceguiPropertySets);

    /**Create and return all available properties for the specified PropertySets.*/
    QList<propertytree::properties::Property*> buildProperties(const QList<CEGUI::PropertySet*>& ceguiPropertySets);

    using Property = propertytree::properties::Property;
    using MultiPropertyWrapper = propertytree::properties::MultiPropertyWrapper;

    using MPWCreatorFunc = std::function<MultiPropertyWrapper*(Property*, const QList<Property*>&, bool)>;

    virtual MultiPropertyWrapper* createMultiPropertyWrapper(Property* templateProperty, const QList<Property*>& innerProperties, bool takeOwnership);

    /**Create one MultiPropertyWrapper based property for the CEGUI Property
    for all of the PropertySets specified.
    */
    /*static*/ /* was static in Python, but visual::CEGUIWidgetPropertyManager overrides it */
    virtual MultiPropertyWrapper* createProperty(CEGUI::Property* ceguiProperty,
                                                 const QList<CEGUI::PropertySet*>& ceguiSets);

    /**Abuses all property manager callbacks defined for given property sets
    to update all values from them to the respective inspector widgets
    */
    static
    void updateAllValues(const QList<CEGUI::PropertySet *> &ceguiPropertySets);

    virtual void triggerPropertyManagerCallback(CEGUI::PropertySet *propertySet, const QSet<QString> &propertyNames);
};

extern PropertyFactoryBase* getPropertyFactory(CEED::VariantType type);
extern CEED::Variant getCeedValueFromString(CEED::VariantType type, const QString& strValue);
extern CEED::Variant getCeguiValueFromString(CEED::VariantType type, const QString& strValue);

} // namespace propertysetinspector
} // CEED

#endif
