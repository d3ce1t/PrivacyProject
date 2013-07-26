#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "DatasetBrowser.h"

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
    void on_btnOpenDataSets_clicked();
    void on_btnParseDataset_clicked();
    void on_btnTest_clicked();
    void on_btnStartKinect_clicked();

private:
    void searchMinAndMaxDepth();
    static void testSegmentation();

private:
    Ui::MainWindow *ui;
    DatasetBrowser m_browser;
};

#endif // MAINWINDOW_H
