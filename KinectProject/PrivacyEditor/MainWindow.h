#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow* m_ui;
    QFileSystemModel m_fs_model;
    QGraphicsScene m_scene;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void load_current_image();

private slots:

    void first_setup();




};

#endif // MAINWINDOW_H
