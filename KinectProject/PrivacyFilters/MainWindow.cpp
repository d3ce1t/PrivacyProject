#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "openni/OpenNIColorInstance.h"
#include "openni/OpenNIUserTrackerInstance.h"
#include <QElapsedTimer>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{   
    ui->setupUi(this);
    m_playback.setFPS(25);
    m_device = dai::OpenNIDevice::create("/mnt/files/capture/PrimeSense Short-Range (1.09) - 1 user.oni");
    //m_device = dai::OpenNIDevice::create();
    //m_device = dai::OpenNIDevice::create("C:/datasets/HuDaAct/S5_C4_U10_BG.oni");
    //m_device = dai::OpenNIDevice::create("C:/datasets/HuDaAct/S14_C12_U30_D1.oni");
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
    //m_device->setRegistration(true);
    m_device->open();

    if (m_device->isFile()) {
        openni::PlaybackControl* oniPlayback = m_device->playbackControl();
        oniPlayback->setSpeed(1.0f);
    }

    // Create Main Producer
    m_playback.clearInstances();
    m_playback.addInstance(colorInstance);
    m_playback.addInstance(userTrackerInstance);
    //m_playback.addListener(&m_depthFilter);
    m_playback.addListener(&m_privacyFilter);

    // Create viewers
    dai::InstanceViewerWindow* out_viewer_color = new dai::InstanceViewerWindow;
    //dai::InstanceViewerWindow* out_viewer_depth = new dai::InstanceViewerWindow;
    //out_viewer->setDrawMode(ViewerEngine::BoundingBox);

    // Connect viewers
    //m_playback.addListener(out_viewer_color);
    //m_depthFilter.addListener(out_viewer_depth);
    m_privacyFilter.addListener(out_viewer_color);

    // Run
    m_privacyFilter.enableFilter(FILTER_DISABLED);
    m_privacyFilter.m_control.show();
    out_viewer_color->show();
    //out_viewer_depth->show();
    m_playback.play();
}

void MainWindow::test()
{
    shared_ptr<dai::OpenNIColorInstance> colorInstance =
            make_shared<dai::OpenNIColorInstance>(m_device);

    m_device->open();

    shared_ptr<ColorFrame> colorFrame = make_shared<ColorFrame>(640, 480);
    QHashDataFrames readFrames;
    readFrames.insert(DataFrame::Color, colorFrame);

    // Skip Initialisation costs
    for (int i=0; i<75; ++i) {
        colorInstance->readNextFrame(readFrames);
    }

    QElapsedTimer timer;
    qint64 total_time = 0;
    int i = 0;

    // Start measure
    while (colorInstance->hasNext() && i < 500)
    {
        timer.start();
        colorInstance->readNextFrame(readFrames);
        total_time += timer.elapsed();
        qDebug() << "Frame readed" << i++;
    }

    float average = total_time / float(i);
    qDebug() << "Average" << average;
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
