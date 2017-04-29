#include "usb_thread.h"
/*!
 * \class UsbThread::UsbThread
 * \brief A controller a thread performing IO on USB device with libusb
 * please use open(const UsbDeviceDesc desc) for initialize thread.
 * \param parent
 */
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

/*!
 * \fn void open(const UsbDeviceDesc desc)
 * \brief open USB device
 * \param desc a description class for USB
 */
void UsbThread::open(const UsbDeviceDesc desc)
{
    this->usbDesc = desc;
    start(HighPriority);
}

void UsbThread::open(const UsbDeviceDesc desc, int extraIndex)
{
    this->extraIndex = extraIndex;
    open(desc);
}

void UsbThread::openHandle(const UsbDeviceDesc desc)
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
                err = libusb_open(dev, &this->dev_handle);
                if (err < 0)
                {
                    emit printLogMessage(QString::asprintf("libusb_get_device_list error: %s", libusb_error_name(err)));
                    goto open_exit;
                }

                hasFound = true;
                FIRMWARE_ID = fid;
                EP_SIZE = libusb_get_max_packet_size(dev, EP_ADDRESS);
                channelName = usbDesc.hashName();
                qDebug("FIRMWARE %d (%d bytes)\n", FIRMWARE_ID, EP_SIZE);
                break;
            }
        }
    }

    if (hasFound)
    {
        emit deviceConnected(this->usbDesc);
        emit deviceConnectedWithExtraId(this->usbDesc, this->extraIndex);
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
    TIMEVAL timeout = {1, 0};

    TRY("init usb context", libusb_init(&this->context));

    libusb_set_debug(this->context, LIBUSB_LOG_LEVEL_DEBUG);
    this->openHandle(this->usbDesc);

    libusb_reset_device(this->dev_handle);
    libusb_release_interface(this->dev_handle, 0);


    TRY("claim interface", libusb_claim_interface(this->dev_handle, 0));
    TRY("init transfer", initTransfer(EP_ADDRESS));
    initNanomsg();
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
    exit_nanomsg();

    qDebug("end run()\ntotal missing: %ld", missingCount);
}

int UsbThread::getFirmwareId(QString product, QString manufacturer)
{
    // default fid indicating no firmware
    int fid = FIRMWARE_ERROR;

    if (product.compare(FIRMWARE_CA_ECG_MONITOR__PD, Qt::CaseInsensitive) == 0
            && manufacturer.compare(FIRMWARE_CA_ECG_MONITOR__MF, Qt::CaseInsensitive) == 0)
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
    else if (product.compare(FIRMWARE_CA_PULSE_OXIMETER__PD, Qt::CaseInsensitive) == 0
             && manufacturer.compare(FIRMWARE_CA_PULSE_OXIMETER__MF, Qt::CaseInsensitive) == 0)
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
    else if (product.compare(FIRMWARE_CA_TEST__PD, Qt::CaseInsensitive) == 0
             && manufacturer.compare(FIRMWARE_CA_TEST__MF, Qt::CaseInsensitive) == 0)
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

int UsbThread::initTransfer(uint8_t endpoint)
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

int UsbThread::initNanomsg()
{
    nanoSock = nn_socket(AF_SP, NN_PUB);
    Q_ASSERT( nanoSock >= 0 );
    QString url = QString("ipc:///tmp/%1.ipc").arg(channelName);
    char *name = url.toLocal8Bit().data();
    nanoEndpoint = nn_bind(nanoSock, name);
    Q_ASSERT( nanoEndpoint >= 0 );

    return 0;
}

void UsbThread::exit_nanomsg()
{
    Q_ASSERT( nn_shutdown(nanoSock, nanoEndpoint) >= 0 );
    Q_ASSERT( nn_close(nanoSock) >= 0 );
}

void UsbThread::callback_transfer(struct libusb_transfer *xfr)
{
    // stop condition
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

        return;
    }
    // end stop condition

    // read USB
    uint8_t *packet = xfr->buffer;
    int bytes;
    switch(FIRMWARE_ID)
    {
    case FIRMWARE_CA_TEST:
        Q_ASSERT( xfr->actual_length >= 2 );
        bytes = nn_send(nanoSock, packet, 2, NN_DONTWAIT);
        if (bytes < 0 && nn_errno() == EAGAIN) missingCount++;

        break;
    case FIRMWARE_CA_PULSE_OXIMETER:

        Q_ASSERT( xfr->actual_length >= 6);
        {
            int64_t *data = (int64_t*)nn_allocmsg(sizeof(uint64_t)*2, 0);
            data[0] = (packet[0] << 16) & 0xFF0000;
            data[0] += (packet[1] << 8) & 0x00FF00;
            data[0] += (packet[2] & 0x0000FF);
            data[1] = (packet[3] << 16) & 0xFF0000;
            data[1] += (packet[4] << 8) & 0x00FF00;
            data[1] += (packet[5] & 0x0000FF);
            bytes = nn_send(nanoSock, data, NN_MSG, NN_DONTWAIT);
            if (bytes < 0 && nn_errno() == EAGAIN) missingCount++;
        }
        break;
    case FIRMWARE_CA_ECG_MONITOR:
        Q_ASSERT( xfr->actual_length >= 27 );
        {
            int64_t *data = (int64_t*)nn_allocmsg(sizeof(uint64_t)*12, 0);
            /**
              *  9 Block of 3 bytes diagram
              * Block /Byte 1 2 3
              *     1:      X X X
              *     2:      V6
              *     3:      Lead-I
              *     4:      Lead-II
              *     5:      V2
              *     6:      V3
              *     7:      V4
              *     8:      V5
              *     9:      V6
              *
              *  Calculations
              * aVR = -(Lead II + Lead I) /2
              * aVL = LeadI - LeadII/2
              * aVF = LeadII - LeadI/2
              */

            // Lead I
            data[0] = (packet[6] << 16) & 0xFF0000;
            data[0] += (packet[7] << 8) & 0x00FF00;
            data[0] += (packet[8] & 0x0000FF);
            // Lead II
            data[1] = (packet[9] << 16) & 0xFF0000;
            data[1] += (packet[10] << 8) & 0x00FF00;
            data[1] += (packet[11] & 0x0000FF);
            // V1
            data[6] = (packet[24] << 16) & 0xFF0000;
            data[6] += (packet[25] << 8) & 0x00FF00;
            data[6] += (packet[26] & 0x0000FF);
            // V2
            data[7] = (packet[12] << 16) & 0xFF0000;
            data[7] += (packet[13] << 8) & 0x00FF00;
            data[7] += (packet[14] & 0x0000FF);
            // V3
            data[8] = (packet[15] << 16) & 0xFF0000;
            data[8] += (packet[16] << 8) & 0x00FF00;
            data[8] += (packet[17] & 0x0000FF);
            // V4
            data[9] = (packet[18] << 16) & 0xFF0000;
            data[9] += (packet[19] << 8) & 0x00FF00;
            data[9] += (packet[20] & 0x0000FF);
            // V5
            data[10] = (packet[21] << 16) & 0xFF0000;
            data[10] += (packet[22] << 8) & 0x00FF00;
            data[10] += (packet[23] & 0x0000FF);
            // V6
            data[11] = (packet[3] << 16) & 0xFF0000;
            data[11] += (packet[4] << 8) & 0x00FF00;
            data[11] += (packet[5] & 0x0000FF);
            // Lead III
            data[2] = data[1] - data[0];
            // aVR
            data[3] = -(data[1]+data[0])/2;
            // aVL
            data[4] = data[0] - data[1]/2;
            // aVF
            data[5] = data[1] - data[0]/2;
            //BUG: HERE
            bytes = nn_send(nanoSock, &data, NN_MSG, NN_DONTWAIT);
            if (bytes < 0 || nn_errno() == EAGAIN) missingCount++;
       }
       break;
    default:
       break;
    }
    // end read USB

    Q_ASSERT( libusb_submit_transfer(xfr) >= 0);
}

void LIBUSB_CALL callback_wrapper(struct libusb_transfer *xfr)
{
    UsbThread *connector = reinterpret_cast<UsbThread*>(xfr->user_data);
    connector->callback_transfer(xfr);
}
