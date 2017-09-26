#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include <QWidget>
#include <QMutex>
#include <QLinkedList>
#include <QTime>
#include "firmware.h"
#include "filter/spo2filter.h"

namespace Ui {
class TestWidget;
}

class TestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TestWidget(usb_firmware _firmware, int _channel, QWidget *parent);
    ~TestWidget();
public slots:
    void onClickStopButton();
    void onClickSaveButton();
    void onChangeTimeRange(int value);
    void onChangeRecordTimeRange(int value);
    void onToggleRecordMode(bool enable);
    void recieve(QVector<int32_t>);
    void realtimeDataSlot();

protected:
    bool isStop = false;
    bool isRecord = false;
    bool isRecordFull = false;
    int channelIndex;
    double lastPointKey = 0;
    double lastRenderTime = 0;
    double lastRemoveTime = 0;
    double convertingFactor = 1;
    uint32_t secTimeRange = 10;
    uint32_t secRecordTimeRange = 10;

    usb_firmware firmware;
    QLinkedList<DataPoint> recordData;

    QMutex mutex;
    QTime time;
    QTimer *timer;
    SpO2Filter filter[2];
private:
    Ui::TestWidget *ui;
    double Tmin = 0, Tmax = 0, Vmin = 0, Vmax = 0;
};

#endif // TESTWIDGET_H
