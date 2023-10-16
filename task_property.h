#ifndef TASK_PROPERTY_H
#define TASK_PROPERTY_H

#include <QJsonObject>
#include <QJsonArray>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QStringListModel>
#include <QItemDelegate>
#include <QListView>
#include "linefilebrowse.h"
#include "togglebox.h"

class TaskProperty : public QObject 
{
    Q_OBJECT
public:
    virtual void toJson(QJsonObject& json) const = 0;
    virtual void fromJson(const QJsonObject& json) = 0;
Q_SIGNALS:
    void changed();
};

class ProtoNumProperty : public TaskProperty
{
    Q_OBJECT
Q_SIGNALS:
    void valueChanged(int);
    void valueChanged(double);
};

template <typename T>
class NumProperty : public ProtoNumProperty
{
public:
    using SpinBoxType = typename std::conditional<std::is_same<T, int>::value, QSpinBox, QDoubleSpinBox>::type;

    NumProperty(std::list<TaskProperty*>& list, const QString& name, const T value, const T min, const T max)
        : name(name), value(value), min_value(min), max_value(max) 
    {
        list.push_back(this);
    }

    QWidget* createControl(const QString & prefix, const QString & suffix)
    {
        control = new SpinBoxType();
        if constexpr (std::is_same<SpinBoxType, QDoubleSpinBox>::value)
            control->setDecimals(3);
        control->setRange(min_value, max_value);
        control->setValue(value);
        control->setPrefix(prefix);
        control->setSuffix(suffix);
        connect(control, &SpinBoxType::editingFinished, this, &NumProperty<T>::updateValue);
//        connect(control, QOverload<T>::of(&SpinBoxType::valueChanged), this, &NumProperty<T>::setValue);
        connect(this, QOverload<T>::of(&NumProperty<T>::valueChanged), control, &SpinBoxType::setValue);
        return control;
    }

    virtual void toJson(QJsonObject& json) const override
    {
        if (!name.isEmpty())
            json[name] = QJsonValue(value);
    }

    virtual void fromJson(const QJsonObject& json) override
    {
        if (!name.isEmpty())
        {
            if (std::is_same<T, int>::value)
                value = json[name].toInt(value);
            else
                value = json[name].toDouble(value);
        }
            
    }

    void updateValue()
    {
        this->value = control->value();
        emit valueChanged(value);
        emit changed();
    }

    void setValue(T value)
    {
        this->value = value;
        emit valueChanged(value);
        emit changed();
    }

    operator T () const
    {
        return value;
    }

    SpinBoxType * control = nullptr;
private:
    T value = 0;
    QString name;
    T min_value = 0;
    T max_value = 0;
};

class EnumProperty : public TaskProperty
{
public:
    EnumProperty(std::list<TaskProperty*>& list, const QString& name, const int value, const QStringList & enum_list)
        : name(name), value(value), enum_list(enum_list)
    {
        list.push_back(this);
    }

    QWidget* createControl()
    {
        control = new QComboBox();
        control->addItems(enum_list);
        control->setCurrentIndex(value);
        connect(control, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EnumProperty::setValue);
        return control;
    }

    virtual void toJson(QJsonObject& json) const override
    {
        if (!name.isEmpty())
            json[name] = QJsonValue(value);
    }

    virtual void fromJson(const QJsonObject& json) override
    {
        if (!name.isEmpty())
            value = json[name].toInt(value);
    }

    void setValue(int value)
    {
        this->value = value;
        emit changed();
    }

    operator int () const
    {
        return value;
    }

    QComboBox* control = nullptr;
private:
    int value = 0;
    QString name;
    QStringList enum_list;
};

////////////////////////////////////////////////////////////////////////////////
// Bool property class

class BoolProperty : public TaskProperty
{
    Q_OBJECT
public:
    BoolProperty(std::list<TaskProperty*> & list, const QString& name, bool value);
    virtual QWidget* createControl(const QString & control_text = QString());
    virtual void toJson(QJsonObject& json) const override;
    virtual void fromJson(const QJsonObject& json) override;
    void setValue(bool value);

    operator bool() const
    {
        return value;
    }

    bool value = false;
    QCheckBox * control = nullptr;

private:
    QString name;

Q_SIGNALS:
    void valueChanged(bool);
};

////////////////////////////////////////////////////////////////////////////////
// String property class
// 
template <typename ControlType>
class StringProperty : public TaskProperty
{
public:
    StringProperty(std::list<TaskProperty*>& list, const QString& name, const QString& value) : name(name), value(value)
    {
        list.push_back(this);
    };
    
    template <typename... Params>
    QWidget* createControl(Params&&... parameters)
    {
        control = new ControlType(parameters...);
        control->setText(value);
        connect(control, &ControlType::textChanged, this, &StringProperty::setValue);
        return control;
    }

    template <>
    QWidget* createControl<const QStringList&>(const QStringList& list)
    {
        control = new ControlType();
        control->addItem("");
        control->addItems(list);
        control->setCurrentText(value);
        connect(control, &ControlType::currentTextChanged, this, &StringProperty::setValue);
        return control;
    }

    virtual void toJson(QJsonObject& json) const override
    {
        if (!name.isEmpty())
            json[name] = QJsonValue(value);
    }

    virtual void fromJson(const QJsonObject& json) override
    {
        if (!name.isEmpty())
            value = json[name].toString(value);
    }

    void setValue(const QString & value)
    {
        this->value = value;
        emit changed();
    }

    ControlType* control = nullptr;

    QString name;
    QString value;
};

template <typename SpinBox>
class StringProperty<ToggleBox<SpinBox>> : public TaskProperty
{
public:
    using ValueType = typename std::conditional<std::is_same<SpinBox, QSpinBox>::value, int, double>::type;

    StringProperty(std::list<TaskProperty*>& list, const QString& name, const QString& text) : name(name)
    {
        std::tie(is_variable, value) = parseText(text);
        list.push_back(this);
    };

    QWidget* createControl(const QStringList& list)
    {
        control = new ToggleBox<SpinBox>();
        control->addItem("");
        control->addItems(list);
        control->setText(is_variable, value);
        control->toggleControl(is_variable);
        connect(control, &ToggleBox<SpinBox>::currentTextChanged, this, &StringProperty::setVariable);
        connect(control, QOverload<ValueType>::of(&ToggleBox<SpinBox>::valueChanged), this, &StringProperty::setValue);
        connect(control, &ToggleBox<SpinBox>::toggled, this, &StringProperty::toggle);
        return control;
    }

    virtual void toJson(QJsonObject& json) const override
    {
        QString formatted_value = QString("%1:%2").arg(is_variable ? "var" : "val").arg(value);
        if (!name.isEmpty())
            json[name] = QJsonValue(formatted_value);
    }

    virtual void fromJson(const QJsonObject& json) override
    {
        if (!name.isEmpty())
        {
            QString formatted_value = QString("%1:%2").arg(is_variable ? "var" : "val").arg(value);
            formatted_value = json[name].toString(formatted_value);
            std::tie(is_variable, value) = parseText(formatted_value);
        }
    }

    static std::pair<bool, QString> parseText(const QString& text)
    {
        std::pair<bool, QString> parsed_text{false, ""};
        QStringList part = text.split(':');
        if (part.count() < 2)
        {
            parsed_text.first = true;
            parsed_text.second = text;
        }
        else
        {
            parsed_text.second = part[1];
            if (part[0].contains("val"))
                parsed_text.first = false;
            else if(part[0].contains("var"))
                parsed_text.first = true;
        }
        return parsed_text;
    }

    void setVariable(const QString& value)
    {
        this->value = value;
        control->value.blockSignals(true);
        control->setText(true, value);
        control->value.blockSignals(false);
        emit changed();
    }

    void setValue(ValueType value)
    {
        this->value = QString("%1").arg(value);
        control->value.blockSignals(true);
        control->value.setValue(value);
        control->value.blockSignals(false);
        emit changed();
    }

    void toggle(bool v)
    {
        is_variable = v;
        if (control) value = control->text();
    }

    ToggleBox<SpinBox>* control = nullptr;

    QString name;
    QString value;        // this is variable name or value in string
    bool    is_variable;  // variable or value
};

typedef StringProperty<ToggleBox<QSpinBox>>        IntVarProperty;
typedef StringProperty<ToggleBox<QDoubleSpinBox>>  DoubleVarProperty;

////////////////////////////////////////////////////////////////////////////////
// StringList property class

class ListItemDelegate : public QItemDelegate
{
public:
    explicit ListItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
private:
    bool mouseCursorIsOnRemoveIcon(QMouseEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    const QIcon remove_icon;
};

class ListModel : public QStringListModel
{
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
};

class StringListProperty : public TaskProperty
{
public:
    StringListProperty(std::list<TaskProperty*>& properties_list, const QString & name, QStringList list);
    virtual QWidget* createControl();
    virtual void toJson(QJsonObject& json) const override;
    virtual void fromJson(const QJsonObject& json) override;
    void insertValue(const QVariant & value);

    QListView* control = nullptr;
    ListItemDelegate item_delegate;
    ListModel        list_model;

private:
    QString name;
};

#endif // TASK_PROPERTY_H