#include <QApplication>
#include "controller.h"
#include "mainwindow.h"
#include "MTLtask.h"

int main(int argc, char *argv[])
{
    bool emulation = false;
    if (argc == 2 && QString(argv[1]) == "-e")
        emulation = true;
    QApplication a(argc, argv);
    MtlController mtl_controller(emulation);
    Controller controller(mtl_controller);
    MainWindow w(controller, mtl_controller);
    controller.setMainWindow(&w);
    w.init();
    w.show();

    return a.exec();
}
