#ifndef USBDEVICEMODEL_H
#define USBDEVICEMODEL_H

#include <QVector>
#include <QString>
#include <QObject>
extern "C" {
#include "libusb.h"
}

class UsbDeviceDesc
{
public:
    UsbDeviceDesc();
    UsbDeviceDesc(qint32 vid, qint32 pid, qint8 bus, qint8 address, QString productName, QString developer);
    qint32 vid;
    qint32 pid;
    qint8 bus;
    qint8 address;
    QString productName;
    QString developer;
    QString hashName();
};
Q_DECLARE_METATYPE(UsbDeviceDesc);

class UsbService : public QObject
{
    Q_OBJECT
    libusb_context *context;
public:
    explicit UsbService();
    ~UsbService();
    QVector<UsbDeviceDesc> *listDevices();

signals:
    void printLogMessage(QString message, int time=0);
    void listDevicesFinish();

};

#endif // USBDEVICEMODEL_H
