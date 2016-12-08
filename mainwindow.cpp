#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    usbService(new UsbService())
{
    // initial main window
    ui->setupUi(this);
    setWindowTitle(APP_TITLE);
    setCentralWidget(ui->mdiArea);
    // set context menu to listWidget
    ui->devlistWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->devlistWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showDevListMenu(QPoint)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showWorkerListMenu(QPoint)));
    connect(ui->devlistWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openDeviceAction()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(closeDeviceAction()));
    // list usb device
    refreshDevice();
}

void MainWindow::refreshDevice()
{
    // query usb from service
    devices = usbService->listDevices();
    ui->statusTextbox->appendPlainText(QString::asprintf("Found %d USB Devices", devices->length()));
    // show in devlistWidget
    ui->devlistWidget->clear();
    for(int i=0, len=devices->length(); i < len; i++)
    {
        ui->devlistWidget->insertItem(i,
         QString("%1 (%2) [bus: %3] [address: %4]")
            .arg(devices->at(i).productName)
            .arg(devices->at(i).developer)
            .arg(devices->at(i).bus)
            .arg(devices->at(i).address));
    }
}

void MainWindow::showDevListMenu(const QPoint &pos)
{
    QPoint globalPos = ui->devlistWidget->mapToGlobal(pos);
    QPoint padding(6,3);
    globalPos += padding;

    // create menu
    QMenu menu;
    int i = ui->devlistWidget->currentRow();
    UsbDeviceDesc desc = devices->at(i);

    if (mapDeviceThread.contains(desc.hashName())) {
        menu.addAction("Close", this, SLOT(closeDeviceAction()));
    } else {
        menu.addAction("Open", this, SLOT(openDeviceAction()));
    }
    menu.exec(globalPos);
}

void MainWindow::showWorkerListMenu(const QPoint &pos)
{
    QPoint globalPos = ui->listWidget->mapToGlobal(pos);
    QPoint padding(6,3);
    globalPos += padding;
    // create menu
    QMenu menu;
    menu.addAction("Close", this, SLOT(closeDevice()));
    menu.exec(globalPos);
}

void MainWindow::openDeviceAction()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Open USB Device");
    msgBox.setText("Do you want to open this device?");
    msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    // deny all return except open returned id
    if (ret != QMessageBox::Open) return;

    // get index of selected device
    int i = ui->devlistWidget->currentRow();
    UsbDeviceDesc desc = devices->at(i);

    // show status message
    ui->statusTextbox->appendHtml(
                QString("<b style=\"color:#093\">Open %1 (%2) bus:%3 addr:%4</b>")
                .arg(desc.productName).arg(desc.developer)
                .arg(desc.bus).arg(desc.address));

    // check exists thread
    if (mapDeviceThread.contains(desc.hashName()))
    {
        ui->statusTextbox->appendHtml(QString("<span style=\"color:#f00\">device is already opened</span>"));
        return;
    }
    // create thread and start
    UsbThread *thread = new UsbThread();
    connect(thread, SIGNAL(printLogMessage(QString)), ui->statusTextbox, SLOT(appendPlainText(QString)));
    connect(thread, SIGNAL(deviceConnected(UsbDeviceDesc)), this, SLOT(addWorker(UsbDeviceDesc)));
    mapDeviceThread.insert(desc.hashName(), thread);
    thread->open(desc);
}

void MainWindow::closeDeviceAction()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Close working device");
    msgBox.setText("Do you want to close?");
    msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();

    // deny all return except open returned id
    if (ret != QMessageBox::Close) return;


    // get index of selected item
    QString name;
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item) {
        int index = ui->devlistWidget->currentRow();
        UsbDeviceDesc desc = devices->at(index);
        name = desc.hashName();
    } else {
        name = item->text();
    }
    ui->statusTextbox->appendHtml(
                QString("<b style=\"color:#f00\">Close %1</b>")
                .arg(name));
    removeWorker(name);
}

void MainWindow::addWorker(UsbDeviceDesc desc)
{
    ui->listWidget->addItem(desc.hashName());
}

void MainWindow::removeWorker(QString channelName) {
    int count = ui->listWidget->count();
    for(int i=0; i < count; i++)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        if ( item->text() == channelName )
        {
            delete ui->listWidget->takeItem(i);
            break;
        }

    }
    if (mapDeviceThread.contains(channelName))
    {
        UsbThread *thread = mapDeviceThread.value(channelName);
        delete thread;
        mapDeviceThread.remove(channelName);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    delete devices;
    delete usbService;
}
