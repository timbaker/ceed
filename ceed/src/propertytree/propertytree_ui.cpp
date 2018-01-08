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

#include "propertytree_ui.h"

#include <QApplication>
#include <QHeaderView>
#include <QPainter>
#include <QPalette>
#include <QVBoxLayout>

namespace CEED {
namespace propertytree {
namespace ui {

PropertyTreeItem::PropertyTreeItem(PropertyTreeRow *propertyTreeRow)
    : QStandardItem()
{
    setSizeHint(QSize(-1, 24));
    m_propertyTreeRow = propertyTreeRow;

    m_finalised = false;
}

void PropertyTreeItem::finalise()
{
    m_propertyTreeRow = nullptr;
    m_finalised = true;
}

bool PropertyTreeItem::bold()
{
    return font().bold();
}

void PropertyTreeItem::setBold(bool value)
{
    QFont font = this->font();
    if (font.bold() == value)
        return;

    font.setBold(value);
    setFont(font);
}

////

PropertyTreeRow::PropertyTreeRow()
{
    m_nameItem = new PropertyTreeItem(this);
    m_valueItem = new PropertyTreeItem(this);
    m_editor = nullptr;
    m_finalised = false;

#if 0 // can't call virtual methods from constructors
    createChildRows();
#endif
}

void PropertyTreeRow::finalise()
{
    //print("Finalising row with nameItem.text() = " + str(m_nameItem.text()))
    if (!m_finalised) {
        // Finalise children before clearing nameItem or we can't get them
        destroyChildRows();

        m_nameItem->finalise();
        delete m_nameItem;
        m_nameItem = nullptr;

        m_valueItem->finalise();
        delete m_valueItem;
        m_valueItem = nullptr;
        m_finalised = true;
    }
}

QString PropertyTreeRow::getName()
{
    return m_nameItem->text();
}

PropertyTreeRow *PropertyTreeRow::getParent()
{
    QStandardItem* parentItem = m_nameItem->parent();
    if (PropertyTreeItem* pti = dynamic_cast<PropertyTreeItem*>(parentItem))
        return pti->m_propertyTreeRow;
    return nullptr;
}

QString PropertyTreeRow::getNamePath()
{
    QStringList names;
    PropertyTreeRow* parentRow = this;
    while (parentRow != nullptr) {
        names.prepend(parentRow->getName());
        parentRow = parentRow->getParent();
    }
    return names.join("/");
}

PropertyTreeRow *PropertyTreeRow::rowFromPath(const QString &path)
{
    if (path.isEmpty())
        return this;

    // split path in two
    QString part0 = path.section('/', 0, 0);
    QString part1 = path.section('/', 1);
    if (part0.isEmpty())
        return this;

    for (PropertyTreeRow* row : childRows()) {
        if (row->getName() == part0) {
            return part1.isEmpty() ? row : row->rowFromPath(part1);
        }
    }

    return nullptr;
}

QList<PropertyTreeRow *> PropertyTreeRow::childRows()
{
    QList<PropertyTreeRow*> ret;
    for (int i = 0; i < m_nameItem->rowCount(); i++) {
        ret += static_cast<PropertyTreeItem*>(m_nameItem->child(i))->m_propertyTreeRow;
    }
    return ret;
}

void PropertyTreeRow::appendChildRow(PropertyTreeRow *row)
{
    m_nameItem->appendRow( { row->m_nameItem, row->m_valueItem });
}

void PropertyTreeRow::destroyChildRows()
{
    for (PropertyTreeRow* row : childRows()) {
        row->finalise();
    }
    m_nameItem->setRowCount(0);
}

PropertyTreeRow::State PropertyTreeRow::getState(PropertyTreeView *view)
{
    State state;
    state.expanded = view->isExpanded(m_nameItem->index());
    if (m_nameItem->hasChildren()) {
        for (PropertyTreeRow* row : childRows()) {
            state.items[row->getName()] = row->getState(view);
        }
    }

    return state;
}

void PropertyTreeRow::setState(PropertyTreeView *view, const PropertyTreeRow::State &state)
{
    view->setExpanded(m_nameItem->index(), state.expanded);

    if (m_nameItem->hasChildren()) {
        for (PropertyTreeRow* row : childRows()) {
            if (state.items.contains(row->getName())) {
                const State& itemState = state.items[row->getName()];
                row->setState(view, itemState);
            }
        }
    }
}

int PropertyTreeRow::setFilter(PropertyTreeView *view, const QRegExp &filterRegEx, bool hideUnmodified)
{
    int i = 0;
    int visibleCount = 0;
    while (i < m_nameItem->rowCount()) {
        PropertyTreeItem* nameItem = static_cast<PropertyTreeItem*>(m_nameItem->child(i, 0));

        bool matched = filterRegEx.indexIn(nameItem->text()) != -1;
        if (hideUnmodified && !nameItem->m_propertyTreeRow->isModified())
            matched = false;

        view->setRowHidden(nameItem->index().row(), nameItem->index().parent(), !matched);

        i += 1;
        if (matched)
            visibleCount += 1;
    }
    return visibleCount;
}

/////

PropertyCategoryRow::PropertyCategoryRow(properties::BasePropertyCategory *propertyCategory)
    : PropertyTreeRow()
{
    // set the category before super init because
    // we need it in createChildRows() which is
    // called by super init.
    m_category = propertyCategory;

//    PropertyTreeRow();

    m_valueItem->setEditable(false);

    m_nameItem->setText(m_category->m_name);
    setupCategoryOptions(m_nameItem);
}

void PropertyCategoryRow::setupCategoryOptions(QStandardItem *categoryItem)
{
    categoryItem->setEditable(false);
    categoryItem->setSelectable(false);
    setCategoryItemFontBold(categoryItem, true);

    // Change default colours
    QPalette palette = QApplication::palette();
    categoryItem->setForeground(palette.brush(QPalette::Normal, QPalette::BrightText));
    categoryItem->setBackground(palette.brush(QPalette::Normal, QPalette::Dark));
}

void PropertyCategoryRow::setCategoryItemFontBold(QStandardItem *categoryItem, bool value)
{
    QFont font = categoryItem->font();

    if (font.bold() == value)
        return;

    font.setBold(value);
    categoryItem->setFont(font);
}

void PropertyCategoryRow::createChildRows()
{
    for (Property* prop : m_category->m_properties.values()) {
        auto* row = new PropertyRow(prop);
        row->createChildRows();
        appendChildRow(row);
    }
}

/////

PropertyRow::PropertyRow(CEED::propertytree::properties::Property *boundProperty)
    : PropertyTreeRow()
{
    // set the property before super init because
    // we need it in createChildRows() which is
    // called by super init.
    m_property = boundProperty;

//    PropertyTreeRow();

    m_nameItem->setEditable(false);
    m_nameItem->setText(m_property->m_name);
    if (!m_property->m_helpText.isEmpty())
        m_nameItem->setToolTip(m_property->m_helpText);

    m_valueItem->setEditable(!m_property->m_readOnly);
    m_valueItem->setText(m_property->valueToString());

    m_property->m_valueChanged.subscribe<PropertyRow, &PropertyRow::cb_propertyValueChanged>(this);
    m_property->m_componentsUpdate.subscribe<PropertyRow, &PropertyRow::cb_propertyComponentsUpdate>(this);

    updateStyle();
}

void PropertyRow::createChildRows()
{
    auto components = m_property->getComponents();
    if (!components.isEmpty()) {
        for (Property* component : components.values()) {
            PropertyRow* row = new PropertyRow(component);
            row->createChildRows();
            appendChildRow(row);
        }
    }
}

void PropertyRow::finalise()
{
    m_property->m_componentsUpdate.unsubscribe<PropertyRow, &PropertyRow::cb_propertyComponentsUpdate>(this);
    m_property->m_valueChanged.unsubscribe<PropertyRow, &PropertyRow::cb_propertyValueChanged>(this);

    PropertyTreeRow::finalise();
}

bool PropertyRow::isModified()
{
    return !m_property->hasDefaultValue();
}

void PropertyRow::cb_propertyComponentsUpdate(CEED::propertytree::properties::Property *senderProperty, properties::ComponentsUpdateType updateType)
{
    // destroy or recreate child rows as necessary
    if (updateType == properties::ComponentsUpdateType::BeforeDestroy)
        destroyChildRows();
    else if (updateType == properties::ComponentsUpdateType::AfterCreate)
        createChildRows();
}

void PropertyRow::cb_propertyValueChanged(CEED::propertytree::properties::Property *senderProperty, properties::ChangeValueReason dummyReason)
{
    m_valueItem->setText(m_property->valueToString());

    updateStyle();
}

void PropertyRow::updateStyle()
{
    m_nameItem->setBold(!m_property->hasDefaultValue());
}

/////

PropertyTreeItemDelegate::PropertyTreeItemDelegate(PropertyTreeWidget *propertyTree, editors::PropertyEditorRegistry* editorRegistry)
    : QStyledItemDelegate(propertyTree)
{
    m_tree = propertyTree;
    m_registry = editorRegistry;

    connect(this, &PropertyTreeItemDelegate::closeEditor,
            this, &PropertyTreeItemDelegate::cb_closeEditor);
}

void PropertyTreeItemDelegate::cb_closeEditor(QWidget *editWidget, QAbstractItemDelegate::EndEditHint hint)
{
    if (auto editor = editWidget->property("delegate_Editor").value<editors::PropertyEditor*>()) {
        //editWidget->delegate_Editor->finalise();
        //del editWidget->delegate_Editor;
        editor->finalise();
        editWidget->setProperty("delegate_Editor", QVariant());
    }
}

PropertyRow *PropertyTreeItemDelegate::getPropertyRow(const QModelIndex &index) const
{
    if (index.isValid()) {
        auto item = dynamic_cast<PropertyTreeItem*>(m_tree->m_model->itemFromIndex(index));
        return dynamic_cast<PropertyRow*>(item->m_propertyTreeRow);
    }
    return nullptr;
}

QWidget *PropertyTreeItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // get the PropertyRow from the index
    PropertyRow* row = getPropertyRow(index);
    if (row == nullptr)
        return nullptr;

    // try to create an editor for the property
    row->m_editor = m_registry->createEditor(row->m_property);
    if (row->m_editor == nullptr) {
        // if no suitable editor was found and the property
        // supports editing it as a string, wrap it and fire
        // up the string editor.
        if (row->m_property->isStringRepresentationEditable()) {
            // don't forget to finalise this property
            auto* wrapperProperty = new properties::StringWrapperProperty(row->m_property);
            QVariantMap v;
            v["validator"] = QVariant::fromValue<QValidator*>(new editors::StringWrapperValidator(row->m_property));
            wrapperProperty->m_editorOptions["string"] = v;
            row->m_editor = m_registry->createEditor(wrapperProperty);
            if (row->m_editor == nullptr) {
                wrapperProperty->finalise();
            } else {
                // set the ownsProperty flag so the editor
                // finalises it when it's finalised.
                row->m_editor->m_ownsProperty = true;
            }
        }
        if (row->m_editor == nullptr)
            return nullptr;
    }

    // tell the newly created editor to create its widget
    auto* editWidget = row->m_editor->createEditWidget(parent);

    // keep a reference to the editor inside the edit widget
    // so we can finalise the editor when the edit widget
    // is closed. See '__init__/cb_closeEditor'
    editWidget->setProperty("delegate_Editor", QVariant::fromValue<editors::PropertyEditor*>(row->m_editor));
//    editWidget->delegate_Editor = row->m_editor;

    return editWidget;
}

void PropertyTreeItemDelegate::setEditorData(QWidget *dummyEditWidget, const QModelIndex &index) const
{
    PropertyRow* row = getPropertyRow(index);
    if (row == nullptr)
        return;

    row->m_editor->setWidgetValueFromProperty();
}

void PropertyTreeItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    PropertyRow* row = getPropertyRow(index);
    if (row == nullptr)
        return;

    row->m_editor->setPropertyValueFromWidget();
}

/////

PropertyTreeView::PropertyTreeView(QWidget *parent)
    : QTreeView(parent)
{
    // optional, set by 'setOptimalDefaults()'
    m_editTriggersForName = 0;
    m_editTriggersForValue = 0;

    setRequiredOptions();
    setOptimalDefaults();

    m_originalBackgroundColour = QColor(213, 243, 233);
}

void PropertyTreeView::setRequiredOptions()
{
    // We work with rows, not columns
    setAllColumnsShowFocus(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    // We need our items expandable
    setItemsExpandable(true);
}

void PropertyTreeView::setOptimalDefaults()
{
    //
    // Behavior
    //

    // We don't want 'SelectedClicked' on the name item - it's really annoyng when
    // you try to end the editing by clicking outside the editor but you hit the name
    // and then it starts to edit again.
    // See the re-implemented currentChanged() too.
    m_editTriggersForName = QAbstractItemView::EditKeyPressed | QAbstractItemView::DoubleClicked;
    m_editTriggersForValue = m_editTriggersForName | QAbstractItemView::SelectedClicked;
    setEditTriggers(m_editTriggersForValue);
    // No tab key because we can already move through the rows using the up/down arrow
    // keys and since we usually show a ton of rows, getting tabs would make it practically
    // impossible for the user to move to the next control using tab.
    setTabKeyNavigation(false);

    // No branches on categories but allow expanding/collapsing
    // using double-click.
    // Note: We don't show branches on categories because:
    //    a) Not sure it's better
    //    b) I couldn't find a way to change the background color
    //    of the branch to match that of the category of the same
    //    row; changing the background color of the 'nameItem'
    //    does not change the background color of the branch.
    // Set to false in C++ version. Overriding drawBranches() fixes the color issue.
    setRootIsDecorated(true);
    setExpandsOnDoubleClick(true);

    // No sorting by default because it's ugly until:
    // TODO: Do custom sorting that only sorts the items inside the
    // categories but not the categories themselves.
    setSortingEnabled(false);

    // No point in selecting more than one row unless we implement
    // something like "Copy" that copies the names and/or values
    // of the selected rows.
    setSelectionMode(QAbstractItemView::SingleSelection);

    //
    // Visual
    //
    setAlternatingRowColors(true);
    //setIndentation(20)    // The default indentation is 20

    // Animation off because it plays when we do batch updates
    // even if setUpdatesEnabled(false) has been called.
    // (On Linux, at least)
    // See qtreeview.cpp, expandToDepth, it calls interruptDelayedItemsLayout()
    // which sounds promising.
    setAnimated(false);
    // Optimisation allowed because all of our rows have the same height
    setUniformRowHeights(true);
}

void PropertyTreeView::currentChanged(const QModelIndex &currentIndex, const QModelIndex &previousIndex)
{
    // See comments in 'setOptimalDefaults()'
    if (m_editTriggersForName != 0 && m_editTriggersForValue != 0) {
        if (currentIndex.isValid()) {
            if ((!previousIndex.isValid()) || (previousIndex.column() != currentIndex.column())) {
                setEditTriggers((currentIndex.column() == 0) ? m_editTriggersForName : m_editTriggersForValue);
            }
        }
    }

    super::currentChanged(currentIndex, previousIndex);
}

void PropertyTreeView::paintAlternatingRowBackground(const QTreeView *treeView, const QColor &originalBackgroundColour,
                                                     QPainter *painter, const QStyleOptionViewItem& option,
                                                     const QModelIndex &index)
{
    // Check if we draw and odd or even element, starting with the element after the category
    int aboveIndicesCount = 0;
    QModelIndex aboveIndex = treeView->indexAbove(index);
    while (aboveIndex.isValid() && aboveIndex.parent().isValid()) {
        aboveIndicesCount += 1;
        aboveIndex = treeView->indexAbove(aboveIndex);
    }

    // We check how many categories there are before this element
    int categoryCount = -1;
    aboveIndex = treeView->indexAbove(index);
    while (aboveIndex.isValid()) {
        if (!aboveIndex.parent().isValid()) {
            categoryCount += 1;
        }
        aboveIndex = treeView->indexAbove(aboveIndex);
    }

    // We use a background colour with a hue depending on the category of this item
    QColor backgroundColour = originalBackgroundColour.toHsv();
    int newHue = (backgroundColour.hue() + categoryCount * 45) % 360;
    backgroundColour.setHsv(newHue, backgroundColour.saturation(), backgroundColour.value());

    // if this is an odd element after the category, we choose an alternative colour
    if (aboveIndicesCount % 2 == 1) {
        backgroundColour = backgroundColour.lighter(112);
    }

    // Draw the background for the elements
    painter->fillRect(option.rect, backgroundColour);
#if 0 // option is 'const'
    option.palette.setBrush(QPalette::Base, QColor(0, 0, 0, 0));
    option.palette.setBrush(QPalette::AlternateBase, QColor(0, 0, 0, 0));
#endif
}

void PropertyTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    paintAlternatingRowBackground(this, m_originalBackgroundColour, painter, option, index);

    // Calling the regular draw function
    super::drawRow(painter, option, index);

    // Using a grey colour for the grid
    QColor gridColor(216, 216, 216, 255);
    // setup a pen
    QPen gridPen(gridColor);
    // x coordinate to draw the vertical line (between the first and the second column)
    qreal colX = columnViewportPosition(1) - 1;

    // save painter state so we can leave it as it was
    painter->save();
    painter->setPen(gridPen);
    // Check if this is a category ( spanning the whole row )
    bool isCategory = isFirstColumnSpanned(index.row(), index.parent());
    // do not draw verticals on spanned rows (i.e. categories)
    painter->drawLine(option.rect.x(), option.rect.bottom(), option.rect.right(), option.rect.bottom());
    if (!isCategory)
        painter->drawLine(colX, option.rect.y(), colX, option.rect.bottom());
    // aaaand restore
    painter->restore();
}

void PropertyTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    bool isCategory = isFirstColumnSpanned(index.row(), index.parent());
    if (isCategory) {
        QPalette palette = QApplication::palette();
        painter->fillRect(rect, palette.brush(QPalette::Normal, QPalette::Dark));
    }
    QTreeView::drawBranches(painter, rect, index);
}

void PropertyTreeView::expand(QStandardItem *item, int startingDepth, int currentDepth)
{
    if (currentDepth >= startingDepth) {
        setExpanded(item->index(), true);
    }
    if (item->hasChildren()) {
        int i = 0;
        while (i < item->rowCount()) {
            expand(item->child(i), startingDepth, currentDepth + 1);
            i += 1;
        }
    }
}

void PropertyTreeView::expandFromDepth(int startingDepth)
{
    /**Expand all items from the startingDepth and below.*/
    PropertyTreeItemModel* model = static_cast<PropertyTreeItemModel*>(this->model());

    int i = 0;
    while (i < model->rowCount()) {
        auto item = model->item(i, 0);
        expand(item, startingDepth, 0);

        i += 1;
    }
}

/////

QModelIndex PropertyTreeItemModel::buddy(const QModelIndex &index) const
{
    /**Point to the value item when the user tries to edit the name item.*/
    // if on column 0 (the name item)
    if (index.isValid() && index.column() == 0) {
        // if it has a sibling (value item), get it
        QModelIndex valueIndex = index.sibling(index.row(), 1);
        // if the value item is valid...
        if (valueIndex.isValid()) {
            Qt::ItemFlags flags = valueIndex.flags();
            // and it is editable but not disabled
            if ((flags & Qt::ItemIsEditable) && (flags & Qt::ItemIsEnabled)) {
                return valueIndex;
            }
        }
    }

    return QStandardItemModel::buddy(index);
}

/////

PropertyTreeWidget::PropertyTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    // create model
    m_model = new PropertyTreeItemModel();

    // finalise rows that are being removed
    connect(m_model, &PropertyTreeItemModel::rowsAboutToBeRemoved,
            this, &PropertyTreeWidget::rowsAboutToBeRemoved);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    m_view = new PropertyTreeView(this);
    m_view->setObjectName("view");
    m_view->setModel(m_model);

    layout->addWidget(m_view);

    m_registry = nullptr;
    m_filterSettings = { "", false };
    m_previousPath = "";

    clear();
}

void PropertyTreeWidget::rowsAboutToBeRemoved(const QModelIndex &parentIndex, int start, int end)
{
    for (int i = start; i <= end; i++) {
        QModelIndex mi = m_model->index(i, 0, parentIndex);
        PropertyTreeItem* item = static_cast<PropertyTreeItem*>(m_model->itemFromIndex(mi));
        if (!item->m_finalised) {
            auto* row = item->m_propertyTreeRow;
            row->finalise();
        }
    }
}

void PropertyTreeWidget::setupRegistry(editors::PropertyEditorRegistry *registry)
{
    m_registry = registry;
    auto* itemDelegate = new PropertyTreeItemDelegate(this, m_registry);
    m_view->setItemDelegate(itemDelegate);
}

void PropertyTreeWidget::clear()
{
    m_model->clear();
    m_model->setHorizontalHeaderLabels({ "Property", "Value" });
}

QString PropertyTreeWidget::getCurrentPath()
{
    QModelIndex index = m_view->currentIndex();
    if (index.isValid()) {
        auto* row = static_cast<PropertyTreeItem*>(m_model->itemFromIndex(index))->m_propertyTreeRow;
        return row->getNamePath();
    }
    return "";
}

PropertyTreeRow *PropertyTreeWidget::rowFromPath(const QString &path)
{
    if (path.isEmpty())
        return nullptr;

    // split path in two
    QString part0 = path.section('/', 0, 0);
    QString part1 = path.section('/', 1);
    if (part0.isEmpty())
        return nullptr;

    int i = 0;
    while (i < m_model->rowCount()) {
        PropertyTreeItem* item = static_cast<PropertyTreeItem*>(m_model->item(i, 0));
        PropertyTreeRow* categoryRow = item->m_propertyTreeRow;
        if (categoryRow->getName() == part0) {
            return part1.isEmpty() ? categoryRow : categoryRow->rowFromPath(part1);
        }

        i += 1;
    }

    return nullptr;
}

bool PropertyTreeWidget::setCurrentPath(const QString &path)
{
    PropertyTreeRow* row = rowFromPath(path);
    if (row != nullptr) {
        m_view->setCurrentIndex(row->m_nameItem->index());
        return true;
    }
    return false;
}

PropertyTreeRow::State PropertyTreeWidget::getRowsState()
{
    PropertyTreeRow::State state;

    int i = 0;
    while (i < m_model->rowCount()) {
        PropertyTreeItem* item = static_cast<PropertyTreeItem*>(m_model->item(i, 0));
        PropertyTreeRow* categoryRow = item->m_propertyTreeRow;
        state.items[categoryRow->getName()] = categoryRow->getState(m_view);

        i += 1;
    }

    return state;
}

void PropertyTreeWidget::setRowsState(const PropertyTreeRow::State &state, bool defaultCategoryExpansion)
{
    PropertyTreeRow::State defaultCategoryState;
    defaultCategoryState.expanded = defaultCategoryExpansion;

    int i = 0;
    while (i < m_model->rowCount()) {
        PropertyTreeItem* item = static_cast<PropertyTreeItem*>(m_model->item(i, 0));
        PropertyTreeRow* categoryRow = item->m_propertyTreeRow;
        PropertyTreeRow::State catState = state.get(categoryRow->getName(), defaultCategoryState);
        categoryRow->setState(m_view, catState);

        i += 1;
    }
}

void PropertyTreeWidget::load(const OrderedMap<QString, properties::BasePropertyCategory *> &categories, bool resetState)
{
    // prevent flicker
    m_view->setUpdatesEnabled(false);

    // save current state
    PropertyTreeRow::State itemsState;
    QString currentPath;
    if (!resetState && m_model->rowCount() != 0) {
        itemsState = getRowsState();
        currentPath = getCurrentPath();
        if (!currentPath.isEmpty())
            m_previousPath = currentPath;
    }

    // clear and setup
    clear();

    // add all categories
    for (auto category : categories.values()) {
        appendCategory(category);
    }

    // expand categories by default
    m_view->expandToDepth(0);
    // skip properties (depth == 1) but expand their components
    // so that when the user expands a property, its components
    // are expanded.
    m_view->expandFromDepth(2);

    // setup headers size
    m_view->header()->setSectionResizeMode(QHeaderView::Stretch);
    //m_view->resizeColumnToContents(0);

    // apply the filter
    setFilter(m_filterSettings.filterText, m_filterSettings.hideUnmodified);

    // restore state
    if (!resetState) {
        setRowsState(itemsState, true);
        setCurrentPath(m_previousPath);
    }

    // reset updates
    m_view->setUpdatesEnabled(true);
}

void PropertyTreeWidget::appendCategory(properties::BasePropertyCategory *category)
{
    auto* row = new PropertyCategoryRow(category);
    row->createChildRows();
    m_model->appendRow({ row->m_nameItem, row->m_valueItem });
    // make the category name span two columns (all)
    m_view->setFirstColumnSpanned(m_model->rowCount() - 1, QModelIndex(), true);
}

void PropertyTreeWidget::setFilter(const QString &filterText_, bool hideUnmodified)
{
    // we store the filter to be able to reapply it when our data changes
    m_filterSettings = { filterText_, hideUnmodified };

    // we append star at the beginning and at the end by default (makes property filtering much more practical)
    QString filterText = ".*" + QRegExp::escape(filterText_) + ".*";
    QRegExp regex(filterText, Qt::CaseInsensitive);

    int i = 0;
    while (i < m_model->rowCount()) {
        PropertyTreeItem* row = static_cast<PropertyTreeItem*>(m_model->item(i, 0));
        PropertyTreeRow* categoryRow = row->m_propertyTreeRow;
        int visibleItemsLeft = categoryRow->setFilter(m_view, regex, hideUnmodified);
        // if no items are visible in the category after applying the filter,
        // hide it, otherwise show it
        m_view->setRowHidden(i, QModelIndex(), visibleItemsLeft == 0);

        i += 1;
    }
}


} // namespace ui
} // namespace propertytree
} // namespace CEED
