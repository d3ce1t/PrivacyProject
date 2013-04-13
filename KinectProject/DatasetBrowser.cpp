#include "DatasetBrowser.h"
#include "ui_DatasetBrowser.h"
#include <QDebug>
#include "dataset/MSRDailyActivity3D.h"
#include "dataset/MSRDailyDepthInstance.h"
#include "dataset/MSRDailySkeletonInstance.h"
#include "dataset/MSR3Action3D.h"
#include "InstanceWidgetItem.h"
#include <QGuiApplication>


using namespace dai;

DatasetBrowser::DatasetBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DatasetBrowser)
{
    ui->setupUi(this);

    m_viewer = NULL;
    m_dataset = NULL;
    loadDataset( (Dataset::DatasetType) ui->comboDataset->currentIndex());

    // Connect Signals
    connect(ui->listActivities, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listActors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listSamples, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(listItemChange(QListWidgetItem*)));
    connect(ui->listInstances, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(instanceItemActivated(QListWidgetItem*)));
    connect(ui->comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChange(int)));
}

DatasetBrowser::~DatasetBrowser()
{
    ui->listActivities->clear();
    ui->listActors->clear();
    ui->listSamples->clear();
    ui->listInstances->clear();
    delete ui;
}

void DatasetBrowser::closeEvent(QCloseEvent * event)
{
    Q_UNUSED(event);
    QMainWindow* parent =  dynamic_cast<QMainWindow*>(this->parent());
    parent->show();
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
    DataInstance* instance = NULL;

    if (info.getType() == InstanceInfo::Depth)
        instance = m_dataset->getDepthInstance(info);
    else if (info.getType() == InstanceInfo::Skeleton) {
        instance = m_dataset->getSkeletonInstance(info);
    }

    InstanceViewer* viewer = NULL;

    if (ui->checkBoxSameViewer->isChecked()) {
        if (m_viewer == NULL) {
            m_viewer = new InstanceViewer;
            connect(m_viewer, SIGNAL(viewerClose(InstanceViewer*)), this, SLOT(viewerClosed(InstanceViewer*)));
        }
        viewer = m_viewer;
    } else {
        viewer = new InstanceViewer;
        connect(viewer, SIGNAL(viewerClose(InstanceViewer*)), this, SLOT(viewerClosed(InstanceViewer*)));
    }

    viewer->show();
    instance->setPlayLoop(ui->checkBoxLoop->isChecked());
    viewer->play(instance, ui->checkSync->isChecked());
}

void DatasetBrowser::viewerClosed(InstanceViewer* viewer)
{
    if (m_viewer == viewer) {
        m_viewer = NULL;
    }

    delete viewer;
}

void DatasetBrowser::loadDataset(Dataset::DatasetType type)
{
    if (m_dataset != NULL) {
        delete m_dataset;
        m_dataset = NULL;
    }

    if (type == Dataset::Dataset_MSRDailyActivity3D) {
        m_dataset = new MSRDailyActivity3D();
    } else if (type == Dataset::Dataset_MSRAction3D) {
        m_dataset = new MSR3Action3D();
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

    InstanceInfo::InstanceType showType;

    if (ui->comboType->currentIndex() == 0) {
        showType = InstanceInfo::Depth;
    } else if (ui->comboType->currentIndex() == 1) {
        showType = InstanceInfo::Color;
    } else if (ui->comboType->currentIndex() == 2) {
        showType = InstanceInfo::Skeleton;
    }

    // Prepare Filter
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

    // Get and Load Instances
    const InstanceInfoList* instances = dsMetadata.instances(showType, &activities, &actors, &samples);

    for (int i=0; i<instances->count(); ++i) {
        InstanceInfo* instanceInfo = instances->at(i);
        InstanceWidgetItem* item = new InstanceWidgetItem(instanceInfo->getFileName(), ui->listInstances);
        item->setInfo(*instanceInfo);
    }
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
