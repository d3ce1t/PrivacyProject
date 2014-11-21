#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QSize>
#include <QPointF>
#include <QPen>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum DrawStatus {
        NO_DRAW,
        READY_TO_DRAW,
        DRAWING,
        END_DRAWING
    };

    Ui::MainWindow* m_ui;
    QFileSystemModel m_fs_model;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_bg_item;
    QImage m_current_image;
    const QSize MAX_IMAGE_SIZE = {600, 800};
    DrawStatus m_draw_status = NO_DRAW;
    QPointF m_last_pixel;
    QPen m_pen;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void load_selected_image();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:

    void first_setup();
    bool exceedSize(const QImage& image) const;
    void scaleImage(QImage &image) const;
    void updateView();

};

#endif // MAINWINDOW_H
