#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStringBuilder>
#include <QMessageBox>
#include "app_var.h"
#include "usb_service.h"
#include "usb_thread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void refreshDevice();
    void showDevListMenu(const QPoint &pos);
    void showWorkerListMenu(const QPoint &pos);
    void openDeviceAction();
    void closeDeviceAction();
    void addWorker(UsbDeviceDesc);

private:
    // UIs
    Ui::MainWindow *ui;
    // Services
    UsbService *usbService;
    // Variables
    QVector<UsbDeviceDesc> *devices = nullptr;
    QMap<QString, UsbThread*> mapDeviceThread;
    // Methods
    void initUsbModel(QStandardItemModel *model);
    void removeWorker(QString name);
};

#endif // MAINWINDOW_H
