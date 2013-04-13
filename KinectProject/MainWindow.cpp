#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "viewer/InstanceViewer.h"
#include "dataset/Dataset.h"
#include <QDebug>
#include <QVector>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_browser = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    /*Viewer* mainWindow = new Viewer();
    mainWindow->setResizeMode( QQuickView::SizeRootObjectToView);
    mainWindow->setSource(QUrl("qrc:///scenegraph/openglunderqml/main.qml"));
    mainWindow->show();*/
}

void MainWindow::on_pushButton_2_clicked()
{
    if (m_browser == NULL) {
        m_browser = new DatasetBrowser(this);
    }

    //m_browser->setWindowModality(Qt::WindowModal);
    m_browser->show();
    m_browser->activateWindow();
    this->hide();
}
