#ifndef USB_THREAD_H
#define USB_THREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

extern "C" {
#include "libusb.h"
#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"
}

#include "usb_service.h"
#include "firmware.h"

//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#define TRY(msg, ret_err) \
    do { if (ret_err < 0) { \
    qDebug("error: %s (%s)\n", msg, libusb_error_name(ret_err));}} while(0)
class UsbThread : public QThread
{
    Q_OBJECT
public:
    UsbThread(QObject *parent = NULL);
    ~UsbThread();
    void open(const UsbDeviceDesc desc);
    QString getChannelName();
    void callback_transfer(struct libusb_transfer *xfr);

public slots:
    void close();

signals:
    void deviceConnected(UsbDeviceDesc);
    void printLogMessage(QString);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    // thread variables
    QMutex mutex;
    QMutex lock;
    QWaitCondition closeTransfer;
    bool isStop;
    bool isCloseTransfer;
    // libusb variables
    libusb_context *context = NULL;
    libusb_device_handle *dev_handle = NULL;
    // nanomsg variables
    QString channelName;
    char *cChannelName;
    int nanoSock;
    int nanoEndpoint;
    // local variables
    int FIRMWARE_ID;
    int EP_SIZE;
    measurement_t mnt;
    UsbDeviceDesc usbDesc;
    uint8_t buffer[1024];
    long missingCount = 0;

    // functions
    int getFirmwareId(QString strProduct, QString strManufacturer);
    int init_transfer(uint8_t endpoint);
    int init_nanomsg();
    void exit_nanomsg();
    void _openHandle(const UsbDeviceDesc desc);

};
void LIBUSB_CALL callback_wrapper(struct libusb_transfer *xfr);
#endif // USB_THREAD_H
