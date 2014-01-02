#include "MainWindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QtWidgets/QDesktopWidget>
#include <fstream>
#include <iostream>
#include "openni/OpenNIDepthInstance.h"
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include "dataset/MSRAction3D/MSR3Action3D.h"
#include "types/DepthFrame.h"
#include "types/SkeletonFrame.h"
#include "types/DataFrame.h"
#include "types/BaseInstance.h"
#include "viewer/InstanceRecorder.h"
#include "KMeans.h"
#include "DepthSeg.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);
    m_playback = nullptr;

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

/*void MainWindow::on_btnParseDataset_clicked()
{
    dai::MSR3Action3D dataset;
    const dai::DatasetMetadata& dsMetadata = dataset.getMetadata();
    const dai::InstanceInfoList* instances = dsMetadata.instances(dai::INSTANCE_SKELETON);

    QListIterator<dai::InstanceInfo*> it(*instances);

    while (it.hasNext())
    {
        dai::InstanceInfo* info = it.next();
        auto dataInstance = static_pointer_cast<dai::DataInstance<dai::SkeletonFrame>>(dataset.getInstance(*info));

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

            shared_ptr<dai::SkeletonFrame> skeletonFrame = static_pointer_cast<dai::SkeletonFrame>(dataInstance->frame());
            of << (skeletonFrame->getIndex() + 1) << endl;

            for (int i=0; i<17; ++i) {
                dai::Quaternion::QuaternionType type = (dai::Quaternion::QuaternionType) i;
                const dai::Quaternion& quaternion = skeletonFrame->getSkeleton(1)->getQuaternion(type);

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
}*/

void MainWindow::on_btnStartKinect_clicked()
{
    // Create instance
    shared_ptr<dai::OpenNIColorInstance> colorInstance(new dai::OpenNIColorInstance);
    //shared_ptr<dai::OpenNIDepthInstance> depthInstance(new dai::OpenNIDepthInstance);
    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance(new dai::OpenNIUserTrackerInstance);

    // Create Playback
    m_playback = new dai::PlaybackControl;
    m_playback->setFPS(15);
    connect(m_playback, &dai::PlaybackControl::onPlaybackFinished, m_playback, &dai::PlaybackControl::deleteLater);

    // Create viewers
    m_colorViewer = new dai::InstanceViewerWindow(dai::MODE_2D);
    //m_ogreScene = new OgreScene;
    //m_colorViewer->qmlEngine().rootContext()->setContextProperty("Camera", m_ogreScene->cameraNode());
    //m_colorViewer->qmlEngine().rootContext()->setContextProperty("OgreEngine", m_ogreScene->engine());
    m_colorViewer->initialise();

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    //connect(m_colorViewer->quickWindow(), &QQuickWindow::beforeSynchronizing, this, &MainWindow::initialiseOgre, Qt::DirectConnection);
    connect(m_colorViewer->viewerEngine(), &ViewerEngine::plusKeyPressed, this, &MainWindow::onPlusKeyPressed);
    connect(m_colorViewer->viewerEngine(), &ViewerEngine::minusKeyPressed, this, &MainWindow::onMinusKeyPressed);
    connect(m_colorViewer->viewerEngine(), &ViewerEngine::spaceKeyPressed, this, &MainWindow::onSpaceKeyPressed);

    // Connect all together
    m_playback->addInstance(colorInstance);
    //m_playback->addInstance(depthInstance);
    m_playback->addInstance(userTrackerInstance);

    m_playback->addListener(m_colorViewer, colorInstance);
    //m_playback->addListener(m_colorViewer, depthInstance);
    m_playback->addListener(m_colorViewer, userTrackerInstance);
    //m_playback->addListener(m_ogreScene, skeletonInstance);

    // Run
    m_playback->play();
    m_colorViewer->show();
}

void MainWindow::onPlusKeyPressed()
{
    dai::OpenNIRuntime* openniInstance = dai::OpenNIRuntime::getInstance();
    openni::PlaybackControl* control = openniInstance->playbackControl();
    float speed = control->getSpeed();
    control->setSpeed(speed + 0.05);
    qDebug() << "Current speed" << control->getSpeed();
}

void MainWindow::onMinusKeyPressed()
{
    dai::OpenNIRuntime* openniInstance = dai::OpenNIRuntime::getInstance();
    openni::PlaybackControl* control = openniInstance->playbackControl();
    float speed = control->getSpeed();
    control->setSpeed(speed - 0.05);
    qDebug() << "Current speed" << control->getSpeed();
}

void MainWindow::onSpaceKeyPressed()
{
    dai::OpenNIRuntime* openniInstance = dai::OpenNIRuntime::getInstance();
    openni::PlaybackControl* control = openniInstance->playbackControl();
    if (control->getSpeed() > 0)
        control->setSpeed(-1);
    else
        control->setSpeed(1);

    qDebug() << "Current speed" << control->getSpeed();
}

void MainWindow::searchMinAndMaxDepth()
{
    dai::MSR3Action3D* dataset = new dai::MSR3Action3D();
    const dai::DatasetMetadata& dsMetadata = dataset->getMetadata();
    const dai::InstanceInfoList* instances = dsMetadata.instances(dai::INSTANCE_DEPTH);

    QListIterator<dai::InstanceInfo*> it(*instances);
    int framesProcessed = 0;

    // Traverse all dataset instances
    while (it.hasNext())
    {
        dai::InstanceInfo* info = it.next();
        auto dataInstance = dataset->getInstance(info->getActivity(), info->getActor(),
                                                 info->getSample(), info->getType());

        dataInstance->open();

        qDebug() << "Instance" << info->getActivity() << info->getActor() << info->getSample() << "open";

        // Traverse each frame of this instance
        while (dataInstance->hasNext())
        {
            dataInstance->readNextFrame();
            //shared_ptr<dai::DepthFrame> depthFrame = static_pointer_cast<dai::DepthFrame>(dataInstance->frame());
            framesProcessed++;
            //qDebug() << "Frame: " << framesProcessed;
        }

        qDebug() << "Close instance";
        dataInstance->close();
    }
}

void MainWindow::on_btnQuit_clicked()
{
    if (m_playback != nullptr) {
        m_playback->stop();
    }

    QApplication::exit(0);
}

/*void MainWindow::initialiseOgre()
{
    // we only want to initialize once
    //disconnect(m_colorViewer->quickWindow(), &QQuickWindow::beforeSynchronizing, this, &MainWindow::initialiseOgre);
    //m_ogreScene->initialiseOgre(m_colorViewer->quickWindow());
}*/
