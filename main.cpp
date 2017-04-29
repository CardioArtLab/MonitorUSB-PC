#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    qRegisterMetaType<UsbDeviceDesc>("UsbDeviceDesc");

    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("MonitorUSB-PC");
    QCoreApplication::setOrganizationName("CardioArt Laboratory");
    QCoreApplication::setApplicationVersion(APP_VERSION);

    // (optional) open program from command line
    QCommandLineParser parser;
    parser.setApplicationDescription("Tool for read monitoring USB device");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("device", "a device to open");
    parser.process(a);

    a.setStyle("fusion");
    MainWindow w;
    // TODO: support command line
    // see: http://doc.qt.io/qt-5/qtwidgets-mainwindows-mdi-main-cpp.html
    w.showMaximized();

    return a.exec();
}
