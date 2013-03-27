#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "viewer.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    Viewer* mainWindow = new Viewer();
    mainWindow->setResizeMode( QQuickView::SizeRootObjectToView);
    mainWindow->setSource(QUrl("qrc:///scenegraph/openglunderqml/main.qml"));
    mainWindow->show();
}
