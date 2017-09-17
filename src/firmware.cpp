#include "firmware.h"

usb_firmware getUsbFirmware(QString product, QString manufacturer)
{
    usb_firmware mnt;
    mnt.id = FIRMWARE_ERROR;

    if (product.compare(FIRMWARE_CA_ECG_MONITOR__PD, Qt::CaseInsensitive) == 0
            && manufacturer.compare(FIRMWARE_CA_ECG_MONITOR__MF, Qt::CaseInsensitive) == 0)
    {
        mnt.id = FIRMWARE_CA_ECG_MONITOR;
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
        mnt.id = FIRMWARE_CA_PULSE_OXIMETER;
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
        mnt.id = FIRMWARE_CA_TEST;
        mnt.ref_min = 0;
        mnt.ref_max = 100;
        mnt.resolution = 100;
        mnt.num_channel = 2;
        mnt.name = QString("general");
        mnt.unit = QString("celcius");
        mnt.descriptor.append("id");
        mnt.descriptor.append("temperature");
    }
    return mnt;
}

