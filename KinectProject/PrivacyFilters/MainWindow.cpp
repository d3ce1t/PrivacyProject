#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtWidgets/QDesktopWidget>
#include <iostream>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include "ogre/OgreScene.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);

    m_playback.setFPS(15);

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
    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance = make_shared<dai::OpenNIUserTrackerInstance>();

    // Create Main Producer
    m_playback.clearInstances();
    m_playback.addInstance(colorInstance);
    m_playback.addInstance(userTrackerInstance);
    m_playback.addListener(&m_privacyFilter);

    // Create viewers
    m_viewer = new dai::InstanceViewerWindow;
    //OgreScene* ogreViewer = new OgreScene;

    // Connect viewers
    m_privacyFilter.addListener(m_viewer);
    //m_playback.addListener(ogreViewer);
    //connect(m_viewer->viewerEngine(), &ViewerEngine::plusKeyPressed, this, &MainWindow::onPlusKeyPressed);
    //connect(m_viewer->viewerEngine(), &ViewerEngine::minusKeyPressed, this, &MainWindow::onMinusKeyPressed);
    //connect(m_viewer->viewerEngine(), &ViewerEngine::spaceKeyPressed, this, &MainWindow::onSpaceKeyPressed);

    // Run
    m_privacyFilter.enableFilter(QMLEnumsWrapper::FILTER_SKELETON);
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
    //m_playback.stop();
    QApplication::exit(0);
}
