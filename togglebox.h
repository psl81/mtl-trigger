
#ifndef TOGGLEBOX_H
#define TOGGLEBOX_H

#include <QHBoxLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QToolButton>

class ComboSpinBox : public QWidget
{
    Q_OBJECT

Q_SIGNALS:
    void currentTextChanged(const QString& text);
    void toggled(bool);
    void valueChanged(int);
    void valueChanged(double);
};

template<typename SpinBoxType>
class ToggleBox : public ComboSpinBox
{
public:
    using ValueType = typename std::conditional<std::is_same<SpinBoxType, QSpinBox>::value, int, double>::type;

    ToggleBox(bool is_variable = false, QWidget *parent = nullptr) :
        layout(parent),
        toggleButton(parent),
        variable(parent),
        value(parent),
        is_variable(is_variable)
    {
        this->setLayout(&layout);
        value.setRange(-1e9, 1e9);
        if constexpr (std::is_same<SpinBoxType, QDoubleSpinBox>::value)
            value.setDecimals(3);
        layout.setContentsMargins({0,0,0,0});
        connect(&toggleButton, &QToolButton::pressed, this, &ToggleBox::toggle);
        connect(&toggleButton, &QToolButton::toggled, this, &ComboSpinBox::toggled);
        connect(&variable, &QComboBox::currentTextChanged, this, &ComboSpinBox::currentTextChanged);
        connect(&value, QOverload<ValueType>::of(&SpinBoxType::valueChanged), this, QOverload<ValueType>::of(&ComboSpinBox::valueChanged));
        layout.addWidget(&toggleButton);
        layout.addWidget(&value);
        layout.addWidget(&variable);
        toggleControl(is_variable);
    }

    void toggleControl(bool is_variable)
    {
        this->is_variable = is_variable;
        if (is_variable)
        {
            toggleButton.setText("#");
            value.hide();
            variable.show();
        }
        else
        {
            toggleButton.setText("{}");
            variable.hide();
            value.show();
        }
        emit toggleButton.toggled(is_variable);
    }

    QString text()
    {
        if (is_variable)
        {
            return variable.currentText();
        }
        else
        {
            return QString("%1").arg(value.value());
        }
    }

    void setText(bool is_variable, const QString& value)
    {
        this->is_variable = is_variable;
        if (is_variable)
        {
            variable.setCurrentText(value);
        }
        else
        {
            if constexpr (std::is_same<SpinBoxType, QSpinBox>::value)
                this->value.setValue(value.toInt());
            else
                this->value.setValue(value.toDouble());
        }
    }

    void addItem(const QString& text)
    {
        variable.addItem(text);
    }

    void addItems(const QStringList& texts)
    {
        variable.addItems(texts);
    }

    QComboBox       variable;
    SpinBoxType     value;
private:
    void toggle()
    {
        is_variable = !is_variable;
        toggleControl(is_variable);
    }

protected:
    QHBoxLayout     layout;
    QToolButton     toggleButton;

    bool            is_variable;
};


#endif // TOGGLEBOX_H
