#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPathItem>
#include <QSize>
#include <QPointF>
#include <QPen>
#include "types/MaskFrame.h"
#include "types/ColorFrame.h"
#include "filters/PrivacyFilter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, dai::FrameListener
{
    Q_OBJECT

    Ui::MainWindow* m_ui;
    QFileSystemModel m_fs_model;
    QGraphicsScene m_scene;
    QGraphicsPixmapItem* m_bg_item;
    QGraphicsPathItem* m_mask_item;
    QImage m_current_image;
    const QSize MAX_IMAGE_SIZE = {600, 800};
    QPointF m_last_pixel;
    QPen m_pen;
    QString m_current_image_path;
    dai::PrivacyFilter m_privacy;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void load_selected_image();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void newFrames(const dai::QHashDataFrames dataFrames);

private slots:

    void first_setup();
    bool exceedSize(const QImage& image) const;
    void scaleImage(QImage &image) const;
    void updateView();
    shared_ptr<dai::MaskFrame> create_mask(const QPainterPath& path);

};

#endif // MAINWINDOW_H
