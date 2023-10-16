#ifndef COM_TRIGGER_TASK_H
#define COM_TRIGGER_TASK_H

#include <QAbstractNativeEventFilter>
#include <QSerialPort>
#include <QLineEdit>
#include <QComboBox>
#include <QTimer>
#include <QEventLoop>
#include "linefilebrowse.h"
#include "task_property.h"
#include "task.h"

class DeviceEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    DeviceEventFilter();
    bool nativeEventFilter(const QByteArray& eventType, void* message, long*) override;

signals:
    void serialDeviceChanged();
};

template<typename TaskType>
struct TimerHandler : public QObject
{
    TimerHandler(TaskType& task)
        : task_(task)
    {
        moveToThread(&task_);
        trigger_timer_.moveToThread(&task_);
        trigger_loop_.moveToThread(&task_);
        period_timer_.moveToThread(&task_);
        period_loop_.moveToThread(&task_);

        trigger_timer_.setTimerType(Qt::PreciseTimer);
        period_timer_.setTimerType(Qt::PreciseTimer);
        trigger_timer_.setSingleShot(true);
        trigger_timer_.callOnTimeout(&task_, &TaskType::invert, Qt::DirectConnection);
        trigger_timer_.callOnTimeout(&trigger_loop_, &QEventLoop::quit, Qt::DirectConnection);
        period_timer_.callOnTimeout(&period_loop_, &QEventLoop::quit, Qt::DirectConnection);
        period_timer_.callOnTimeout(this, &TimerHandler::raiseTrigger, Qt::DirectConnection);
    }
    TaskType&    task_;
    QTimer       trigger_timer_;
    QEventLoop   trigger_loop_;
    QTimer       period_timer_;
    QEventLoop   period_loop_;

    void start() {
        task_.invert();
        trigger_timer_.start();
        period_timer_.start();
    }

    void stop() {
        period_timer_.stop();
    }

    void wait() {
        if (trigger_timer_.isActive())
            trigger_loop_.exec();
        if (period_timer_.isActive())
            period_loop_.exec();
    }

    void raiseTrigger()
    {
        if (!trigger_timer_.isActive()) {
            task_.invert();
            trigger_timer_.start();
        }
    }
};

/// @brief class ComTriggerTask
class ComTriggerTask : public Task, public TaskCreator<ComTriggerTask>
{
    Q_OBJECT

    // properties
    StringProperty<QComboBox>      variable_name;
    StringProperty<LineFileBrowse> logfile;
    NumProperty<int>               duration;
    StringProperty<QComboBox>      serial_port;
    BoolProperty                   rts_trigger;
    BoolProperty                   inv_rts;
    BoolProperty                   dtr_trigger;
    BoolProperty                   inv_dtr;
    StringProperty<QLineEdit>      output_template;    // template for output filename
    NumProperty<int>               period;             // period between pulse in multiple shoot mode
    BoolProperty                   ignore_power_status;
    NumProperty<int>               triggers_number;
  public:
    ComTriggerTask(Task* parent = nullptr);
    // form
    virtual void createForm(QFormLayout *layout) override;
    // execute task
    virtual void run() override;
    // task name
    virtual const QString& taskname() override;
    // read & write
    virtual void read(const QJsonObject & json) override;
    virtual void write(QJsonObject& json) override;
    //
    virtual QString information() override;
    // initialize task
    virtual void initialize() override;
    //
    void writeToLog();
    void invert();
  private:
    QStringList  seriallist_;

    void setPort(int index);
    void setPort(const QString& port_name);
    void setInvRTS(bool s);
    void setInvDTR(bool s);
    void addVarToTemplate();
  signals:
    void toggleInvRTS(bool s);
    void toggleInvDTR(bool s);
};

#endif // COM_TRIGGER_TASK_H
