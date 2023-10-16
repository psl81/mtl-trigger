#include <QMouseEvent>
#include <QListView>
#include <QJsonArray>
#include "task_property.h"

////////////////////////////////////////////////////////////////////////////////
// BoolProperty class

BoolProperty::BoolProperty(std::list<TaskProperty*> & list, const QString& name, bool value)
    : name(name), value(value)
{
    list.push_back(this);
}

QWidget* BoolProperty::createControl(const QString & control_text)
{
    control = new QCheckBox();
    control->setText(control_text);
    control->setChecked(value);
    connect(control, &QCheckBox::toggled, this, &BoolProperty::setValue);
    connect(this, &BoolProperty::valueChanged, control, &QCheckBox::setChecked);
    return control;
}

void BoolProperty::toJson(QJsonObject& json) const
{
    if (!name.isEmpty())
        json[name] = QJsonValue(value);
}

void BoolProperty::fromJson(const QJsonObject& json)
{
    if (!name.isEmpty())
        value = json[name].toBool(value);
}

void BoolProperty::setValue(bool value)
{
    if (this->value != value)
    {
        this->value = value;
        emit valueChanged(value);
        emit changed();
    }
}

////////////////////////////////////////////////////////////////////////////////
// ListItemDelegate class

ListItemDelegate::ListItemDelegate(QObject* parent) :
    QItemDelegate(parent),
    remove_icon(":/ts/remove_32px.png")
{
}

void ListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QItemDelegate::paint(painter, option, index);
    remove_icon.paint(painter, option.rect, Qt::AlignRight);
}

bool ListItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::MouseButtonPress &&
        mouseCursorIsOnRemoveIcon(dynamic_cast<QMouseEvent*> (event), model, option, index))
        model->removeRow(index.row());
    return QItemDelegate::editorEvent(event, model, option, index);
}

bool ListItemDelegate::mouseCursorIsOnRemoveIcon(
    QMouseEvent* event,
    QAbstractItemModel* model,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    if (event == nullptr) return false;
    QPoint point = event->pos();
    QRect item_rect = option.rect;
    item_rect.setX(item_rect.width() - option.rect.height());
    return item_rect.contains(point);
}

////////////////////////////////////////////////////////////////////////////////
// ListModel class

bool ListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    double d = QVariant(value.toString().replace(",", ".")).toDouble();
    return QStringListModel::setData(index, QVariant(QLocale().toString(d)), role);
}

////////////////////////////////////////////////////////////////////////////////
// StringList property class

StringListProperty::StringListProperty(
    std::list<TaskProperty*>& properties_list,
    const QString & name,
    QStringList list) :
    name(name)
{
    properties_list.push_back(this);
    list_model.setStringList(list);
}

QWidget* StringListProperty::createControl()
{
    control = new QListView();
    control->setModel(&list_model);
    control->setDragEnabled(true);
    control->setAcceptDrops(true);
    control->setDropIndicatorShown(true);
    control->setDefaultDropAction(Qt::MoveAction);
    control->setItemDelegateForColumn(0, &item_delegate);
    return control;
}

void StringListProperty::toJson(QJsonObject& json) const
{
    if (!name.isEmpty())
        json[name] = QJsonArray::fromStringList(list_model.stringList());
}

void StringListProperty::fromJson(const QJsonObject& json)
{
    QStringList values;
    if (!name.isEmpty())
    {
        QJsonArray json_array = json[name].toArray();
        for (auto it = json_array.begin(); it != json_array.end(); ++it)
            values.append(it->toString());
    }
    list_model.setStringList(values);
}

void StringListProperty::insertValue(const QVariant& value)
{
    int row = control->currentIndex().row() + 1;
    list_model.insertRow(row);
    QModelIndex idx = list_model.index(row);
    list_model.setData(idx, value);
    control->setCurrentIndex(idx);
}