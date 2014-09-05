#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playback/PlaybackControl.h"
#include "filters/PrivacyFilter.h"
#include "viewer/DepthFilter.h"
#include "viewer/InstanceViewerWindow.h"
#include "openni/OpenNIDevice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
    void test();
    
private slots:
    void onPlusKeyPressed();
    void onMinusKeyPressed();
    void onSpaceKeyPressed();
    void on_btnStartKinect_clicked();
    void on_btnQuit_clicked();

private:
    dai::OpenNIDevice*    m_device;
    dai::PlaybackControl  m_playback;
    dai::PrivacyFilter    m_privacyFilter;
    dai::DepthFilter      m_depthFilter;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
