#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playback/PlaybackControl.h"
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
    QString number(int value);
    
public slots:
    void initialiseOgre();

private slots:
    void onPlusKeyPressed();
    void onMinusKeyPressed();
    void onSpaceKeyPressed();
    void on_btnStartKinect_clicked();

    void on_btnQuit_clicked();

private:
    void searchMinAndMaxDepth();
    static void testSegmentation();

private:
    dai::InstanceViewerWindow* m_colorViewer;
    OgreScene*            m_ogreScene;
    dai::PlaybackControl* m_playback;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
