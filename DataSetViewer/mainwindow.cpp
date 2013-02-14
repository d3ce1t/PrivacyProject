#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <limits>
#include <QPainter>
#include <QApplication>
#include <QThread>
#include <QTimer>
#include "dataset/MSR3Action3D.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(ui->actionOpen_DataSet, SIGNAL(triggered()), this, SLOT(show_open_dialog()));
    connect(timer, SIGNAL(timeout()), this, SLOT(readNextFrame()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readNextFrame()
{
    if (ds.hasNext())
    {
        Sample* sample = ds.next();

        int nNonZeroPoints = sample->NumberOfNonZeroPoints();
        float avg = sample->AvgNonZeroDepth();

        ui->txtConsole->append(QString("frame[%1], ncols=%2, nrows=%3, count=%4, avg=%5, max=%6, min=%7").arg(QString::number(sample->getFrameIndex()),
                                                                                              QString::number(sample->GetNCols()),
                                                                                              QString::number(sample->GetNRows()),
                                                                                              QString::number(nNonZeroPoints),
                                                                                              QString::number(avg),
                                                                                              QString::number(sample->MaxValue()),
                                                                                              QString::number(sample->MinValue())));
        ui->viewer->setDepthMap(*sample);
        ui->viewer->update();
        delete sample;
    } else {
        timer->stop();
        ds.close();
    }
}

void MainWindow::show_open_dialog()
{
    currentDataSetDir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "");

    ui->txtConsole->append("DataSet directory is " + currentDataSetDir);

    QDir currentDir(currentDataSetDir);
    currentDir.setFilter(QDir::Files);

    QStringList entries = currentDir.entryList();

    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        QString dirname=*entry;
        ui->listDataSetFiles->addItem(dirname);
    }
}

void MainWindow::on_listDataSetFiles_itemActivated(QListWidgetItem *item)
{
    ds.open((currentDataSetDir + QDir::separator() + item->text()).toStdString());
    timer->start(100);
}
