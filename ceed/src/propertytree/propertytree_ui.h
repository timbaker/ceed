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

#ifndef CEED_propertytree_ui_
#define CEED_propertytree_ui_

#include "CEEDBase.h"

/**Qt property tree widget supporting classes.

PropertyTreeItem -- Base item for all items.
PropertyTreeRow -- Pair of name and value items, manages it's child rows.
PropertyCategoryRow -- Special tree items placed at the root of the tree.
PropertyRow -- Tree row bound to a Property.
PropertyTreeItemDelegate -- Facilitates editing of the rows' values.
PropertyTreeView -- QTreeView with some modifications for better results.
PropertyTreeWidget -- The property tree widget.
*/

#include "propertytree/propertytree_editors.h"
#include "propertytree/propertytree_properties.h"

#include <QStandardItem>
#include <QStyledItemDelegate>
#include <QTreeView>

namespace CEED {
namespace propertytree {

using properties::Property;
using properties::PropertyCategory;

namespace ui {

class PropertyTreeRow;
class PropertyTreeView;
class PropertyTreeWidget;

/*!
\brief PropertyTreeItem

Base item for all items.
*/
class PropertyTreeItem : public QStandardItem
{
public:
    PropertyTreeRow* m_propertyTreeRow;
    bool m_finalised;

    PropertyTreeItem(PropertyTreeRow* propertyTreeRow);

    void finalise();

    bool bold();

    void setBold(bool value);
};

/*!
\brief PropertyTreeRow

Pair of name and value items, manages it's child rows.
*/
class PropertyTreeRow
{
public:
    PropertyTreeItem* m_nameItem;
    PropertyTreeItem* m_valueItem;
    editors::PropertyEditor* m_editor;
    bool m_finalised;

    PropertyTreeRow();

    virtual void finalise();

    /**Return the name of the row (the text of the nameItem usually).*/
    virtual QString getName();

    PropertyTreeRow *getParent();

    /**Return the path to this item, using its name and the names of its parents separated by a slash.*/
    QString getNamePath();

    /**Find and return the child row with the specified name-path,
    searching in children and their children too, or None if not found.

    Return self if the path is empty.

    See getNamePath().
    */
    PropertyTreeRow* rowFromPath(const QString& path);

    /**Get the child rows; m_nameItem must exist and be valid.*/
    QList<PropertyTreeRow*> childRows();

    void appendChildRow(PropertyTreeRow* row);

    /**Create and add child rows.*/
    virtual void createChildRows()
    {
    }

    void destroyChildRows();

    /**Return the state of the row and its children as a dictionary.

    The state includes the expanded state.

    Sample return value:
        { "expanded": true, "items": { "item1": <recurse>, "item2": <recurse> } }

    Note that the "items" key/value pair may be missing if the current row
    has no children.
    */
    struct State
    {
        bool expanded = false;
        QMap<QString, State> items;

        State get(const QString& name, const State& defaultState) const
        {
            return items.contains(name) ? items[name] : defaultState;
        }
    };

    State getState(PropertyTreeView* view);

    /**Restore the state of the row and its children.

    See getState() for more information.
    */
    void setState(PropertyTreeView* view, const State& state);

    virtual bool isModified()
    {
        return false;
    }

    /**Filter children using the specified regular expression
    and return the count of children left visible.

    view -- The Tree View that manages the visibility state
    filterRegEx -- A regular expression that will be matched
                against the names of the children. Only those
                that match the regular expression will remain
                visible.
    hideUnmodified -- If true, hide all children that have
                their default values (haven't been modified).
    */
    int setFilter(PropertyTreeView* view, const QRegExp& filterRegEx, bool hideUnmodified = false);
};

/*!
\brief PropertyCategoryRow

Special tree items placed at the root of the tree.
*/
class PropertyCategoryRow : public PropertyTreeRow
{
public:
    properties::BasePropertyCategory* m_category;

    PropertyCategoryRow(properties::BasePropertyCategory* propertyCategory);

    /**
    Sets up the standard look and options for a category item
    :param categoryItem: QStandardItem
    :return:
    */
    static void setupCategoryOptions(QStandardItem *categoryItem);

    static void setCategoryItemFontBold(QStandardItem *categoryItem, bool value);

    void createChildRows() override;
};

/*!
\brief PropertyRow

Tree row bound to a Property.
*/
class PropertyRow : public PropertyTreeRow
{
public:
    Property* m_property;

    PropertyRow(Property* boundProperty);

    void createChildRows() override;

    void finalise() override;

    bool isModified() override;

    void cb_propertyComponentsUpdate(Property* senderProperty, properties::ComponentsUpdateType updateType);

    void cb_propertyValueChanged(Property* senderProperty, properties::ChangeValueReason dummyReason);

    /**Update the style of the row,
        i.e. make the name bold if the property value is not the default.
        */
    void updateStyle();
};

/*!
\brief PropertyTreeItemDelegate

Facilitates editing of the rows' values.
*/
class PropertyTreeItemDelegate : public QStyledItemDelegate
{
public:
    PropertyTreeWidget* m_tree;
    editors::PropertyEditorRegistry* m_registry;

    // Sample delegate
    // http://www.qtforum.org/post/81956/qtreeview-qstandarditem-and-singals.html#post81956

    PropertyTreeItemDelegate(PropertyTreeWidget* propertyTree, editors::PropertyEditorRegistry* editorRegistry);

    void cb_closeEditor(QWidget* editWidget, QAbstractItemDelegate::EndEditHint hint);

    PropertyRow *getPropertyRow(const QModelIndex& index) const;

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**Set the value of the editor to the property's value.*/
    void setEditorData(QWidget* dummyEditWidget, const QModelIndex& index) const override;

    /**Set the value of the property to the editor's value.*/
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

/*!
\brief PropertyTreeView

QTreeView with some modifications for better results.
*/
class PropertyTreeView : public QTreeView
{
    typedef QTreeView super;
public:
    QAbstractItemView::EditTriggers m_editTriggersForName;
    QAbstractItemView::EditTriggers m_editTriggersForValue;
    QColor m_originalBackgroundColour;

    PropertyTreeView(QWidget* parent);

    void setRequiredOptions();

    void setOptimalDefaults();

    /**Called when the current index changes.*/
    void currentChanged(const QModelIndex &currentIndex, const QModelIndex &previousIndex) override;

    /** Chooses and draws an alternating background colours for an item in a QTreeView. The colour
        is chosen depending on the numbers of top-level elements (interpreted as categories) before the current item.

        :param treeView: QTreeView
        :param originalBackgroundColour: QColor
        :param painter:
        :param option:
        :param index:
        :return:
        */
    static void paintAlternatingRowBackground(const QTreeView* treeView, const QColor& originalBackgroundColour,
                                              QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index);

    /**Draws grid lines and draws alternating background colours, depending on the category.
    */
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;

    void expand(QStandardItem* item, int startingDepth, int currentDepth);

    void expandFromDepth(int startingDepth);
};

class PropertyTreeItemModel : public QStandardItemModel
{
public:
    QModelIndex buddy(const QModelIndex& index) const override;
};

/*!
\brief PropertyTreeWidget

The property tree widget.

    Sets up any options necessary.
    Provides easy access methods.

*/
class PropertyTreeWidget : public QWidget
{
public:
    PropertyTreeItemModel* m_model;
    PropertyTreeView* m_view;
    struct {
        QString filterText;
        bool hideUnmodified;
    } m_filterSettings;
    QString m_previousPath;
    editors::PropertyEditorRegistry* m_registry;

    /**Initialise the widget instance.

        'setupRegistry()' should be called next,
        before any property editing can happen.
        */
    PropertyTreeWidget(QWidget* parent = nullptr);

    void rowsAboutToBeRemoved(const QModelIndex& parentIndex, int start, int end);

    /**Setup the registry and the item delegate.*/
    void setupRegistry(editors::PropertyEditorRegistry* registry);

    /**Clear the tree.
        Does not clear the current filter.
        */
    void clear();

    /**Return the name path of the current row, or None.*/
    QString getCurrentPath();

    /**Find and return the row with the specified name-path, or None.

        See PropertyTreeRow.getNamePath()
        */
    PropertyTreeRow* rowFromPath(const QString& path);

    /**Set the current row by a name-path and return true
        on success, false on failure.
        */
    bool setCurrentPath(const QString& path);

    /**Return the current state of the items.

    See PropertyTreeRow.getState().
    */
    PropertyTreeRow::State getRowsState();

    /**Restore the state of the items to a saved state.

    defaultCategoryExpansion -- None, to leave categories that are not in
                                the specified 'state' to their current
                                expansion state; true to expand them;
                                false to collapse them.

    Note: This does not call m_view.setUpdatesEnabled() before or
    after changing the items' state; it's left to the caller because
    this operation may be a part of another that handles updates
    already.

    See getRowsState() and PropertyTreeRow.getState().
    */
    void setRowsState(const PropertyTreeRow::State& state, bool defaultCategoryExpansion = false);

    /**Clear tree and load the specified categories into it.

    categories -- Dictionary
    resetState -- false to try to maintain as much of the previous items' state
                as possible, true to reset it.

    Note: This does not change the current filter.

    See getRowsState() and setRowsState().
    */
    void load(const OrderedMap<QString, propertytree::properties::BasePropertyCategory *> &categories, bool resetState = false);

    void appendCategory(properties::BasePropertyCategory *category);

    void setFilter(const QString& filterText_ = "", bool hideUnmodified = false);
};

} // namespace ui
} // namespace propertytree
} // namespace CEED

#endif
