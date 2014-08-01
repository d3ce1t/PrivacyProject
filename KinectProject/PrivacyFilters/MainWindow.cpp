#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);
    m_playback.setFPS(10);
    m_device = dai::OpenNIDevice::create("C:/opt/captures/PrimeSense Short-Range (1.09) - 1 user.oni");
}

MainWindow::~MainWindow()
{
    m_playback.stop();
    delete ui;
}

void MainWindow::on_btnStartKinect_clicked()
{
    // Create instances
    shared_ptr<dai::OpenNIColorInstance> colorInstance =
            make_shared<dai::OpenNIColorInstance>(m_device);

    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance =
            make_shared<dai::OpenNIUserTrackerInstance>(m_device);

    // Open Device and configure playback
    m_device->open();
    openni::PlaybackControl* oniPlayback = m_device->playbackControl();
    oniPlayback->setSpeed(0.5);

    // Create Main Producer
    m_playback.clearInstances();
    m_playback.addInstance(colorInstance);
    m_playback.addInstance(userTrackerInstance);
    m_playback.addListener(&m_privacyFilter);

    // Create viewers
    m_viewer = new dai::InstanceViewerWindow;
    m_viewer->setDrawMode(ViewerEngine::BoundingBox);

    // Connect viewers
    //m_playback.addListener(m_viewer);
    m_privacyFilter.addListener(m_viewer);

    // Run
    m_privacyFilter.enableFilter(FILTER_DISABLED);
    m_viewer->show();
    m_playback.play();
}

void MainWindow::onPlusKeyPressed()
{
    /*dai::OpenNIRuntime* openniInstance = dai::OpenNIRuntime::getInstance();
    openni::PlaybackControl* control = openniInstance->playbackControl();
    float speed = control->getSpeed();
    control->setSpeed(speed + 0.05);
    qDebug() << "Current speed" << control->getSpeed();*/
}

void MainWindow::onMinusKeyPressed()
{
    /*dai::OpenNIRuntime* openniInstance = dai::OpenNIRuntime::getInstance();
    openni::PlaybackControl* control = openniInstance->playbackControl();
    float speed = control->getSpeed();
    control->setSpeed(speed - 0.05);
    qDebug() << "Current speed" << control->getSpeed();*/
}

void MainWindow::onSpaceKeyPressed()
{
    //m_playback->pause();
}

void MainWindow::on_btnQuit_clicked()
{
    QApplication::exit(0);
}
