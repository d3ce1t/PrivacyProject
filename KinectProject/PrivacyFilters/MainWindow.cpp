#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtWidgets/QDesktopWidget>
#include <fstream>
#include <iostream>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIDepthInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);

    // Setup playback
    m_playback.setFPS(10);

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

void MainWindow::on_btnStartKinect_clicked()
{
    // Create instance
    shared_ptr<dai::OpenNIColorInstance> colorInstance = make_shared<dai::OpenNIColorInstance>();
    //shared_ptr<dai::OpenNIDepthInstance> depthInstance = make_shared<dai::OpenNIDepthInstance>();
    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance = make_shared<dai::OpenNIUserTrackerInstance>();

    // Playback
    m_playback.clearInstances();
    m_playback.addInstance(colorInstance);
    m_playback.addInstance(userTrackerInstance);
    //m_playback.addInstance(depthInstance);

    // Create viewers
    m_viewer = new dai::InstanceViewerWindow(dai::MODE_2D);
    m_ogreScene = new OgreScene;
    connect(m_viewer, &dai::InstanceViewerWindow::destroyed, m_ogreScene, &OgreScene::deleteLater);
    m_viewer->qmlEngine().rootContext()->setContextProperty("Scene", m_ogreScene);
    m_viewer->qmlEngine().rootContext()->setContextProperty("Camera", m_ogreScene->cameraNode());
    m_viewer->qmlEngine().rootContext()->setContextProperty("OgreEngine", m_ogreScene->engine());
    m_viewer->initialise();

    // start Ogre once we are in the rendering thread (Ogre must live in the rendering thread)
    connect(m_viewer->quickWindow(), &QQuickWindow::beforeSynchronizing, this, &MainWindow::initialiseOgre, Qt::DirectConnection);
    connect(m_viewer->viewerEngine(), &ViewerEngine::plusKeyPressed, this, &MainWindow::onPlusKeyPressed);
    connect(m_viewer->viewerEngine(), &ViewerEngine::minusKeyPressed, this, &MainWindow::onMinusKeyPressed);
    connect(m_viewer->viewerEngine(), &ViewerEngine::spaceKeyPressed, this, &MainWindow::onSpaceKeyPressed);

    // Connect playback with viewers
    connect(m_playback.worker(), &dai::PlaybackWorker::onNewFrames, m_viewer, &dai::InstanceViewerWindow::newFrames);
    connect(m_playback.worker(), &dai::PlaybackWorker::onNewFrames, m_ogreScene, &OgreScene::newFrames);

    // Run
    m_viewer->show();
    m_playback.play();
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
    //m_playback->pause();
}

void MainWindow::on_btnQuit_clicked()
{
    m_playback.stop();
    QApplication::exit(0);
}

void MainWindow::initialiseOgre()
{
    // we only want to initialize once
    disconnect(m_viewer->quickWindow(), &QQuickWindow::beforeSynchronizing, this, &MainWindow::initialiseOgre);
    connect(m_viewer->quickWindow(), &QQuickWindow::beforeSynchronizing, this, &MainWindow::renderScreen, Qt::DirectConnection);
    m_ogreScene->initialiseOgre(m_viewer->quickWindow());
}

void MainWindow::renderScreen()
{
    m_ogreScene->renderOgre();
}
