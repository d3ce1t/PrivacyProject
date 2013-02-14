#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include "dataset/MSR3Action3D.h"
#include "dataset/MSRDailyAct3D.h"
#include <QListWidgetItem>

using namespace DataSet;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void show_open_dialog();
    void readNextFrame();
    
private slots:
    void on_listDataSetFiles_itemActivated(QListWidgetItem *item);

private:
    QString currentDataSetDir;
    Ui::MainWindow *ui;
    QTimer* timer;
    //MSR3Action3D ds;
    MSRDailyAct3D ds;
};

#endif // MAINWINDOW_H
