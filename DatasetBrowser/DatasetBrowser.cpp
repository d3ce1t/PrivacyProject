#include "DatasetBrowser.h"
#include "ui_DatasetBrowser.h"
#include "dataset/MSRDaily/MSRDailyActivity3D.h"
#include "dataset/MSRAction3D/MSR3Action3D.h"
#include "dataset/DAI/DAIDataset.h"
#include "playback/PlaybackControl.h"
#include "viewer/InstanceViewerWindow.h"
#include "InstanceWidgetItem.h"
#include <QGuiApplication>
#include <QDebug>
#include <QtWidgets/QDesktopWidget>

using namespace dai;

DatasetBrowser::DatasetBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DatasetBrowser)
{
    ui->setupUi(this);

    m_playback.enablePlayLoop(true);
    m_dataset = nullptr;
    loadDataset( (Dataset::DatasetType) ui->comboDataset->currentIndex());

    // Connect Signals
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

    if (m_dataset != nullptr) {
        delete m_dataset;
        m_dataset = nullptr;
    }

    ui->listActivities->clear();
    ui->listActors->clear();
    ui->listSamples->clear();
    ui->listInstances->clear();
    ui->comboDataset->clear();
    ui->comboType->clear();
    delete ui;
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
        InstanceViewerWindow* windowViewer = new InstanceViewerWindow;

        if (instance->getType() == INSTANCE_COLOR || instance->getType() == INSTANCE_USER) {
            windowViewer->setMode(MODE_2D);
        }
        else if (instance->getType() == INSTANCE_DEPTH || instance->getType() == INSTANCE_SKELETON) {
            windowViewer->setMode(MODE_3D);
        }

        m_playback.addInstance(instance);
        m_playback.addListener(windowViewer, instance);
        m_playback.play(ui->checkSync->isChecked());
        windowViewer->setTitle("Instance Viewer (" + instance->getTitle() + ")");
        windowViewer->show();
    }
}

void DatasetBrowser::loadDataset(Dataset::DatasetType type)
{
    if (m_dataset != nullptr) {
        delete m_dataset;
        m_dataset = nullptr;
    }

    if (type == Dataset::Dataset_MSRDailyActivity3D) {
        m_dataset = new MSRDailyActivity3D();
    } else if (type == Dataset::Dataset_MSRAction3D) {
        m_dataset = new MSR3Action3D();
    } else if (type == Dataset::Dataset_DAI) {
        m_dataset = new DAIDataset();
    }

    // Load widgets with DataSet Info
    const DatasetMetadata& info = m_dataset->getMetadata();

    // Load Activities
    ui->listActivities->clear();
    for (int i=1; i<=info.getNumberOfActivities(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(info.getActivityName(i), ui->listActivities);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked);
    }

    // Load Actors
    ui->listActors->clear();
    for (int i=1; i<=info.getNumberOfActors(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(info.getActorName(i), ui->listActors);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked); // AND initialize check state
    }

    // Load Samples
    ui->listSamples->clear();
    for (int i=1; i<=info.getNumberOfSampleTypes(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(info.getSampleName(i), ui->listSamples);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
        item->setCheckState(Qt::Checked); // AND initialize check state
    }

    loadInstances();
}

void DatasetBrowser::loadInstances()
{
    const DatasetMetadata& dsMetadata = m_dataset->getMetadata();

    ui->listInstances->clear();

    // Prepare Filter
    InstanceType showType = (InstanceType) ui->comboType->currentIndex();
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

void DatasetBrowser::on_comboDataset_activated(int index)
{
    loadDataset((Dataset::DatasetType) index);
}
