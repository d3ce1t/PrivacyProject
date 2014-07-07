#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playback/PlaybackControl.h"
#include "filters/PrivacyFilter.h"
#include "ogre/OgreScene.h"
#include "viewer/InstanceViewerWindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
    
public slots:
    void initialiseOgre();
    void renderScreen();

private slots:
    void onPlusKeyPressed();
    void onMinusKeyPressed();
    void onSpaceKeyPressed();
    void on_btnStartKinect_clicked();
    void on_btnQuit_clicked();

private:
    dai::InstanceViewerWindow* m_viewer;
    OgreScene*            m_ogreScene;
    dai::PlaybackControl  m_playback;
    dai::PrivacyFilter    m_privacyFilter;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
