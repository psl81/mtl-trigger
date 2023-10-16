#ifndef KEYVALUEMODEL_H
#define KEYVALUEMODEL_H

#include <QAbstractTableModel>
#include <QJsonObject>

class KeyValueModel : public QAbstractTableModel
{
    Q_OBJECT

    QStringList names;
    QHash<QString, QString> name_value;
public:
    KeyValueModel(QObject* parent = nullptr);
    ~KeyValueModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    QStringList getVariables();
    bool getValue(const QString& variable, QString& value);

    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
};

#endif // KEYVALUEMODEL_H
