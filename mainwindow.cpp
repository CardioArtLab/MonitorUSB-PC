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

    refreshDevice();
}

/**
 * @brief MainWindow::refreshDevice
 * @details List all USB devices
 */
void MainWindow::refreshDevice()
{
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

void MainWindow::addTreeChild(QTreeWidgetItem *parent, QString name, QString description, QVariant userData)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();
    treeItem->setText(0, name);
    treeItem->setText(1, description);
    //treeItem->setData(2, Qt::UserRole, userData);
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
    int i = ui->treeWidget->currentIndex().row();
    UsbDeviceDesc desc = devices->at(i);

    if (mapDeviceThread.contains(desc.hashName())) {
        menu.addAction("Close", this, SLOT(closeDeviceAction()));
    } else {
        menu.addAction("Open", this, SLOT(openDeviceAction()));
    }
    menu.exec(globalPos);
}

/**
 * @brief MainWindow::clickDeviceAction
 * @param item
 * @details This event fired when double-click child in device tree
 */
void MainWindow::clickDeviceAction(QTreeWidgetItem* item)
{
    /*
    if(item->parent()) {
        //TODO: plot graph
        UsbGraphWidget *child = new UsbGraphWidget();
        ui->mdiArea->addSubWindow(child);
        //ui->mdiArea->setActiveSubWindow();
    }
    else
        openDeviceAction();
    */
    GraphWidget *child = new GraphWidget(this);
    ui->mdiArea->addSubWindow(child);
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
    msgBox.setDefaultButton(QMessageBox::Cancel);
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
    msgBox.setDefaultButton(QMessageBox::Cancel);
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
    for (int i=0; i < length ; i++)
    {
        delete item->child(i);
    }
    mapDeviceThread.remove(desc.hashName());
}

void MainWindow::addSubDeviceTree(UsbDeviceDesc description, int extraId)
{
    QTreeWidgetItem *item = ui->treeWidget->topLevelItem(extraId);
    QVariant var;
    var.setValue(description);
    addTreeChild(item, "Plot Graph", "", var);
    ui->treeWidget->expandItem(item);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete devices;
    delete usbService;
}
