#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStringBuilder>
#include <QMessageBox>
#include <QTreeWidget>
#include <QDebug>
#include "version.h"
#include "firmware.h"
#include "usb_service.h"
#include "usb_thread.h"

#include "graphwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void addTreeRoot(QString name, QString description);
    void addTreeChild(QTreeWidgetItem *parent,
                      QString name, QString description,
                      QVariant userData, QVariant index);

public slots:
    void refreshDevice();
    void openDeviceAction();
    void closeDeviceAction(); 
    void showDevTreeMenu(const QPoint &pos);
    void clickDeviceAction(QTreeWidgetItem*);
    void addSubDeviceTree(UsbDeviceDesc description, int extraId);

private:
    Ui::MainWindow  *ui;
    UsbService      *usbService;
    QVector<UsbDeviceDesc>      *devices = nullptr;
    QMap<QString, UsbThread*>   mapDeviceThread;
    // Methods

};

#endif // MAINWINDOW_H
