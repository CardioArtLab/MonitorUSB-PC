#include "testwidget.h"
#include "ui_testwidget.h"

TestWidget::TestWidget(usb_firmware _firmware, int _channel, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestWidget)
{
    ui->setupUi(this);

    // initial variables
    static QTime _time(QTime::currentTime());
    time = _time;
    firmware = _firmware;
    channelIndex = _channel;

    // calculate data converting factor from firmware
    convertingFactor = double(firmware.ref_max - firmware.ref_min) / (firmware.resolution - 1);

    // graph setups
    ui->customPlot->addGraph();
    ui->customPlot->axisRect()->setupFullAxesBox();

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s:%z");
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->xAxis->setLabel("Time");
    ui->customPlot->yAxis->setLabel(QString("Voltage (%1)").arg(firmware.unit));
    ui->customPlot->yAxis->setRange(-5,5);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QCPScatterStyle dotScatter;
    dotScatter.setShape(QCPScatterStyle::ssCircle);
    dotScatter.setPen(QPen(Qt::red));
    dotScatter.setSize(5);
    ui->customPlot->addGraph();
    ui->customPlot->graph(1)->setScatterStyle(dotScatter);
    ui->customPlot->graph(1)->setLineStyle(QCPGraph::lsNone);

    ui->customPlot->replot();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect UI event
    connect(ui->stopButton, SIGNAL(pressed()), this, SLOT(onClickStopButton()));
    connect(ui->recordButton, SIGNAL(pressed()), this, SLOT(onClickSaveButton()));
    connect(ui->timeRangeBox, SIGNAL(valueChanged(int)), this, SLOT(onChangeTimeRange(int)));
    connect(ui->recordSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onChangeRecordTimeRange(int)));
    connect(ui->recordCheckBox, SIGNAL(toggled(bool)), this, SLOT(onToggleRecordMode(bool)));
    ui->timeRangeBox->setValue(secTimeRange);
    ui->recordSpinBox->setValue(secRecordTimeRange);

    // setup timer
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    timer->start(0);
}

void TestWidget::recieve(QVector<int32_t> packet)
{
    if (isStop) return;
    double key = time.elapsed()/1000.0; // in seconds
    if (key > lastPointKey) {
        int size = packet.size();
        double values[size];
        for (int i=0; i<size; i++) {
            values[i] = packet.at(i);
            // decode two complement
            if (values[i] >= firmware.resolution / 2) {
                values[i] -= firmware.resolution;
            }
            // convert from bit value to voltage value
            values[i] = (values[i] + (firmware.resolution / 2)) * convertingFactor + firmware.ref_min;
            values[i] = filter[i].calculate(key, values[i]);
        }
        double value = 0;
        if (firmware.id == FIRMWARE_CA_PULSE_OXIMETER) {
            if (channelIndex == CUSTOM_CHANNEL_SPO2_PERCENT) {
                int type = filter[1].getSaturationType();
                value = log(filter[1].getRatio(type)) / log(filter[0].getRatio(type));
                ui->customPlot->graph(1)->addData(key, value);
            }
            else if (channelIndex == CUSTOM_CHANNEL_SPO2_HR) {
                value = filter[1].getHearthRate();
                ui->customPlot->graph(1)->addData(key, value);
            }
        }
        // record to buffer
        if (!isRecord) return;
        DataPoint point = {key, value};
        recordData.append(point);
        lastPointKey = key;
    }
}

void TestWidget::realtimeDataSlot()
{
    if (isStop) return;

    double key = time.elapsed()/1000.0; // in seconds

    if (key-lastPointKey > 0.1) {
        // render graph every 100 ms
        ui->customPlot->graph(1)->rescaleValueAxis();
        ui->customPlot->xAxis->setRange(key, secTimeRange, Qt::AlignRight);
        ui->customPlot->replot();
        lastPointKey = key;
    }

    if (key-lastRenderTime > 0.33) {
        // remove older than 10 seconds data
        ui->customPlot->graph(1)->data()->removeBefore(key-secTimeRange);
        lastRemoveTime = key;
    }

    if (isRecord && !isRecordFull && !recordData.isEmpty()) {
        if (key - recordData.front().time > secRecordTimeRange) {
            // set button to green if record data is full
            ui->recordButton->setStyleSheet("background-color: green");
            isRecordFull = true;
        }
        while (key - recordData.front().time > secRecordTimeRange) {
            recordData.removeFirst();
        }
    }
}

void TestWidget::onClickStopButton()
{
    isStop = !isStop;
    if (isStop) {
        ui->stopButton->setText(tr("Start"));
    } else {
        ui->stopButton->setText(tr("Stop"));
    }
}

void TestWidget::onClickSaveButton()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save data file"), "", tr("CSV (*.csv);;All files (*)"));
    if (filename.isEmpty()) {
        return;
    } else {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        } else {
            QTextStream stream(&file);
            // write header
            stream << "INDEX" << ',' << "TIME (SEC)" << ',' << "VALUE (" << firmware.unit << ")" << endl;
            // write content from data buffer
            int size = recordData.size();
            QLinkedList<DataPoint>::iterator i = recordData.begin();
            for (int index=1;size > 0; size--, i++, index++) {
                stream << index << ',' << (*i).time << ',' << (*i).value << endl;
            }
            file.close();
        }
    }
}

void TestWidget::onChangeTimeRange(int value)
{
    secTimeRange = value;
}

void TestWidget::onChangeRecordTimeRange(int value)
{
    secRecordTimeRange = value;
}

void TestWidget::onToggleRecordMode(bool isEnabled)
{
    isRecord = isEnabled;
    ui->recordSpinBox->setEnabled(isEnabled);
    ui->recordButton->setEnabled(isEnabled);
}

TestWidget::~TestWidget()
{

    timer->stop();
    delete timer;
    delete ui;
}
