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

#include "hierarchy_tree_model.h"

#include "editors/looknfeel/hierarchy_tree_item.h"
#include "propertytree/propertytree_ui.h"

#include <QMimeData>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_model {

LookNFeelHierarchyTreeModel::LookNFeelHierarchyTreeModel(hierarchy_dock_widget::LookNFeelHierarchyDockWidget *dockWidget):
    super()
{
    m_dockWidget = dockWidget;
    /** :type : LookNFeelHierarchyDockWidget */

    setItemPrototype(new hierarchy_tree_item::LookNFeelHierarchyItem(FalagardElement()));

    m_widgetLookObject = nullptr;

    /** A string defining a name of a StateImagery. This StateImagery, and everything referenced within it, will be the only one
        to be displayed in the hierarchy. The value None means there won't be any limitation and everything will be displayed. */
    m_limitDisplayToStateImagery = "";
}

void LookNFeelHierarchyTreeModel::updateTree(CEGUI::WidgetLookFeel *widgetLookObject, const QString &limitDisplayTo)
{
    m_limitDisplayToStateImagery = limitDisplayTo;
    m_widgetLookObject = widgetLookObject;

    // Clear any existing hierarchy
    clear();

    if (m_widgetLookObject == nullptr)
        return;

    appendAllWidgetLookFeelChildren(m_widgetLookObject);
}

void LookNFeelHierarchyTreeModel::appendAllWidgetLookFeelChildren(CEGUI::WidgetLookFeel *widgetLookObject)
{
    // Add all properties

    auto propertyDefMap = widgetLookObject->getPropertyDefinitionMap(false);
    if (!propertyDefMap.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("PropertyDefinitions", "type: PropertyDefinitionBase");
        for (auto currentPropertyDef : propertyDefMap) {
//            currentPropertyDef = CEGUI::Workarounds.PropertyDefinitionBaseMapGet(propertyDefMap, propDefMapEntry.key);
            createAndAddItem(currentPropertyDef.second, categoryItem);
        }
    }

    auto propertyLinkDefMap = widgetLookObject->getPropertyLinkDefinitionMap(false);
    if (!propertyLinkDefMap.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("PropertyLinkDefinitions", "type: PropertyDefinitionBase");
        for (auto currentPropertyLinkDef : propertyLinkDefMap) {
//            currentPropertyLinkDef = CEGUI::Workarounds.PropertyDefinitionBaseMapGet(propertyLinkDefMap, propLinkDefMapEntry.key);
            createAndAddItem(currentPropertyLinkDef.second, categoryItem);
        }
    }

    auto propertyInitialiserMap = widgetLookObject->getPropertyInitialiserMap(false);
    if (!propertyInitialiserMap.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("Properties", "type: PropertyInitialiser");
        for (auto currentPropertyInitialiser : propertyInitialiserMap) {
//            currentPropertyInitialiser = CEGUI::Workarounds.PropertyInitialiserMapGet(propertyInitialiserMap, propertyInitialiserMapEntry.key);
            createAndAddItem(currentPropertyInitialiser.second, categoryItem);
        }
    }

    // Create and add all view-dependent hierarchy items owned by the WidgetLookFeel

    auto namedAreaMap = widgetLookObject->getNamedAreaMap(false);
    if (!namedAreaMap.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("NamedAreas", "type: NamedArea");
        for (auto currentNamedArea : namedAreaMap) {
//            currentNamedArea = CEGUI::Workarounds.NamedAreaMapGet(namedAreaMap, namedAreaMapEntry.key);
            createAndAddItem(currentNamedArea.second, categoryItem);
        }
    }

    // Gather all elements associated with the currently selected view
    CEGUI::StringList stateImageryNames;
    CEGUI::StringList imagerySectionNames;
    getViewDependentElementNames(widgetLookObject, stateImageryNames, imagerySectionNames);

    auto imagerySectionMap = widgetLookObject->getImagerySectionMap(false);
    if (!imagerySectionNames.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("ImagerySections", "type: ImagerySection");
        for (auto imageSectionName : imagerySectionNames) {
//            currentImagerySection = CEGUI::Workarounds.ImagerySectionMapGet(imagerySectionMap, imagerySectionName);
            createAndAddItem(imagerySectionMap[imageSectionName], categoryItem);
        }
    }

    auto stateImageryMap = widgetLookObject->getStateImageryMap(false);
    if (!stateImageryNames.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("StateImageries", "type: StateImagery");
        for (auto stateImageryName : stateImageryNames) {
//            currentStateImagery = CEGUI::Workarounds.StateImageryMapGet(stateImageryMap, stateImageryName);
            createAndAddItem(stateImageryMap[stateImageryName], categoryItem);
        }
    }

    auto widgetComponentMap = widgetLookObject->getWidgetComponentMap(false);
    if (!widgetComponentMap.empty()) {
        QStandardItem* categoryItem = createAndAddCategoryToRoot("WidgetComponents", "type: WidgetComponent");
        for (auto widgetComponent : widgetComponentMap) {
//            widgetComponent = CEGUI::Workarounds.WidgetComponentMapGet(widgetComponentMap, widgetComponentMapEntry.key);
            createAndAddItem(widgetComponent.second, categoryItem);
        }
    }
}

void LookNFeelHierarchyTreeModel::getViewDependentElementNames(CEGUI::WidgetLookFeel *widgetLookObject, CEGUI::StringList &stateImageryNames, CEGUI::StringList &imagerySectionNames)
{
    // We get all names of all elements
    auto stateImageryNamesSet = widgetLookObject->getStateImageryNames(true);
    auto imagerySectionNamesSet = widgetLookObject->getImagerySectionNames(true);
    stateImageryNames.assign(stateImageryNamesSet.begin(), stateImageryNamesSet.end()); // set -> list
    imagerySectionNames.assign(imagerySectionNamesSet.begin(), imagerySectionNamesSet.end()); // set -> list

    // If the current mode is unlimited, return all names unaltered
    if (m_limitDisplayToStateImagery.isEmpty())
        return;

    // We retrieve the StateImagery object that we want to display exclusively
    CEGUI::WidgetLookFeel::StateImageryPointerMap stateImageryMap = widgetLookObject->getStateImageryMap(true);
    CEGUI::StateImagery* viewedStateImagery = stateImageryMap[m_limitDisplayToStateImagery.toStdString()]; //  CEGUI::Workarounds.StateImageryMapGet(stateImageryMap, self.limitDisplayToStateImagery)
    Q_ASSERT(viewedStateImagery != nullptr);

    // We iterate over all layers and all SectionSpecification in the layers, looking for all
    // ImagerySections that are referenced from there and are "owned" by this WidgetLookFeel
    // (They could also be inside another WLF definition, in which case we won't display them)
    // All such ImagerySections will be added to a list
    CEGUI::StringList referencedImagerySections;
    CEGUI::StateImagery::LayerSpecificationPointerList layerSpecList = viewedStateImagery->getLayerSpecificationPointers();
    for (CEGUI::LayerSpecification* layerSpec : layerSpecList) {
        CEGUI::LayerSpecification::SectionSpecificationPointerList sectionSpecList = layerSpec->getSectionSpecificationPointers();
        for (CEGUI::SectionSpecification* sectionSpec : sectionSpecList) {
            auto ownerWidgetFeel = sectionSpec->getOwnerWidgetLookFeel();
            if (ownerWidgetFeel == m_widgetLookObject->getName()) {
                referencedImagerySections.emplace_back(sectionSpec->getSectionName());
            }
        }
    }

    // We make each ImagerySection unique using a set
    imagerySectionNamesSet.clear();
    imagerySectionNamesSet.insert(referencedImagerySections.begin(), referencedImagerySections.end()); // list -> set
    imagerySectionNames.assign(imagerySectionNamesSet.begin(), imagerySectionNamesSet.end()); // set -> list

    stateImageryNames = { m_limitDisplayToStateImagery.toStdString() };
    return;
}

QStandardItem *LookNFeelHierarchyTreeModel::createAndAddCategoryToRoot(const QString &name, const QString &toolTip)
{
    auto rootItem = invisibleRootItem();

    auto categoryItem = new QStandardItem(name);
    categoryItem->setToolTip(toolTip);

    propertytree::ui::PropertyCategoryRow::setupCategoryOptions(categoryItem);

    rootItem->appendRow(categoryItem);

    return categoryItem;
}

QStandardItem *LookNFeelHierarchyTreeModel::createAndAddItem(FalagardElement falagardElement, QStandardItem *parentItem)
{
    if (falagardElement.isType(FalagardElement::Type::NONE))
        throw Exception("Invalid parameter supplied to LookNFeelHierarchyTreeModel.createAndAddItem");

    auto newItem = new hierarchy_tree_item::LookNFeelHierarchyItem(falagardElement);
    parentItem->appendRow(newItem);

    return newItem;
}

QMimeData *LookNFeelHierarchyTreeModel::mimeData(const QModelIndexList &indexes) const
{
    // if the selection contains children of something that is also selected, we don't include that
    // (it doesn't make sense to move it anyways, it will be moved with its parent)

    QList<QStandardItem*> topItems;

    for (auto index : indexes) {
        auto item = itemFromIndex(index);
        bool hasParent = false;

        for (auto parentIndex : indexes) {
            if (parentIndex == index)
                continue;

            auto potentialParent = itemFromIndex(parentIndex);

            if (isChild(item, potentialParent)) {
                hasParent = true;
                break;
            }
        }

        if (!hasParent)
            topItems.append(item);
    }

    QStringList paths;
    for (auto item : topItems)
        paths.append(item->data(Qt::UserRole).toString());
#if 1 // copy-paste errors again?
    // TODO
    auto ret = new QMimeData();
#else
    auto ret = new layout::visual::WidgetPathsMimeData();
    ret->paths = paths;
    ret->setData("application/x-ceed-widget-paths", QByteArray());
#endif

    return ret;
}


} // namespace hierarchy_tree_model
} // namespace looknfeel
} // namespace editors
} // namespace CEED
