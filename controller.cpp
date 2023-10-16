#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QFile>
#include "com_trigger_task.h"
#include "digispark_trigger_task.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "controller.h"

Controller::Controller(MtlController& mtl_controller)
    : mtl_controller_(mtl_controller)
    , is_trigger_enable_(true)
{
    QString config_dir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    QDir().mkpath(config_dir);
    config_filepath_ = config_dir + "/mtl-trigger";
    loadConfig();
}

Controller::~Controller()
{
}

void Controller::setMainWindow(MainWindow* main_window)
{
    main_window_ = main_window;
}

void Controller::onTriggerTypeChanged(const QString& type)
{
    static QString current_type;
    if (current_type == type)
        return;
    // save
    saveTask();
    current_type = type;
    main_window_->clearLayout(main_window_->ui->trigger_properties->layout());
    //
    if (type == TriggerType::digispark)
        trigger_task_ = std::make_unique<DigiSparkTriggerTask>();
    else if (type == TriggerType::com_port)
        trigger_task_ = std::make_unique<ComTriggerTask>();
    // load
    loadTask();
    //
    if (trigger_task_)
        trigger_task_->createForm(qobject_cast<QFormLayout*>(main_window_->ui->trigger_properties->layout()));
}

void Controller::onDataReceived(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    QString cy_value;
    mtl_controller_.model()->getValue("CY-Actuator", cy_value);
    int cy = cy_value.toInt();
    static int cy_pre = cy;
    static int low_count = 0;
    static int high_count = 0;
    bool is_low = cy - cy_pre < 2;

    // diagram
    // cy - cy_pre   23462431000012152
    // is_low        00000001111110100
    // low_count     00000001234560100
    // is_high       11111110000001011
    // high_count    12345670000001012
    // raise         00000000100000000
    // fall          01000000000000001

    if (is_low) {
        low_count++;
        high_count = 0;
    }
    else {
        low_count = 0;
        high_count++;
    }

    if (low_count == 2) {
        // raise trigger
        trigger_task_->break_loop = false;
        if (is_trigger_enable_) trigger_task_->start();
    }

    if (high_count == 2) {
        // fall trigger
        trigger_task_->break_loop = true;
    }

    cy_pre = cy;
}

void Controller::enableTrigger(bool enable)
{
    is_trigger_enable_ = enable;
}

void Controller::loadSetings()
{
    if (main_window_ != nullptr)
    {
        auto curent_type = settings_[ConfigValues::curent_type].toString(TriggerType::digispark.toString());
        main_window_->ui->trigger_type->setCurrentText(curent_type);
        auto ignore_power_status = settings_[ConfigValues::ignore_power_status].toBool();
        main_window_->ui->ignorepowerstatus_chb->setChecked(ignore_power_status);
        is_trigger_enable_ = settings_[ConfigValues::trigger_enable].toBool(true);
        main_window_->ui->trigger_switch->setChecked(!is_trigger_enable_);
    }
}

void Controller::saveSetings()
{
    if (main_window_ != nullptr)
    {
        settings_[ConfigValues::curent_type] = main_window_->ui->trigger_type->currentText();
        settings_[ConfigValues::ignore_power_status] = QJsonValue(main_window_->ui->ignorepowerstatus_chb->isChecked());
        settings_[ConfigValues::trigger_enable] = is_trigger_enable_;

        saveTask();
    }

    saveConfig();
}

void Controller::saveTask()
{
    if (trigger_task_) {
        QJsonObject task_json;
        trigger_task_->write(task_json);
        settings_[trigger_task_->taskname()] = task_json;
    }
}

void Controller::loadTask()
{
    if (trigger_task_) {
        auto task_json = settings_[trigger_task_->taskname()].toObject();
        trigger_task_->read(task_json);
    }
}

void Controller::saveConfig()
{
    QFile save_file(config_filepath_);

    if (!save_file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open save file.");
        return/* false*/;
    }

    QJsonDocument save_doc;
    save_doc.setObject(settings_);
    save_file.write(save_doc.toJson());
}

void Controller::loadConfig()
{
    QFile load_file(config_filepath_);

    if (!load_file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return /*false*/;
    }

    QByteArray load_data = load_file.readAll();
    QJsonDocument load_doc(QJsonDocument::fromJson(load_data));
    settings_ = load_doc.object();
}