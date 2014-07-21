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
}

MainWindow::~MainWindow()
{
    m_playback.stop();
    delete ui;
}

void MainWindow::on_btnStartKinect_clicked()
{
    // Create instance
    shared_ptr<dai::OpenNIColorInstance> colorInstance = make_shared<dai::OpenNIColorInstance>();
    shared_ptr<dai::OpenNIUserTrackerInstance> userTrackerInstance = make_shared<dai::OpenNIUserTrackerInstance>();
    openni::PlaybackControl* oniPlayback = OpenNIRuntime::getInstance()->playbackControl();
    oniPlayback->setSpeed(0.5);

    // Create Main Producer
    m_playback.clearInstances();
    m_playback.addInstance(colorInstance);
    m_playback.addInstance(userTrackerInstance);
    m_playback.addListener(&m_privacyFilter);

    // Create viewers
    m_viewer = new dai::InstanceViewerWindow;

    // Connect viewers
    m_privacyFilter.addListener(m_viewer);

    // Run
    m_privacyFilter.enableFilter(FILTER_DISABLED);
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
    QApplication::exit(0);
}
