#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "viewer/InstanceViewerWindow.h"
#include "dataset/Dataset.h"
#include <QDebug>
#include "dataset/MSRAction3D/MSR3Action3D.h"
#include "dataset/DAI/DAIDataset.h"
#include "openni/OpenNIDepthInstance.h"
#include "openni/OpenNIColorInstance.h"
#include "types/DepthFrame.h"
#include "viewer/PlaybackControl.h"
#include "filters/BasicFilter.h"
#include "KMeans.h"
#include "DepthSeg.h"
#include <fstream>
#include <iostream>
#include "types/DataFrame.h"
#include "QtWidgets/QDesktopWidget"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);

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
    setFixedSize(windowSize.width(), windowSize.height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::number(int value)
{
    QString result = QString::number(value);

    if (value < 10)
        result = "0" + QString::number(value);

    return result;
}

void MainWindow::testSegmentation()
{
    /*float data[] = {0.1, 0.3, 0.2, 0.5, 0.6, 0.2, 0.3, 0.1, 0.30, 0.36, 0.45, 0.3, 0.15, 0.17, -0.1, -0.3, -0.2, -0.5, -0.6, -0.2, -0.3, -0.1, -0.30, -0.36, -0.45, -0.3, -0.15, -0.17};
    //float data[] = {1, 3, 2, 5, 6, 2, 3, 1, 30, 36, 45, 3, 15, 17};
    float n = sizeof(data) / sizeof(float);
    const dai::KMeans* kmeans = dai::KMeans::execute(data, n, 4);

    qDebug() << "--------------------------------------";
    qDebug() << "Centroids" << kmeans->getCentroids();
    qDebug() << "Compactness" << kmeans->getCompactness();

    const QList<float>* values = kmeans->getClusterValues();
    for (int i=0; i<kmeans->getK(); ++i) {
        qDebug() << "Cluster" << i << values[i];
    }*/


    int width = 12;
    int height = 12;

    dai::DepthFrame depth_frame(width, height);

    float my_frame1[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
        0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0,
        0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0,
        0, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0,
        0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    float my_frame2[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 1, 4, 4, 4, 4, 4, 4, 4, 4, 1, 0,
        0, 1, 4, 8, 8, 8, 8, 8, 8, 4, 1, 0,
        0, 1, 4, 8, 8, 8, 8, 8, 8, 4, 1, 0,
        0, 1, 4, 8, 8, 9, 9, 8, 8, 4, 1, 0,
        0, 1, 4, 8, 8, 9, 9, 8, 8, 4, 1, 0,
        0, 1, 4, 8, 8, 8, 8, 8, 8, 4, 1, 0,
        0, 1, 4, 8, 8, 8, 8, 8, 8, 4, 1, 0,
        0, 1, 4, 4, 4, 4, 4, 4, 4, 4, 1, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    for (int i=0; i<12; ++i)
    {
        for (int j=0; j<12; ++j)
        {
            depth_frame.setItem(i, j, my_frame2[i*12 + j ]);
        }
    }

    dai::DepthSeg dseg(depth_frame);
    dseg.execute();
    //dseg.print_cluster_mask(3, 3);

    for (int i=0; i<height; ++i) {
        for (int j=0; j<width; ++j) {
            cout << dseg.getCluster(i, j) + 1 << " ";
        }
        cout << endl;
    }
}

void MainWindow::on_btnOpenDataSets_clicked()
{
    m_browser.show();
}

void MainWindow::on_btnParseDataset_clicked()
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
            dataInstance->readNextFrame();
            const dai::Skeleton& skeletonFrame = dataInstance->frame();

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

void MainWindow::on_btnTest_clicked()
{
    dai::DAIDataset dataset;
    dai::DAIColorInstance* colorInstance = dataset.getColorInstance(1, 1, 1);
    dai::DAIDepthInstance* depthInstance = dataset.getDepthInstance(1, 1, 1);

    dai::PlaybackControl* playback = new dai::PlaybackControl;
    connect(playback, &dai::PlaybackControl::onPlaybackStoped, playback, &dai::PlaybackControl::deleteLater);
    playback->addInstance(colorInstance);
    playback->addInstance(depthInstance);
    playback->enablePlayLoop(true);

    // Show color instance
    InstanceViewerWindow* mainViewer = new InstanceViewerWindow;
    mainViewer->setPlayback(playback);

    // Show color instance
    InstanceViewerWindow* colorViewer = new InstanceViewerWindow;
    colorViewer->setPlayback(playback);

    playback->addNewFrameListener(mainViewer, colorInstance);
    playback->addNewFrameListener(colorViewer, depthInstance);

    playback->play();
    mainViewer->show();
    colorViewer->show();
}

void MainWindow::on_btnStartKinect_clicked()
{
    // Create instance
    dai::OpenNIColorInstance* colorInstance = new dai::OpenNIColorInstance;
    dai::OpenNIDepthInstance* depthInstance = new dai::OpenNIDepthInstance;
    //colorInstance->setOutputFile("/files/capture/capture.rgb");
    //colorInstance->setOutputFile("/ramfs/jose/capture.rgb");

    // Create Playback
    dai::PlaybackControl* playback = new dai::PlaybackControl;
    connect(playback, &dai::PlaybackControl::onPlaybackStoped, playback, &dai::PlaybackControl::deleteLater);

    // Create viewers
    InstanceViewerWindow* colorViewer = new InstanceViewerWindow;
    InstanceViewerWindow* depthViewer = new InstanceViewerWindow;

    // Connect all together
    playback->addInstance(colorInstance);
    playback->addInstance(depthInstance);
    colorViewer->setPlayback(playback);
    depthViewer->setPlayback(playback);
    playback->addNewFrameListener(colorViewer, colorInstance);
    playback->addNewFrameListener(colorViewer, depthInstance);
    playback->addNewFrameListener(depthViewer, depthInstance);

    // Run
    playback->play();
    colorViewer->show();
    depthViewer->show();
}
