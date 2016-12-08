#include "usb_thread.h"

UsbThread::UsbThread(QObject *parent) : QThread(parent)
{
    this->isStop = false;
    this->isCloseTransfer = false;
}

UsbThread::~UsbThread()
{
    qDebug("start close()");
    close();
    qDebug("start wait()");
    wait();
    qDebug("~UsbThread()");
}

void UsbThread::open(const UsbDeviceDesc desc)
{
    this->usbDesc = desc;
    start(HighPriority);
}

void UsbThread::_openHandle(const UsbDeviceDesc desc)
{
    libusb_device **list, *dev;
    int i=0;
    bool hasFound = false;

    // open device
    int err = libusb_get_device_list(this->context, &list);
    if (err < 0)
    {
        emit printLogMessage(QString::asprintf("libusb_get_device_list error: %s", libusb_error_name(err)));
        goto open_exit;
    }

    // loop to scan device
    for(i=0; i < err; i++)
    {
        dev = list[i];
        uint8_t bus = libusb_get_bus_number(dev);
        uint8_t address = libusb_get_device_address(dev);
        if (bus == desc.bus && address == desc.address)
        {
            int fid = getFirmwareId(desc.productName, desc.developer);
            if (fid != FIRMWARE_ERROR)
            {
                hasFound = true;
                FIRMWARE_ID = fid;
                EP_SIZE = libusb_get_max_packet_size(dev, EP_ADDRESS);
                qDebug("FIRMWARE %d (%d bytes)\n", FIRMWARE_ID, EP_SIZE);

                err = libusb_open(dev, &this->dev_handle);
                if (err < 0)
                {
                    emit printLogMessage(QString::asprintf("libusb_get_device_list error: %s", libusb_error_name(err)));
                    goto open_exit;
                }

                break;
            }
        }
    }

    if (hasFound)
    {
        emit deviceConnected(this->usbDesc);
        emit printLogMessage(QString("channel %1 created").arg(channelName));
    } else {
        emit printLogMessage(QString("Not found device"));
    }
open_exit:
    libusb_free_device_list(list, TRUE);
}

void UsbThread::close()
{
    lock.lock();
    isStop = true;
    lock.unlock();
}

void UsbThread::run()
{
    int ret_err;
    TIMEVAL timeout = {1, 0};

    TRY("init usb context", libusb_init(&this->context));

    libusb_set_debug(this->context, LIBUSB_LOG_LEVEL_DEBUG);
    this->_openHandle(this->usbDesc);

    libusb_reset_device(this->dev_handle);
    libusb_release_interface(this->dev_handle, 0);


    TRY("claim interface", libusb_claim_interface(this->dev_handle, 0));
    TRY("init transfer", init_transfer(EP_ADDRESS));

    forever {
        bool stop = false;
        lock.lock();
        stop = this->isStop;
        lock.unlock();

        if (stop) break;
        libusb_handle_events_timeout(this->context, &timeout);
    }
    qDebug("read isClose");
    lock.lock();
    bool isClose = this->isCloseTransfer;
    lock.unlock();

    if (!isClose) {
        qDebug("wait for closeTransfer");
        closeTransfer.wait(&mutex);
    }

    TRY("release interface", libusb_release_interface(this->dev_handle, 0));
    libusb_close(this->dev_handle);
    libusb_exit(this->context);

    qDebug("end run()\n");
}

int UsbThread::getFirmwareId(QString product, QString manufacturer)
{
    // default fid indicating no firmware
    int fid = FIRMWARE_ERROR;

    if (product.compare(FIRMWARE_CA_ECG_MONITOR__PD, Qt::CaseInsensitive)
            && manufacturer.compare(FIRMWARE_CA_ECG_MONITOR__MF, Qt::CaseInsensitive))
    {
        fid = FIRMWARE_CA_ECG_MONITOR;
        mnt.ref_min = 0;
        mnt.ref_max = 3.3;
        mnt.resolution = 16777216;
        mnt.num_channel = 12;
        mnt.name = QString("ecg");
        mnt.unit = QString("mV");
        mnt.descriptor.append("Lead_I");
        mnt.descriptor.append("Lead_II");
        mnt.descriptor.append("Lead_III");
        mnt.descriptor.append("aVR");
        mnt.descriptor.append("aVL");
        mnt.descriptor.append("aVF");
        mnt.descriptor.append("V1");
        mnt.descriptor.append("V2");
        mnt.descriptor.append("V3");
        mnt.descriptor.append("V4");
        mnt.descriptor.append("V5");
        mnt.descriptor.append("V6");
    }
    else if (product.compare(FIRMWARE_CA_PULSE_OXIMETER__PD, Qt::CaseInsensitive)
             && manufacturer.compare(FIRMWARE_CA_PULSE_OXIMETER__MF, Qt::CaseInsensitive))
    {
        fid = FIRMWARE_CA_PULSE_OXIMETER;
        mnt.ref_min = 0;
        mnt.ref_max = 3.3;
        mnt.resolution = 4194304;
        mnt.num_channel = 2;
        mnt.sampling_rate = 1000;
        mnt.name = QString("oxigen_sat");
        mnt.unit = QString("mV");
        mnt.descriptor.append("LED1");
        mnt.descriptor.append("LED2");
    }
    else if (product.compare(FIRMWARE_CA_TEST__PD, Qt::CaseInsensitive)
             && manufacturer.compare(FIRMWARE_CA_TEST__MF, Qt::CaseInsensitive))
    {
        fid = FIRMWARE_CA_TEST;
        mnt.ref_min = 0;
        mnt.ref_max = 100;
        mnt.resolution = 100;
        mnt.num_channel = 2;
        mnt.name = QString("general");
        mnt.unit = QString("celcius");
        mnt.descriptor.append("id");
        mnt.descriptor.append("temperature");
    }
    return fid;
}

int UsbThread::init_transfer(uint8_t endpoint)
{
    int ret_err = LIBUSB_SUCCESS;
    struct libusb_transfer *xfr;

    // allocate transfer structure
    xfr = libusb_alloc_transfer(1);
    if (!xfr) {
        return LIBUSB_ERROR_NO_MEM;
    }
    // clear buffer
    memset(this->buffer, 0, sizeof(this->buffer));
    // set transfer option
    libusb_fill_interrupt_transfer(
        xfr, // transfer
        this->dev_handle, //handle
        endpoint, //target endpoint
        this->buffer, // buffer
        this->EP_SIZE, // size of buffer
        callback_wrapper, // pointer callback function
        this, // pass USBThread in form of user data
        0); // unlimit timeout
    // submit transfer
    TRY("submit transfer", libusb_submit_transfer(xfr));
    return ret_err;
}

void UsbThread::callback_transfer(struct libusb_transfer *xfr)
{
    int ret_err;
    bool stop = false;
    lock.lock();
    stop = this->isStop;
    lock.unlock();

    if (stop || xfr->status != LIBUSB_TRANSFER_COMPLETED)
    {
        libusb_free_transfer(xfr);
        lock.lock();
        this->isCloseTransfer = true;
        lock.unlock();
        closeTransfer.wakeAll();
        qDebug("clear transfer complete!");
        return;
    }

    TRY("submit transfer", libusb_submit_transfer(xfr));
}

void LIBUSB_CALL callback_wrapper(struct libusb_transfer *xfr)
{
    UsbThread *connector = reinterpret_cast<UsbThread*>(xfr->user_data);
    connector->callback_transfer(xfr);
}
