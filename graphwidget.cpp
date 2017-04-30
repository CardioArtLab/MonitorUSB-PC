#include "graphwidget.h"
#include "ui_graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWidget)
{
    ui->setupUi(this);

    // graph setups
    ui->customPlot->addGraph();
    ui->customPlot->axisRect()->setupFullAxesBox();

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%m:%s:%z");
    ui->customPlot->xAxis->setTicker(timeTicker);

    ui->customPlot->xAxis->setLabel("Time");
    ui->customPlot->yAxis->setLabel("Voltage");
    ui->customPlot->yAxis->setRange(-5,5);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->customPlot->replot();

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    timer->start(0);
}

void GraphWidget::realtimeDataSlot()
{
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) {
        ui->customPlot->graph(0)->addData(key, 5.0 * qSin(key) + qrand() / (double)RAND_MAX);
        ui->customPlot->graph(0)->rescaleValueAxis();
        lastPointKey = key;
    }

    ui->customPlot->xAxis->setRange(key, 10, Qt::AlignRight);
    ui->customPlot->replot();
}

GraphWidget::~GraphWidget()
{
    timer->stop();
    delete timer;
    delete ui;
}
