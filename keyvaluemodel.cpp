#include "keyvaluemodel.h"

KeyValueModel::KeyValueModel(QObject* parent) :
    QAbstractTableModel(parent)
{
}

KeyValueModel::~KeyValueModel()
{
}

int KeyValueModel::rowCount(const QModelIndex& parent) const
{
    return names.size() + 1;
}

int KeyValueModel::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant KeyValueModel::data(const QModelIndex& index, int role) const
{
    if ((role == Qt::EditRole || role == Qt::DisplayRole) &&
        0 <= index.row() && index.row() < names.size())
    {
        if (index.column() == 0)
            return names[index.row()];
        else if (index.column() == 1)
            return name_value[names[index.row()]];
    }

    return QVariant();
}

bool KeyValueModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole && 0 <= index.row() && index.row() < names.size())
    {
        if (index.column() == 0 && !value.toString().isEmpty())
        {
            names[index.row()] = value.toString();
        }
        else if (index.column() == 0 && value.toString().isEmpty())
        {
            beginRemoveRows(index.parent(), index.row(), index.row());
            name_value.remove(names[index.row()]);
            names.removeAt(index.row());
            endRemoveRows();
        }
        else if (index.column() == 1)
        {
            name_value[names[index.row()]] = value.toString();
        }
        return true;
    }

    if (role == Qt::EditRole && index.row() == names.size() && index.column() == 0 && !value.toString().isEmpty())
        if (auto search = name_value.find(value.toString()); search == name_value.end())
        {
            beginInsertRows(index.parent(), index.row(), index.row());
            names.append(value.toString());
            name_value[value.toString()] = "0";
            endInsertRows();
            return true;
        }

    return false;
}

QVariant KeyValueModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static QString header[2] = { "Name", "Value"};
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && 0 <= section && section < 2)
        return header[section];
    else if (role == Qt::DisplayRole && orientation == Qt::Vertical && 0 <= section && section < names.size())
        return QVariant(section);
    return QVariant();
}

Qt::ItemFlags KeyValueModel::flags(const QModelIndex& index) const
{
    if (0 <= index.column() && index.column() < 2)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractTableModel::flags(index);
}

QStringList KeyValueModel::getVariables()
{
    return names;
}

bool KeyValueModel::getValue(const QString& variable, QString& value)
{
    if (auto search = name_value.find(variable); search != name_value.end())
    {
        value = search.value();
        return true;
    }
    else
    {
        return false;
    }
}

QJsonObject KeyValueModel::toJson() const
{
    QJsonObject json;
    for (const auto& n : names)
        if (auto search = name_value.find(n); search != name_value.end())
            json[n] = search.value();
    return json;
}

void KeyValueModel::fromJson(const QJsonObject& json)
{
    for (auto kv = json.constBegin(); kv != json.constEnd(); ++kv)
    {
        names.push_back(kv.key());
        name_value[kv.key()] = kv.value().toString();
    };
}
