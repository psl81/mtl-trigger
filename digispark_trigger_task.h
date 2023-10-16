#pragma once

#include <QAbstractNativeEventFilter>
#include <QSerialPort>
#include <QLineEdit>
#include <QComboBox>
#include "linefilebrowse.h"
#include "task_property.h"
#include "task.h"

class DigiSparkDevice;

/// @brief class ComTriggerTask
class DigiSparkTriggerTask : public Task, public TaskCreator<DigiSparkTriggerTask>
{
    Q_OBJECT

    // properties
    StringProperty<QComboBox>      variable_name;
    StringProperty<LineFileBrowse> logfile;
    NumProperty<int>               duration;
    BoolProperty                   inv_trigger;
    StringProperty<QLineEdit>      output_template;     // template for output filename
    NumProperty<int>               period;              // period between pulse in multiple shoot mode
    BoolProperty                   ignore_power_status;
    NumProperty<int>               triggers_number;
  public:
    DigiSparkTriggerTask(Task* parent = nullptr);
    ~DigiSparkTriggerTask();
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
  private:
    std::shared_ptr<DigiSparkDevice>  device;
  private slots:
    void setInvTrigger(bool s);
    void addVarToTemplate();
  signals:
    void toggleInvRTS(bool s);
    void toggleInvDTR(bool s);
    void deviceFoundEvent(QString);
};
