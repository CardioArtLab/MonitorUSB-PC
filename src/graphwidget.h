#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QMutex>
#include <QLinkedList>
#include <QTime>
#include "firmware.h"
#include "filter/pulseoximterfilter.h"
#include "filter/spo2filter.h"

namespace Ui {
class GraphWidget;
}

struct DataPoint
{
    double time;
    double value;
};

class GraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GraphWidget(usb_firmware firmware, int channel = 0, QWidget *parent = 0);
    ~GraphWidget();

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
    Ui::GraphWidget *ui;
    BaseFilter *filter;
};

#endif // GRAPHWIDGET_H
