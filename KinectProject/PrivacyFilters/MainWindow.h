#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "playback/PlaybackControl.h"

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
    
private slots:
    void on_btnParseDataset_clicked();
    void on_btnTest_clicked();
    void on_btnStartKinect_clicked();

    void on_btnQuit_clicked();

private:
    void searchMinAndMaxDepth();
    static void testSegmentation();

private:
    dai::PlaybackControl* m_playback;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
