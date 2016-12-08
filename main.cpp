#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<UsbDeviceDesc>("UsbDeviceDesc");
    QApplication a(argc, argv);
    a.setStyle("fusion");
    MainWindow w;
    w.showMaximized();

    return a.exec();
}
