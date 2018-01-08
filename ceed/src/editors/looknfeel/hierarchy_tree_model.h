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

#ifndef CEED_editors_looknfeel_hierarchy_tree_model_
#define CEED_editors_looknfeel_hierarchy_tree_model_

#include "CEEDBase.h"

#include <QStandardItemModel>

namespace CEED {
namespace editors {
namespace looknfeel {
namespace hierarchy_tree_model {

class LookNFeelHierarchyTreeModel : public QStandardItemModel
{
    typedef QStandardItemModel super;
public:
    hierarchy_dock_widget::LookNFeelHierarchyDockWidget* m_dockWidget;
    CEGUI::WidgetLookFeel* m_widgetLookObject;
    QString m_limitDisplayToStateImagery; // optional<QString> maybe

    LookNFeelHierarchyTreeModel(hierarchy_dock_widget::LookNFeelHierarchyDockWidget* dockWidget);

    /**
    Sets the WidgetLookFeel object based on which this widget should create the hierarchy.
    Rebuilds the hierarchy tree based on a WidgetFeelLook object.
    :param widgetLookObject: CEGUI::WidgetLookFeel
    :param limitDisplayTo: str
    :return:
    */
    void updateTree(CEGUI::WidgetLookFeel* widgetLookObject, const QString& limitDisplayTo);

    /**
    Iterates over all contained elements of the WidgetLookFeel. Creates the contained elements as
    new items and appends them to this tree view's root item
    :param widgetLookObject: CEGUI::WidgetLookFeel
    :return:
    */
    void appendAllWidgetLookFeelChildren(CEGUI::WidgetLookFeel* widgetLookObject);

    /**
    Returns the StateImagery, ImagerySection, WidgetComponent and NamedArea names associated with the current view
    :param widgetLookObject: CEGUI::WidgetLookFeel
    :return:
    */
    void getViewDependentElementNames(CEGUI::WidgetLookFeel* widgetLookObject,
                                     CEGUI::StringList& stateImageryNames,
                                     CEGUI::StringList& imagerySectionNames);

    /**
    Creates a category item based on the supplied name and tooltip
    :param name:
    :param toolTip:
    :return: QStandardItem
    */
    QStandardItem* createAndAddCategoryToRoot(const QString& name, const QString& toolTip);

    /**
    Creates an item based on the supplied object and appends it to the rootItem of this tree model
    :param falagardElement:
    :param parentItem: QStandardItem
    :return: QStandardItem
    */
    static
    QStandardItem* createAndAddItem(FalagardElement falagardElement, QStandardItem* parentItem);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        return super::data(index, role);
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        return super::setData(index, value, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return super::flags(index);
    }

    bool isChild(QStandardItem* parent, QStandardItem* potentialChild) const
    {
        int i = 0;
        // DFS, Qt doesn't have helper methods for this it seems to me :-/
        while (i < parent->rowCount()) {
            auto child = parent->child(i);

            if (child == potentialChild)
                return true;

            if (isChild(child, potentialChild))
                return true;

            i += 1;
        }

        return false;
    }

    QMimeData* mimeData(const QModelIndexList &indexes) const;

    QStringList mimeTypes()
    {
        return { "application/x-ceed-widget-paths", "application/x-ceed-widget-type" };
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override
    {
        return false;
    }
};

} // namespace hierarchy_tree_model
} // namespace looknfeel
} // namespace editors
} // namespace CEED

#endif
