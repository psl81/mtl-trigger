#pragma once

#include <QObject>
#include <QJsonObject>

class MainWindow;
class MtlController;
class Task;

struct TriggerType {
    TriggerType() = delete;
    constexpr static QStringView digispark = u"DigiSpark";
    constexpr static QStringView com_port = u"Com Port";
};

struct ConfigValues {
    ConfigValues() = delete;
    constexpr static QStringView curent_type = u"curent_type";
    constexpr static QStringView ignore_power_status = u"ignore_power_status";
    constexpr static QStringView trigger_enable = u"trigger_enable";
};

class Controller : public QObject
{
    Q_OBJECT
private:
    QString config_filepath_;
    QJsonObject settings_;
    MainWindow * main_window_;
    MtlController& mtl_controller_;
    std::unique_ptr<Task> trigger_task_;
    bool is_trigger_enable_;
public:
    Controller(MtlController& mtl_controller);
    ~Controller();
    void setMainWindow(MainWindow* main_window);
    void onTriggerTypeChanged(const QString& type);
    void onDataReceived(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = { Qt::DisplayRole });
    void enableTrigger(bool enable);
    void loadSetings();
    void saveSetings();
private:
    void saveTask();
    void loadTask();
    void saveConfig();
    void loadConfig();
};