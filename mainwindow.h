#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "task.h"

namespace Ui {
    class MainWindow;
}

class Controller;

/// @brief class MainWindow
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        Controller & controller,
        MtlController& mtl_controller,
        QWidget* parent = 0);
    ~MainWindow();

    void onMachineStateChanged(bool state);
    void init();
    static void clearLayout(QLayout* layout);
    void enableTrigger(bool enable);

    Ui::MainWindow* ui;

private slots:
    void on_ignorepowerstatus_chb_toggled(bool checked);
    void on_connect_btn_clicked();

private:
    MtlController & mtl_controller_;
    Controller & app_controller_;
};

#endif // MAINWINDOW_H
