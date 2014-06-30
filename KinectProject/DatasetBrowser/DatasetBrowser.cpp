#include "DatasetBrowser.h"
#include "ui_DatasetBrowser.h"
#include "dataset/MSRDaily/MSRDailyActivity3D.h"
#include "dataset/MSRAction3D/MSR3Action3D.h"
#include "playback/PlaybackControl.h"
#include "viewer/InstanceViewerWindow.h"
#include "InstanceWidgetItem.h"
#include "DatasetSelector.h"
#include <QGuiApplication>
#include <QDebug>
#include <QtWidgets/QDesktopWidget>
#include <QTimer>
#include <QMessageBox>

using namespace dai;

DatasetBrowser::DatasetBrowser(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::DatasetBrowser), m_settings(this)
{
    ui->setupUi(this);

    m_playback.enablePlayLoop(true);
    m_playback.setFPS(10);
    m_dataset = nullptr;

    // First Setup
    if (m_settings.getMSRAction3D().isEmpty() || m_settings.getMSRDailyActivity3D().isEmpty()) {
        QTimer::singleShot(1, &m_settings, SLOT(show()));
    }

    // Connect Signals
    connect(ui->actionClose_dataset, SIGNAL(triggered()), this, SLOT(closeDataset()));
    connect(ui->actionOpen_dataset, SIGNAL(triggered()), this, SLOT(openDatasetSelector()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettings()));
    connect(ui->listActivities, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listActors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listSamples, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listInstances, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(instanceItemActivated(QListWidgetItem*)));
    connect(ui->comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChange(int)));

    // Show this window centered
    QDesktopWidget *desktop = QApplication::desktop();
    int screenWidth, width;
    int screenHeight, height;
    int x, y;
    QSize windowSize;

    screenWidth = desktop->availableGeometry(desktop->primaryScreen()).width();
    screenHeight = desktop->availableGeometry(desktop->primaryScreen()).height();
    windowSize = size();
    width = windowSize.width();
    height = windowSize.height();

    x = (screenWidth - width) / 2;
    y = (screenHeight - height) / 2;
    y -= 50;

    move ( x, y );
}

DatasetBrowser::~DatasetBrowser()
{
    ui->listActivities->disconnect();
    ui->listActors->disconnect();
    ui->listSamples->disconnect();
    ui->listInstances->disconnect();
    ui->comboType->disconnect();

    closeDataset();
    delete ui;
}

void DatasetBrowser::openSettings()
{
    if (m_settings.exec() == QDialog::Accepted) {
        closeDataset();
    }
}

void DatasetBrowser::openDatasetSelector()
{
    DatasetSelector* window = new DatasetSelector(this);

    if (window->exec() != QDialog::Rejected) {
        loadDataset( (Dataset::DatasetType) (window->result()-1) );
    }
}

void DatasetBrowser::closeDataset()
{
    ui->comboType->blockSignals(true);

    if (m_dataset != nullptr) {
        delete m_dataset;
        m_dataset = nullptr;
    }

    ui->listActivities->clear();
    ui->listActors->clear();
    ui->listSamples->clear();
    ui->listInstances->clear();
    ui->comboType->clear();

    ui->comboType->blockSignals(false);
}

void DatasetBrowser::listItemChange(QListWidgetItem * item)
{
    Q_UNUSED(item);
    loadInstances();
}

void DatasetBrowser::comboBoxChange(int index)
{
    Q_UNUSED(index);
    loadInstances();
}

void DatasetBrowser::instanceItemActivated(QListWidgetItem * item)
{
    InstanceWidgetItem* instanceItem = dynamic_cast<InstanceWidgetItem*>(item);
    InstanceInfo& info = instanceItem->getInfo();
    shared_ptr<BaseInstance> instance = m_dataset->getInstance(info);

    if (instance)
    {
        ViewerMode mode;

        if (info.getType() == DataFrame::Color || info.getType() == DataFrame::User) {
            mode = MODE_2D;
        } else if (info.getType() == DataFrame::Depth || info.getType() == DataFrame::Skeleton) {
            mode = MODE_3D;
        }

        InstanceViewerWindow* windowViewer = new InstanceViewerWindow(mode);
        windowViewer->initialise();
        m_playback.clearInstances();
        m_playback.addInstance(instance);
        connect(m_playback.worker(), &dai::PlaybackWorker::onNewFrames, windowViewer, &dai::InstanceViewerWindow::newFrames);

        try {
            m_playback.play();
            windowViewer->setTitle("Instance Viewer (" + instance->getTitle() + ")");
            windowViewer->show();
        }
        catch (CannotOpenInstanceException ex)
        {
            QMessageBox::warning(this, tr("Error reading the instance"),
                                           tr("The selected instance cannot be opened maybe\n"
                                              "due to file permissions or wrong dataset path"));
        }
    }
}

void DatasetBrowser::loadDataset(Dataset::DatasetType type)
{
    closeDataset();

    ui->comboType->blockSignals(true);

    if (type == Dataset::Dataset_MSRDailyActivity3D) {
        m_dataset = new MSRDailyActivity3D();
        m_dataset->setPath(m_settings.getMSRDailyActivity3D());
    } else if (type == Dataset::Dataset_MSRAction3D) {
        m_dataset = new MSR3Action3D();
        m_dataset->setPath(m_settings.getMSRAction3D());
    }

    // Load widgets with DataSet Info
    const DatasetMetadata& dsMetaData = m_dataset->getMetadata();

    // Load Activities
    for (int i=1; i<=dsMetaData.getNumberOfActivities(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(dsMetaData.getActivityName(i), ui->listActivities);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked);
    }

    // Load Actors
    for (int i=1; i<=dsMetaData.getNumberOfActors(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(dsMetaData.getActorName(i), ui->listActors);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked); // AND initialize check state
    }

    // Load Samples
    for (int i=1; i<=dsMetaData.getNumberOfSampleTypes(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(dsMetaData.getSampleName(i), ui->listSamples);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked); // AND initialize check state
    }

    // Load Instance Types
    const DataFrame::SupportedFrames supportedFrames = dsMetaData.availableInstanceTypes();

    if (supportedFrames.testFlag(DataFrame::Depth)) {
        ui->comboType->addItem("Depth", QVariant(DataFrame::Depth));
    }
    if (supportedFrames.testFlag(DataFrame::Color)) {
        ui->comboType->addItem("Color", QVariant(DataFrame::Color));
    }
    if (supportedFrames.testFlag(DataFrame::Skeleton)) {
        ui->comboType->addItem("Skeleton", QVariant(DataFrame::Skeleton));
    }
    if (supportedFrames.testFlag(DataFrame::User)) {
        ui->comboType->addItem("User", QVariant(DataFrame::User));
    }

    ui->comboType->blockSignals(false);
    loadInstances();
}

void DatasetBrowser::loadInstances()
{
    const DatasetMetadata& dsMetadata = m_dataset->getMetadata();

    ui->listInstances->clear();

    // Prepare Filter
    DataFrame::FrameType showType = (DataFrame::FrameType) ui->comboType->itemData(ui->comboType->currentIndex()).toInt();
    QList<int> activities;

    for (int i=0; i<ui->listActivities->count(); ++i) {
        QListWidgetItem* item = ui->listActivities->item(i);
        if (item != 0 && item->checkState() == Qt::Checked)
            activities.append(i+1);
    }

    QList<int> actors;

    for (int i=0; i<ui->listActors->count(); ++i) {
        QListWidgetItem* item = ui->listActors->item(i);
        if (item != 0 && item->checkState() == Qt::Checked)
            actors.append(i+1);
    }

    QList<int> samples;

    for (int i=0; i<ui->listSamples->count(); ++i) {
        QListWidgetItem* item = ui->listSamples->item(i);
        if (item != 0 && item->checkState() == Qt::Checked)
            samples.append(i+1);
    }

    // Get and Load Instances (I'm responsible to free this memory)
    const InstanceInfoList* instances = dsMetadata.instances(showType, &activities, &actors, &samples);

    for (int i=0; i<instances->count(); ++i) {
        InstanceInfo* instanceInfo = instances->at(i);
        InstanceWidgetItem* item = new InstanceWidgetItem(instanceInfo->getFileName(), ui->listInstances);
        item->setInfo(*instanceInfo);
    }

    delete instances;
}

void DatasetBrowser::on_btnSelectAllActivities_clicked()
{
    for (int i=0; i<ui->listActivities->count(); ++i) {
        QListWidgetItem* item = ui->listActivities->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void DatasetBrowser::on_btnUnselectAllActivities_clicked()
{
    for (int i=0; i<ui->listActivities->count(); ++i) {
        QListWidgetItem* item = ui->listActivities->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void DatasetBrowser::on_btnSelectAllActors_clicked()
{
    for (int i=0; i<ui->listActors->count(); ++i) {
        QListWidgetItem* item = ui->listActors->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void DatasetBrowser::on_btnUnselectAllActors_clicked()
{
    for (int i=0; i<ui->listActors->count(); ++i) {
        QListWidgetItem* item = ui->listActors->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void DatasetBrowser::on_btnSelectAllSamples_clicked()
{
    for (int i=0; i<ui->listSamples->count(); ++i) {
        QListWidgetItem* item = ui->listSamples->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void DatasetBrowser::on_btnUnselectAllSamples_clicked()
{
    for (int i=0; i<ui->listSamples->count(); ++i) {
        QListWidgetItem* item = ui->listSamples->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}
