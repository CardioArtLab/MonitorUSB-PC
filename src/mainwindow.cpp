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

    // customize device panel
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels(QStringList() << "Device" << "Description");
    ui->treeWidget->setWordWrap(true);
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showDevTreeMenu(QPoint)));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(clickDeviceAction(QTreeWidgetItem*)));

    // register type use in inter-thread communication
    qRegisterMetaType<QVector<int32_t>>();

    // refresh device
    refreshDevice();
}

/**
 * @brief MainWindow::refreshDevice
 * @details List all USB devices
 */
void MainWindow::refreshDevice()
{
    // clear working usbthread
    QMapIterator<QString, UsbThread*> i(mapDeviceThread);
    while (i.hasNext()) {
        i.next();
        delete i.value();
    }
    mapDeviceThread.clear();
    mapCustomChannelDescription.clear();
    mapCustomChannelType.clear();
    // query usb from service
    devices = usbService->listDevices();
    ui->statusTextbox->appendPlainText(QString::asprintf("Found %d USB Devices", devices->length()));

    // reset tree
    ui->treeWidget->clear();

    // loop to add device to tree
    for(int i=0, len=devices->length(); i < len; i++)
    {
        QString name = QString("%1 (%2)").arg(devices->at(i).productName).arg(devices->at(i).developer);
        QString description = QString("[bus: %3] [address: %4]").arg(devices->at(i).bus).arg(devices->at(i).address);
        addTreeRoot(name, description);
    }
    ui->treeWidget->resizeColumnToContents(0);
}

void MainWindow::addTreeRoot(QString name, QString description)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, name);
    treeItem->setText(1, description);
}

void MainWindow::addTreeChild(QTreeWidgetItem *parent, QString name, QString description, QVariant userData, QVariant index)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();
    treeItem->setText(0, name);
    treeItem->setText(1, description);
    treeItem->setData(0, Qt::UserRole, userData);
    treeItem->setData(1, Qt::UserRole, index);
    parent->addChild(treeItem);
}

/**
 * @brief MainWindow::showDevTreeMenu
 * @details Show Right click menu on Device Panel
 * @param pos
 */
void MainWindow::showDevTreeMenu(const QPoint &pos)
{
    QPoint globalPos = ui->treeWidget->mapToGlobal(pos);
    QPoint padding(6,24);
    globalPos += padding;

    // create menu
    QMenu menu;
    if (ui->treeWidget->currentIndex().isValid()) {
        QTreeWidgetItem *item = ui->treeWidget->currentItem();
        if(!ui->treeWidget->currentItem()->parent()) {
            int i = ui->treeWidget->currentIndex().row();
            if (i < devices->size()) {
                UsbDeviceDesc desc = devices->at(i);
                if (mapDeviceThread.contains(desc.hashName())) {
                    menu.addAction("Close", this, SLOT(closeDeviceAction()));
                } else {
                    QString label = "Open " + desc.productName;
                    menu.addAction(label, this, SLOT(openDeviceAction()));
                }
            }
        } else {
            // Custom channel
            // extract user data to usb device description
            UsbDeviceDesc description = item->data(0, Qt::UserRole).value<UsbDeviceDesc>();
            // get firmware from device description
            usb_firmware firmware = getUsbFirmware(description.productName, description.developer);
            if (firmware.id == FIRMWARE_CA_PULSE_OXIMETER) {
                // add key map based on name
                QString spo2HashName = QString("%1_%2").arg(description.hashName()).arg(CUSTOM_CHANNEL_SPO2_PERCENT);
                QString hrHashName = QString("%1_%2").arg(description.hashName()).arg(CUSTOM_CHANNEL_SPO2_HR);
                mapCustomChannelDescription.insert(spo2HashName, description);
                mapCustomChannelDescription.insert(hrHashName, description);
                mapCustomChannelType.insert(spo2HashName, CUSTOM_CHANNEL_SPO2_PERCENT);
                mapCustomChannelType.insert(hrHashName, CUSTOM_CHANNEL_SPO2_HR);

                QSignalMapper *signalMapper = new QSignalMapper(this);
                // menu 1
                menu.addAction("SPO2", signalMapper, SLOT(map()));
                signalMapper->setMapping(menu.actions().last(), spo2HashName);
                // menu 2
                menu.addAction("Hearth Rate", signalMapper, SLOT(map()));
                signalMapper->setMapping(menu.actions().last(), hrHashName);
                connect(signalMapper, SIGNAL(mapped(QString)), this, SLOT(openCustomChannel(QString)));
            }
        }
    }
    menu.addAction("Refresh", this, SLOT(refreshDevice()));
    menu.exec(globalPos);
}

/**
 * @brief MainWindow::clickDeviceAction
 * @param item
 * @details This event fired when double-click child in device tree
 */
void MainWindow::clickDeviceAction(QTreeWidgetItem* item)
{
    if(item->parent()) {
        // extract user data to usb device description
        UsbDeviceDesc description = item->data(0, Qt::UserRole).value<UsbDeviceDesc>();
        int index = item->data(1, Qt::UserRole).toInt();
        // get firmware from device description
        usb_firmware firmware = getUsbFirmware(description.productName, description.developer);
        // create graph widget
        GraphWidget *child = new GraphWidget(firmware, index, this);
        ui->mdiArea->addSubWindow(child);
        // connect signal from thread to widget
        UsbThread* thread = mapDeviceThread[description.hashName()];
        connect(thread, SIGNAL(send(QVector<int32_t>)), child, SLOT(recieve(QVector<int32_t>)));
        child->setWindowTitle(
            QString("%1 %2 [id:%3][channel:%4]")
                .arg(description.productName)
                .arg(firmware.descriptor.at(index))
                .arg(description.hashName())
                .arg(index)
        );
        child->showFullScreen();
    }
    else {
        openDeviceAction();
    }
}
void MainWindow::openCustomChannel(QString hashname)
{
    // assert
    if (!mapCustomChannelType.contains(hashname)) return;
    // get firmware from device description
    UsbDeviceDesc description = mapCustomChannelDescription[hashname];
    int type = mapCustomChannelType[hashname];
    usb_firmware firmware = getUsbFirmware(description.productName, description.developer);
    // create graph widget
    TestWidget *child = new TestWidget(firmware, type, this);
    ui->mdiArea->addSubWindow(child);
    // connect signal from thread to widget
    UsbThread* thread = mapDeviceThread[description.hashName()];
    connect(thread, SIGNAL(send(QVector<int32_t>)), child, SLOT(recieve(QVector<int32_t>)));
    child->setWindowTitle(
        QString("%1 %2 [id:%3]")
            .arg(description.productName)
            .arg(CUSTOM_CHANNEL[type])
            .arg(description.hashName())
    );
    child->showFullScreen();
}

void MainWindow::openDeviceAction()
{
    // get index of selected device
    int i = ui->treeWidget->currentIndex().row();
    UsbDeviceDesc desc = devices->at(i);

    // check exists thread
    if (mapDeviceThread.contains(desc.hashName()))
    {
        ui->statusTextbox->appendHtml(QString("<span style=\"color:#f00\">device is already opened</span>"));
        return;
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("Open USB Device");
    msgBox.setText("Do you want to open this device?");
    msgBox.setStandardButtons(QMessageBox::Open | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Close);
    int ret = msgBox.exec();

    // deny all return except open returned id
    if (ret != QMessageBox::Open) return;

    // show status message
    ui->statusTextbox->appendHtml(
                QString("<b style=\"color:#093\">Open %1 (%2) bus:%3 addr:%4</b>")
                .arg(desc.productName).arg(desc.developer)
                .arg(desc.bus).arg(desc.address));


    // create thread and start
    UsbThread *thread = new UsbThread();
    connect(thread, SIGNAL(printLogMessage(QString)), ui->statusTextbox, SLOT(appendPlainText(QString)));
    connect(thread, SIGNAL(deviceConnectedWithExtraId(UsbDeviceDesc, int)), this, SLOT(addSubDeviceTree(UsbDeviceDesc, int)));
    mapDeviceThread.insert(desc.hashName(), thread);
    thread->open(desc, i);
}

void MainWindow::closeDeviceAction()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Close working device");
    msgBox.setText("Do you want to close?");
    msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Close);
    int ret = msgBox.exec();

    // deny all return except open returned id
    if (ret != QMessageBox::Close) return;


    // get index of selected item
    int index = ui->treeWidget->currentIndex().row();
    UsbDeviceDesc desc = devices->at(index);
    QString name = desc.hashName();

    ui->statusTextbox->appendHtml(
                QString("<b style=\"color:#f00\">Close %1</b>")
                .arg(name));
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    int length = item->childCount();
    for (int i=length-1; i >= 0 ; i--)
    {
        delete item->child(i);
    }
    delete mapDeviceThread[desc.hashName()];
    mapDeviceThread.remove(desc.hashName());
}

void MainWindow::addSubDeviceTree(UsbDeviceDesc description, int extraId)
{
    // get parant item
    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(extraId);
    // prepare usb device description to save at each child
    QVariant var = QVariant::fromValue(description);
    // get firmare and channel list
    usb_firmware firmware = getUsbFirmware(description.productName, description.developer);
    // add channel as a child
    for (int i=0; i<firmware.num_channel; i++) {
        addTreeChild(item, firmware.descriptor.at(i), firmware.unit, var, QVariant(i));
    }
    ui->treeWidget->expandItem(item);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete devices;
    delete usbService;
}
