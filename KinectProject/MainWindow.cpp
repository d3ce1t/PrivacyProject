#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "viewer/InstanceViewer.h"
#include "dataset/Dataset.h"
#include <QDebug>
#include <QVector>
#include "dataset/MSR3Action3D.h"
#include <fstream>
#include "OpenNIDepthInstance.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_browser = NULL;
    QWidget::setFixedSize(this->width(), this->height());
}

MainWindow::~MainWindow()
{
    if (m_browser != NULL) {
        delete m_browser;
        m_browser = NULL;
    }

    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    InstanceViewer* viewer = new InstanceViewer;
    connect(viewer, SIGNAL(viewerClose(InstanceViewer*)), this, SLOT(viewerClosed(InstanceViewer*)));
    dai::OpenNIDepthInstance* instance = new dai::OpenNIDepthInstance;

    viewer->show();
    //instance->setPlayLoop(ui->checkBoxLoop->isChecked());
    viewer->play(instance, false);
}

void MainWindow::viewerClosed(InstanceViewer *viewer)
{
    delete viewer;
}

QString MainWindow::number(int value)
{
    QString result = QString::number(value);

    if (value < 10)
        result = "0" + QString::number(value);

    return result;
}

void MainWindow::on_pushButton_2_clicked()
{
    if (m_browser == NULL) {
        m_browser = new DatasetBrowser(this);
    }

    m_browser->show();
    m_browser->activateWindow();
    this->hide();
}

void MainWindow::on_pushButton_3_clicked()
{
    dai::MSR3Action3D* dataset = new dai::MSR3Action3D();
    const dai::DatasetMetadata& dsMetadata = dataset->getMetadata();
    const dai::InstanceInfoList* instances = dsMetadata.instances(dai::InstanceInfo::Skeleton);

    QListIterator<dai::InstanceInfo*> it(*instances);

    while (it.hasNext())
    {
        dai::InstanceInfo* info = it.next();
        dai::MSRActionSkeletonInstance* dataInstance = dataset->getSkeletonInstance(info->getActivity(), info->getActor(), info->getSample());

        dataInstance->open();

        qDebug() << "Instance" << info->getActivity() << info->getActor() << info->getSample() << "open";
        int framesProcessed = 0;

        QString fileName = "a" + number(info->getActivity()) +
                "_s" + number(info->getActor()) +
                "_e" + number(info->getSample()) + "_quaternion.txt";

        ofstream of;
        of.open( (dsMetadata.getPath() + "/" + fileName).toStdString().c_str(), ios::out | ios::trunc );
        of << dataInstance->getTotalFrames() << endl;

        while (dataInstance->hasNext() && of.is_open())
        {
            const dai::Skeleton& skeletonFrame = dataInstance->nextFrame();

            of << (skeletonFrame.getIndex() + 1) << endl;

            for (int i=0; i<17; ++i) {
                dai::Quaternion::QuaternionType type = (dai::Quaternion::QuaternionType) i;
                const dai::Quaternion& quaternion = skeletonFrame.getQuaternion(type);

                float w = quaternion.scalar();
                float x = quaternion.vector().x();
                float y = quaternion.vector().y();
                float z = quaternion.vector().z();
                float theta = quaternion.getAngle();

                of << w << " " << x << " " << y << " " << z << " " << theta << endl;
            }

            framesProcessed++;
            qDebug() << "Frame: " << framesProcessed;
        }

        qDebug() << "Close instance";
        dataInstance->close();
        of.close();
    }
}
