#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    Ui::MainWindow* m_ui;
    QFileSystemModel m_fs_model;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_bg_item;
    QImage m_current_image;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void load_current_image();

private slots:

    void first_setup();




};

#endif // MAINWINDOW_H
