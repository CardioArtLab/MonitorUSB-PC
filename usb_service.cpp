#include "usb_service.h"

UsbService::UsbService()
{
    libusb_init(&context);
}

UsbService::~UsbService()
{
    libusb_exit(context);
}

QVector<UsbDeviceDesc> *UsbService::listDevices()
{
    libusb_device **devs, *dev;
    int i=0, count, item_i=0;

    count = libusb_get_device_list(context, &devs);
    if (count < 0)
    {
        emit printLogMessage(QString("libusb failed to initialize"));
        return nullptr;
    }

    QVector<UsbDeviceDesc> *usbVector = new QVector<UsbDeviceDesc>();
    while( (dev = devs[i++]) != NULL)
    {
        struct libusb_device_descriptor desc;
        libusb_device_handle * handle = NULL;
        // get usb description
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            emit printLogMessage(QString("libusb can not get device descriptor"));
            continue;
        }


        // open usb handle
        r = libusb_open(dev, &handle);
        if (r < 0) {continue;}

        // create struct
        UsbDeviceDesc usbDesc;

        // get device name
        unsigned char *dev_name;
        int length;
        dev_name = (unsigned char*) malloc(255);

        // get product name
        length = libusb_get_string_descriptor_ascii(handle, desc.iProduct, dev_name, 255);
        usbDesc.productName = QString::fromLocal8Bit((const char*)dev_name, length);
        // get company name
        length = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, dev_name, 255);
        usbDesc.developer = QString::fromLocal8Bit((const char*)dev_name, length);
        // clear resource
        free(dev_name);
        libusb_close(handle);
        // get VID and PID
        usbDesc.vid = desc.idVendor;
        usbDesc.pid = desc.idProduct;
        // get bus and address
        usbDesc.bus = libusb_get_bus_number(dev);
        usbDesc.address = libusb_get_device_address(dev);
        // append to vector and increase the counter
        usbVector->append(usbDesc);
        item_i++;
    }
    libusb_free_device_list(devs, TRUE);
    emit listDevicesFinish();
    return usbVector;
}

UsbDeviceDesc::UsbDeviceDesc() {}

QString UsbDeviceDesc::hashName()
{
    QString name = productName;
    name.replace(" ", "_");
    return QString("%1_%2%3").arg(name.toUpper()).arg(bus).arg(address);
}
