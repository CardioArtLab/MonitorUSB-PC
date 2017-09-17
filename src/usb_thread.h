#ifndef USB_THREAD_H
#define USB_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
#include <libusb.h>
}

#include "usb_service.h"
#include "firmware.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define TRY(msg, ret_err) \
    do { if (ret_err < 0) { \
    qDebug("error: %s (%s)\n", msg, libusb_error_name(ret_err));}} while(0)
//------------------------------------------------------------------------------

Q_DECLARE_METATYPE(QVector<int32_t>)

class UsbThread : public QThread
{
    Q_OBJECT

public:
    UsbThread(QObject *parent = NULL);
    ~UsbThread();
    void callback_transfer(struct libusb_transfer *xfr);
    QString getChannelName();

public slots:
    void open(const UsbDeviceDesc desc);
    void open(const UsbDeviceDesc desc, int extraIndex);
    void close();

signals:
    void deviceConnected(UsbDeviceDesc);
    void deviceConnectedWithExtraId(UsbDeviceDesc, int);
    void send(QVector<int32_t>);
    void printLogMessage(QString);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    void exit_nanomsg();
    bool openHandle(const UsbDeviceDesc desc);
    int initTransfer(uint8_t endpoint);

    int FIRMWARE_ID;
    int EP_SIZE;
    int extraIndex;
    bool isStop;
    bool isCloseTransfer;
    long missingCount = 0;
    uint8_t buffer[1024];
    QMutex mutex;
    QMutex lock;
    QString channelName;
    QWaitCondition closeTransfer;
    UsbDeviceDesc usbDesc;
    usb_firmware mnt;
    libusb_context *context = NULL;
    libusb_device_handle *dev_handle = NULL;
};

void LIBUSB_CALL callback_wrapper(struct libusb_transfer *xfr);

#endif // USB_THREAD_H
