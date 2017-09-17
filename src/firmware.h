#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QVector>
#include <QObject>
//------------------------------------------------------------------------------
// Defines
//------------------------------------------------------------------------------
#ifndef ID_VENDOR
#define ID_VENDOR        0x10C4
#endif
#ifndef ID_PRODUCT
#define ID_PRODUCT        0x8A40
#endif
#ifndef EP_ADDRESS
#define EP_ADDRESS        0x83
#endif
//------------------------------------------------------------------------------
// Firmware
//------------------------------------------------------------------------------
#define FIRMWARE_ERROR             -1

#define FIRMWARE_CA_TEST           0
#define FIRMWARE_CA_TEST__MF       "Silicon Laboratories Inc."
#define FIRMWARE_CA_TEST__MF_LEN   sizeof(FIRMWARE_CA_TEST__MF)
#define FIRMWARE_CA_TEST__PD       "Fake Streaming 64byt"
#define FIRMWARE_CA_TEST__PD_LEN   sizeof(FIRMWARE_CA_TEST__PD)

#define FIRMWARE_CA_PULSE_OXIMETER         1
#define FIRMWARE_CA_PULSE_OXIMETER__MF     "CardioArt Laboratory"
#define FIRMWARE_CA_PULSE_OXIMETER__MF_LEN sizeof(FIRMWARE_CA_PULSE_OXIMETER__MF) -1
#define FIRMWARE_CA_PULSE_OXIMETER__PD     "Pulse Oximeter"
#define FIRMWARE_CA_PULSE_OXIMETER__PD_LEN sizeof(FIRMWARE_CA_PULSE_OXIMETER__PD) -1

#define FIRMWARE_CA_ECG_MONITOR            2
#define FIRMWARE_CA_ECG_MONITOR__MF        "CardioArt Laboratory"
#define FIRMWARE_CA_ECG_MONITOR__MF_LEN    sizeof(FIRMWARE_CA_ECG_MONITOR__MF)-1
#define FIRMWARE_CA_ECG_MONITOR__PD        "ECG Monitor"
#define FIRMWARE_CA_ECG_MONITOR__PD_LEN    sizeof(FIRMWARE_CA_ECG_MONITOR__PD)-1
//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
// macro to handle ERROR

//------------------------------------------------------------------------------
// Type declear
//------------------------------------------------------------------------------
typedef struct _firmware_t {
    int id;
    QString name;
    QString unit;
    int64_t resolution;
    double ref_min;
    double ref_max;
    int64_t sampling_rate;
    QVector<QString> descriptor;
    int num_channel;
    int active;
} usb_firmware;
Q_DECLARE_METATYPE(usb_firmware)

usb_firmware getUsbFirmware(QString product, QString manufacturer);
#endif // FIRMWARE_H
