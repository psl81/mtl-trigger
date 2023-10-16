#include <QMouseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include "controller.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

///
/// @brief class MainWindow
MainWindow::MainWindow(
    Controller& controller,
    MtlController& mtl_controller,
    QWidget* parent)
    : QMainWindow(parent)
    , mtl_controller_(mtl_controller)
    , app_controller_(controller)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    app_controller_.saveSetings();
    delete ui;
}

void MainWindow::onMachineStateChanged(bool state)
{
    QPalette pal;
    pal.setColor(QPalette::Background, state ? Qt::green : Qt::red);
    ui->state_frame->setAutoFillBackground(true);
    ui->state_frame->setPalette(pal);
}

void MainWindow::init()
{
    ui->connect_btn->setEnabled(MTL32::isLoaded());
    ui->data_view->setModel(mtl_controller_.model());
    connect(mtl_controller_.model(), &MtlDataModel::machineStateChanged, this, &MainWindow::onMachineStateChanged, Qt::QueuedConnection);
    connect(mtl_controller_.model(), &MtlDataModel::dataReceived, &app_controller_, &Controller::onDataReceived);
    //
    connect(ui->trigger_type, &QComboBox::currentTextChanged, &app_controller_, &Controller::onTriggerTypeChanged);
    ui->trigger_type->addItems({ TriggerType::digispark.toString(), TriggerType::com_port.toString() });
    //
    connect(ui->trigger_switch, &QPushButton::toggled, this, &MainWindow::enableTrigger);
    //
    app_controller_.loadSetings();
}

void MainWindow::clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while (layout->count())
    {
        item = layout->takeAt(0);

        if (item->layout())
        {
            clearLayout(item->layout());
            delete item->layout();
        }
        if (item->widget())
        {
            delete item->widget();
        }
        delete item;
    }
}

void MainWindow::enableTrigger(bool enable)
{
    this->ui->trigger_switch->setText(enable ? "Trigger Off" : "Trigger On");
    app_controller_.enableTrigger(!enable);
}

void MainWindow::on_ignorepowerstatus_chb_toggled(bool checked)
{
    MtlDataModel::power_status_ignore = checked;
}

void MainWindow::on_connect_btn_clicked()
{
    if (MTL32::isConnected())
    {
        mtl_controller_.disconnectStation();
        ui->connect_btn->setText("Connect");
    }
    else
    {
        mtl_controller_.connectStation(ui->station_le->text());
        if (MTL32::isConnected()) ui->connect_btn->setText("Disconnect");
    }
}
