#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>
#include <windows.h>
#include <dbt.h>
#include "MTLtask.h"
#include "com_trigger_task.h"

DeviceEventFilter::DeviceEventFilter() :
    QObject(), QAbstractNativeEventFilter()
{
}

bool DeviceEventFilter::nativeEventFilter(const QByteArray& eventType, void* message, long*)
{
    if (eventType == "windows_generic_MSG")
    {
        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_DEVICECHANGE)
        {
            if (msg->wParam == DBT_DEVICEARRIVAL || 
                msg->wParam == DBT_DEVICEREMOVECOMPLETE ||
                msg->wParam == DBT_DEVNODES_CHANGED)
            {
                // connect to this signal to reread available ports or devices etc
                emit serialDeviceChanged();
            }
        }
    }
    return false;
}

/// @brief class SerialList
class SerialList : public QObject
{
public:
    static SerialList& inst()
    {
        static SerialList inst;
        return inst;
    }

    static QSerialPort& port(const QString & port)
    {
        if (inst().serial_map.find(port) == inst().serial_map.end())
        {
            inst().serial_map[port] = new QSerialPort(port);
            QObject::connect(inst().serial_map[port], &QSerialPort::errorOccurred, &inst(), &SerialList::onError);
        }
        return  *inst().serial_map[port];
    }

    static QStringList availablePorts()
    {
        QStringList seriallist;
        QMap<QString, QSerialPort*> new_map(inst().serial_map);
        for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
        {
            // add port to static map
            SerialList::port(info.portName());
            // add port name to list
            seriallist.append(info.portName());
            //
            new_map.remove(info.portName());
        }
        // remove non-existent ports
        QMapIterator<QString, QSerialPort*> i(new_map);
        while (i.hasNext()) 
        {
            i.next();
            delete inst().serial_map[i.key()];
            inst().serial_map.remove(i.key());
        }
        return seriallist;
    }

private:
    SerialList() 
    {
        static DeviceEventFilter ef;
        QApplication::instance()->installNativeEventFilter(&ef);
        connect(&ef, &DeviceEventFilter::serialDeviceChanged, this, &SerialList::serialDeviceChange);
    }
    ~SerialList() 
    {
        for (auto i = serial_map.begin(); i != serial_map.end(); ++i)
            if (i.value()) delete i.value();
        serial_map.clear();
    }
    SerialList(const SerialList& root) = delete;
    SerialList& operator=(const SerialList&) = delete;

    void onError(QSerialPort::SerialPortError error)
    {
        if (error == QSerialPort::ResourceError)
        {
            QSerialPort* port = dynamic_cast<QSerialPort*>(sender());
            port = inst().serial_map[port->portName()];
            if (port)
                port->close();
        }
    }

    void serialDeviceChange()
    {
        availablePorts();
    }

    QMap<QString, QSerialPort*> serial_map;
};

/// @brief class ComTriggerTask
const QString TaskCreator<ComTriggerTask>::name = TaskCreator<ComTriggerTask>::addTaskName("ComTrigger");


ComTriggerTask::ComTriggerTask(Task* parent) 
    :Task(parent)
    , rts_trigger(properties, "rts_trigger", false), inv_rts(properties, "inv_rts", false)
    , dtr_trigger(properties, "dtr_trigger", false), inv_dtr(properties, "inv_dtr", false)
    , serial_port(properties, "port", "")
    , duration(properties, "duration", 100, 0, INT_MAX)
    , logfile(properties, "logfile", "")
    , variable_name(properties, "", "")
    , output_template(properties, "output_template", "")
    , period(properties, "period", 1000, 0, INT_MAX)
    , ignore_power_status(properties, "ignore_power_status", false)
    , triggers_number(properties, "triggers_number", 1, 0, INT_MAX)
{
    connectPropertiesToRoot({ &serial_port, &duration });
}

void ComTriggerTask::createForm(QFormLayout *layout)
{
    seriallist_ = SerialList::availablePorts();
    layout->addRow(new QLabel("Port: "), serial_port.createControl<const QStringList&>(seriallist_));
    connect(serial_port.control, QOverload<int>::of(&QComboBox::currentIndexChanged), this, QOverload<int>::of(&ComTriggerTask::setPort));
    // ----- RTS 
    layout->addRow(rts_trigger.createControl("RTS Trigger"), inv_rts.createControl("Invert Level of RTS"));
    connect(&inv_rts, &BoolProperty::valueChanged, this, &ComTriggerTask::setInvRTS);
//    connect(this, &ComTriggerTask::toggleInvRTS, &inv_rts, &BoolProperty::setValue);
    // ----- DTR
    layout->addRow(dtr_trigger.createControl("DTR Trigger"), inv_dtr.createControl("Invert Level of DTR"));
    connect(&inv_dtr, &BoolProperty::valueChanged, this, &ComTriggerTask::setInvDTR);
//    connect(this, &ComTriggerTask::toggleInvDTR, &inv_dtr, &BoolProperty::setValue);
    //
    setPort(serial_port.value);
    // ----- duration
    layout->addRow("Duration: ", duration.createControl("", ""));
    duration.control->setSuffix(" ms");
    // ----- triggers number
    layout->addRow(new QLabel("Triggers Number: "), triggers_number.createControl(QString(), QString()));
    // ----- period
    layout->addRow(new QLabel("Period: "), period.createControl(QString(), " ms"));
    // ----- vars for template
    QPushButton* add_var_btn = new QPushButton("Add Var");
    connect(add_var_btn, &QPushButton::clicked, this, &ComTriggerTask::addVarToTemplate);
    layout->addRow(add_var_btn, variable_name.createControl<const QStringList&>(getVariables()));
    // ---- var template
    layout->addRow("Output: ", output_template.createControl(layout->parentWidget()));
    // ---- log file
    layout->addRow("Log File: ", logfile.createControl(layout->parentWidget(), false, true));
    // ----- ignore power status
    layout->addRow(new QLabel("Ignore Power Status: "), ignore_power_status.createControl());
}

void ComTriggerTask::run()
{
    if (checkSkipping())
        return;
    if (MtlController::checkPower() == false && ignore_power_status == false)
        return;

    QSerialPort& serial = SerialList::port(serial_port.value);
    if (serial.isOpen() == false)
        serial.open(QIODevice::ReadWrite);

    int n = 0;
    QEventLoop loop;
    QTimer t;
    t.setSingleShot(false);
    t.setTimerType(Qt::PreciseTimer);
    t.connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
    t.start(period);
    do
    {
        // invert levels
        if (this->dtr_trigger.value) serial.setDataTerminalReady(!serial.isDataTerminalReady());
        if (this->rts_trigger.value) serial.setRequestToSend(!serial.isRequestToSend());
        QThread::msleep(duration);
        // invert levels
        if (this->dtr_trigger.value) serial.setDataTerminalReady(!serial.isDataTerminalReady());
        if (this->rts_trigger.value) serial.setRequestToSend(!serial.isRequestToSend());
        //
        writeToLog();
        // timeout
        loop.exec();
        n++;
    } while (
        MtlDataModel::mtltask_state == MtlDataModel::kBusy && 
        !break_loop && 
        MtlController::checkPower() &&
        (triggers_number == 0 || n < triggers_number));
    t.stop();
    //timer_handler_.start();
    //do
    //{
    //    //
    //    writeToLog();
    //    timer_handler_.wait();
    //} while (MtlDataModel::mtltask_state == MtlDataModel::kBusy && !break_loop && MtlController::checkPower());
    //timer_handler_.stop();
    //if (timer_handler_.trigger_timer_.isActive())
    //    timer_handler_.trigger_loop_.exec();
    //else
    //{
    //    if (this->dtr_trigger.value) serial.setDataTerminalReady(!serial.isDataTerminalReady());
    //    if (this->rts_trigger.value) serial.setRequestToSend(!serial.isRequestToSend());
    //    QThread::msleep(duration);
    //    if (this->dtr_trigger.value) serial.setDataTerminalReady(!serial.isDataTerminalReady());
    //    if (this->rts_trigger.value) serial.setRequestToSend(!serial.isRequestToSend());
    //    writeToLog();
    //}
}

const QString& ComTriggerTask::taskname()
{
    return name;
}

void ComTriggerTask::read(const QJsonObject & json)
{
    Task::read(json);

    if (!serial_port.value.isEmpty())
    {
        QSerialPort& serial = SerialList::port(serial_port.value);
        if (serial.isOpen() == false)
        {
            serial.open(QIODevice::ReadWrite);
            serial.setRequestToSend(inv_rts.value);
            serial.setDataTerminalReady(inv_dtr.value);
        }
    }
}

void ComTriggerTask::write(QJsonObject& json)
{
    Task::write(json);
    if (!serial_port.value.isEmpty())
    {
        QSerialPort& serial = SerialList::port(serial_port.value);
        if (serial.isOpen())
        {
            inv_rts.setValue(serial.isRequestToSend());
            inv_dtr.setValue(serial.isDataTerminalReady());
        }
    }
}

QString ComTriggerTask::information()
{
    return QString("%1 port=%2 timeout=%3").arg(name).arg(serial_port.value).arg(duration);
}

void ComTriggerTask::initialize()
{
    if (serial_port.value.isEmpty()) return;

    QSerialPort& serial = SerialList::port(serial_port.value);
    if (serial.isOpen() == false)
        serial.open(QIODevice::ReadWrite);
}

void ComTriggerTask::writeToLog()
{
    if (logfile.value.isEmpty()) return;
    // save
    QFile file(logfile.value);
    if (file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << replaceNamesWithValues(output_template.value) << endl;
    }
}

void ComTriggerTask::invert()
{
    QSerialPort& serial = SerialList::port(serial_port.value);
    if (serial.isOpen() == false)
        serial.open(QIODevice::ReadWrite);
    if (dtr_trigger.value)
        serial.setDataTerminalReady(!serial.isDataTerminalReady());
    if (rts_trigger.value)
        serial.setRequestToSend(!serial.isRequestToSend());
}

void ComTriggerTask::setPort(int index)
{
    if (0 <= index-1 && index-1 < seriallist_.size())
        serial_port.setValue(seriallist_[index - 1]);
    else
        return;

    setPort(serial_port.value);
}

void ComTriggerTask::setPort(const QString & port_name)
{
    // set levels DTR & RTS
    if (!port_name.isEmpty())
    {
        serial_port.setValue(port_name);
        QSerialPort& serial = SerialList::port(serial_port.value);
        if (serial.isOpen() == false)
            serial.open(QIODevice::ReadWrite);
        inv_rts.setValue(serial.isRequestToSend());
        inv_dtr.setValue(serial.isDataTerminalReady());
    }
}

void ComTriggerTask::setInvRTS(bool s)
{
    if (serial_port.value.isEmpty()) return;
    QSerialPort& serial = SerialList::port(serial_port.value);
    if (serial.isOpen() == false)
        serial.open(QIODevice::ReadWrite);

    serial.setRequestToSend(s);
}

void ComTriggerTask::setInvDTR(bool s)
{
    if (serial_port.value.isEmpty()) return;
    QSerialPort& serial = SerialList::port(serial_port.value);
    if (serial.isOpen() == false)
        serial.open(QIODevice::ReadWrite);

    serial.setDataTerminalReady(s);
}

void ComTriggerTask::addVarToTemplate()
{
    output_template.control->insert("{" + variable_name.value + "}");
}

