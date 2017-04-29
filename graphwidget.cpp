#include "graphwidget.h"
#include "ui_graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWidget)
{
    ui->setupUi(this);
}

GraphWidget::~GraphWidget()
{
    delete ui;
}
