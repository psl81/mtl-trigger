#include <optional>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSerialPortInfo>
#include <QPushButton>
#include <QFile>
#include <QEventLoop>
#include <QTimer>
#include <QApplication>
#include "MTLtask.h"
#include "lusb0_usb.h"
#include "com_trigger_task.h"
#include "digispark_trigger_task.h"

///
class DigiSparkDevice
{
private:
    struct usb_device* device_ = nullptr;
    struct usb_dev_handle* device_handle_ = nullptr;
    bool level_ = false;

private:
    bool open() {
        if (device_)
            device_handle_ = usb_open(device_);
        if (device_handle_ != nullptr)
            return true;
        return false;
    }

    void close() {
        if (device_handle_) {
            usb_close(device_handle_);
            device_handle_ = nullptr;
        }
    }
public:
    DigiSparkDevice() = default;
    DigiSparkDevice(const DigiSparkDevice&) = default;

    DigiSparkDevice(struct usb_device* device) 
        : device_(device)
    {
        open();
    }

    ~DigiSparkDevice() {
        close();
    }

    //bool find() {
    //    device_ = nullptr;
    //    device_handle_ = nullptr;
    //    struct usb_bus* bus = nullptr;
    //    struct usb_device* device = nullptr;
    //    // Enumerate the USB device tree
    //    usb_find_busses();
    //    usb_find_devices();
    //    // Iterate through attached busses and devices
    //    bus = usb_get_busses();
    //    while (bus != nullptr) {
    //        device = bus->devices;
    //        while (device != nullptr) {
    //            // Check to see if each USB device matches the DigiSpark Vendor and Product IDs
    //            if ((device->descriptor.idVendor == 0x16c0) && (device->descriptor.idProduct == 0x05df)) {
    //                device_ = device;
    //                open();
    //                break;
    //            }
    //            device = device->next;
    //        }
    //        bus = bus->next;
    //    }
    //    return device_ != nullptr ? true : false;
    //}

    void setLevel(bool level) {
        int result = 0;
        level_ = level;
        if (device_handle_) {
            char cmd = level ? 'H' : 'L';
            result = usb_control_msg(device_handle_, (0x01 << 5), 0x09, 0, cmd, 0, 0, 0);
        }
    }

    bool getLevel() {
        return level_;
    }
};

/// @brief class DigiSparkList
class DigiSparkList : public QObject
{
    std::shared_ptr<DigiSparkDevice> device_;
public:
    static DigiSparkList& inst()
    {
        static DigiSparkList inst;
        return inst;
    }

    static std::shared_ptr<DigiSparkDevice> device() {
        return inst().device_;
    }

    static void findDevices()
    {
        std::shared_ptr<DigiSparkDevice> new_device;
        struct usb_bus* bus = nullptr;
        struct usb_device* device = nullptr;

        // Enumerate the USB device tree
        usb_find_busses();
        usb_find_devices();
        // Iterate through attached busses and devices
        bus = usb_get_busses();
        while (bus != nullptr) {
            device = bus->devices;
            while (device != nullptr) {
                // Check to see if each USB device matches the DigiSpark Vendor and Product IDs
                if ((device->descriptor.idVendor == 0x16c0) && (device->descriptor.idProduct == 0x05df)) {
                    // add device
                    new_device = std::make_shared<DigiSparkDevice>(device);
                    break;
                }
                device = device->next;
            }
            bus = bus->next;
        }
        inst().device_ = new_device;
        if (inst().deviceFound)
            inst().deviceFound(new_device ? true : false);
    }

    std::function<void(bool)> deviceFound;

private:
    DigiSparkList()
    {
        // Initialize the USB library
        usb_init();
        // Create USB event filter
        static DeviceEventFilter ef;
        QApplication::instance()->installNativeEventFilter(&ef);
        connect(&ef, &DeviceEventFilter::serialDeviceChanged, this, &DigiSparkList::usbDeviceChange);
    }
    ~DigiSparkList() = default;
    DigiSparkList(const DigiSparkList& other) = delete;
    DigiSparkList& operator=(const DigiSparkList&) = delete;

    void usbDeviceChange()
    {
        findDevices();
    }
};

/// @brief class DigiSparkTriggerTask
const QString TaskCreator<DigiSparkTriggerTask>::name = TaskCreator<DigiSparkTriggerTask>::addTaskName("DigiSparkTrigger");


DigiSparkTriggerTask::DigiSparkTriggerTask(Task* parent) 
    : Task(parent)
    , inv_trigger(properties, "inv_trigger", false)
    , duration(properties, "duration", 100, 0, INT_MAX)
    , logfile(properties, "logfile", "")
    , variable_name(properties, "", "")
    , output_template(properties, "output_template", "")
    , period(properties, "period", 1000, 0, INT_MAX)
    , ignore_power_status(properties, "ignore_power_status", false)
    , triggers_number(properties, "triggers_number", 1, 0, INT_MAX)
{
    DigiSparkList::inst().deviceFound = [this](bool found) {
        this->deviceFoundEvent(found ? "Found" : "Not Found");
        };
}

DigiSparkTriggerTask::~DigiSparkTriggerTask()
{
    DigiSparkList::inst().deviceFound = nullptr;
}

void DigiSparkTriggerTask::createForm(QFormLayout *layout)
{
    // device label
    DigiSparkList::findDevices();
    device = DigiSparkList::device();
    QLabel *device_state = new QLabel(device ? "Found" : "Not Found");
    connect(this, &DigiSparkTriggerTask::deviceFoundEvent, device_state, &QLabel::setText);
    layout->addRow(new QLabel("Device: "), device_state);
    // ----- Trigger
    layout->addRow(inv_trigger.createControl("Invert Trigger Level"));
    connect(&inv_trigger, &BoolProperty::valueChanged, this, &DigiSparkTriggerTask::setInvTrigger);
    //
    // ----- duration
    layout->addRow("Duration: ", duration.createControl("", ""));
    duration.control->setSuffix(" ms");
    // ----- triggers number
    layout->addRow(new QLabel("Triggers Number: "), triggers_number.createControl(QString(), QString()));
    // ----- period
    layout->addRow(new QLabel("Period: "), period.createControl(QString(), " ms"));
    // ----- vars for template
    QPushButton* add_var_btn = new QPushButton("Add Var");
    connect(add_var_btn, &QPushButton::clicked, this, &DigiSparkTriggerTask::addVarToTemplate);
    layout->addRow(add_var_btn, variable_name.createControl<const QStringList&>(getVariables()));
    // ---- var template
    layout->addRow("Output: ", output_template.createControl(layout->parentWidget()));
    // ---- log file
    layout->addRow("Log File: ", logfile.createControl(layout->parentWidget(), false, true));
    // ----- ignore power status
    layout->addRow(new QLabel("Ignore Power Status: "), ignore_power_status.createControl());
}

void DigiSparkTriggerTask::run()
{
    if (checkSkipping())
        return;
    if (MtlController::checkPower() == false && ignore_power_status == false)
            return;

    auto device = DigiSparkList::device();
    if (static_cast<bool>(device) == false)
        return;

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
        device->setLevel(!device->getLevel());
        QThread::msleep(duration);
        // invert levels
        device->setLevel(!device->getLevel());
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
}

const QString& DigiSparkTriggerTask::taskname()
{
    return name;
}

void DigiSparkTriggerTask::read(const QJsonObject & json)
{
    Task::read(json);

    if (device)
    {
        device->setLevel(inv_trigger.value);
    }
}

void DigiSparkTriggerTask::write(QJsonObject& json)
{
    Task::write(json);
    if (device)
    {
        inv_trigger.setValue(device->getLevel());
    }
}

QString DigiSparkTriggerTask::information()
{
    return QString("%1 timeout=%2").arg(name).arg(duration);
}

void DigiSparkTriggerTask::initialize()
{
}

void DigiSparkTriggerTask::writeToLog()
{
    if (logfile.value.isEmpty()) return;
    // save
    QFile file(logfile.value);
    if (file.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream stream(&file);
        stream << replaceNamesWithValues(output_template.value) << Qt::endl;
    }
}

void DigiSparkTriggerTask::setInvTrigger(bool s)
{
    if (device)
        device->setLevel(s);
}

void DigiSparkTriggerTask::addVarToTemplate()
{
    output_template.control->insert("{" + variable_name.value + "}");
}

