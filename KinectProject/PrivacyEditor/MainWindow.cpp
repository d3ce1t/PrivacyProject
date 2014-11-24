#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <opencv2/opencv.hpp>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    // Setup scene
    m_pen = {Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    m_bg_item = m_scene.addPixmap(QPixmap());
    m_mask_item = m_scene.addPath(QPainterPath(), m_pen);
    m_scene.installEventFilter(this);
    m_ui->graphicsView->setScene(&m_scene);

    // Action: Open Folder (setup model)
    connect(m_ui->actionOpen_folder, &QAction::triggered, [=]() {

        QString dirName = QFileDialog::getExistingDirectory(this,
                                                            tr("Open image folder"),
                                                            QDir::currentPath());
        if (!dirName.isEmpty()) {

            // FS Model loaded (setup view)
            connect(&m_fs_model, &QFileSystemModel::directoryLoaded, [=](QString path) {
                qDebug() << "Directory" << path << "loaded";
                QModelIndex parentIndex = m_fs_model.index(m_fs_model.rootPath());
                m_ui->listView->setModel(&m_fs_model);
                m_ui->listView->setRootIndex(parentIndex);
                QTimer::singleShot(50, this, SLOT(first_setup())); // WORKAROUND to select the first child of the model
                m_fs_model.disconnect();
            });

            QStringList filters = {"*.jpg", "*.jpeg", "*.bmp", "*.png"};
            m_fs_model.setRootPath(dirName);
            m_fs_model.setFilter(QDir::Files);
            m_fs_model.setNameFilters(filters);
        }
    });

    // Action: Fit image (scale down)
    connect(m_ui->actionFit_image_to_screen, &QAction::triggered, [=]() {

        // Scale Image if needed
        if (exceedSize(m_current_image)) {
            scaleImage(m_current_image);
            updateView();
            qDebug() << "Image has been scaled";
        }
    });

    // Action: Finish selection
    connect(m_ui->actionFinish_selection, &QAction::triggered, [=]() {
        QBrush brush(Qt::DiagCrossPattern);
        QPainterPath pp = m_mask_item->path();
        pp.closeSubpath();
        m_mask_item->setPath(pp);
        m_mask_item->setBrush(brush);

        shared_ptr<dai::MaskFrame> mask = create_mask(pp);
        cv::Mat mat_mask(mask->height(), mask->width(), CV_8UC1, (void*) mask->getDataPtr(), mask->getStride());
        cv::Mat mat_color = cv::imread(m_current_image_path.toStdString());
        shared_ptr<dai::ColorFrame> color = make_shared<dai::ColorFrame>(mat_color.cols, mat_color.rows,
                                                                         (dai::RGBColor*) mat_color.data, mat_color.step);

        dai::QHashDataFrames frames;
        frames.insert(dai::DataFrame::Color, color);
        frames.insert(dai::DataFrame::Mask, mask);

        m_privacy.addListener(this);
        m_privacy.newFrames(frames);

        for (int i=0; i<mat_mask.rows; ++i)
        {
            uchar* mask_pixel = mat_mask.ptr<uchar>(i);
            cv::Vec3b* color_pixel = mat_color.ptr<cv::Vec3b>(i);

            for (int j=0; j<mat_mask.cols; ++j) {

                if (mask_pixel[j] > 0) {
                    color_pixel[j][0] = 255;
                }
            }
        }

        cv::imshow("image", mat_color);
        cv::waitKey();
    });

    // Action: Clear selection
    connect(m_ui->actionClearh_selection, &QAction::triggered, [=]() {
       m_mask_item->setPath(QPainterPath());
    });

    // Action: Print info
    connect(m_ui->actionPrint_Scene_Info, &QAction::triggered, [=]() {

        QPainterPath pp = m_mask_item->path();
        qDebug() << "Number of Items:" << m_scene.items().size();
        qDebug() << "Path Lenght:" << pp.length();

    });

    // Button: Next
    connect(m_ui->btnNext, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QString filePath = m_fs_model.filePath(index);
            qDebug() << filePath;
        }
    });

    // Button: Prev
    connect(m_ui->btnPrev, &QPushButton::clicked, [=]() {

    });

    // Button: Reload
    connect(m_ui->btnReload, &QPushButton::clicked, [=]() {
        load_selected_image();
    });

    // List View: Activate
    connect(m_ui->listView, &QListView::activated, [=]() {
        load_selected_image();
    });
}

void MainWindow::first_setup()
{
    // Select first item of the list
    QModelIndex index = m_fs_model.index(m_fs_model.rootPath());

    if (index.isValid()) {
        m_ui->listView->setCurrentIndex(index.child(0,0));
    }

    // Load image
    load_selected_image();
}

void MainWindow::load_selected_image()
{
    QModelIndex index = m_ui->listView->currentIndex();

    if (index.isValid()) {

        m_current_image_path = m_fs_model.filePath(index);
        qDebug() << "Loading" << m_current_image_path;

        m_current_image.load(m_current_image_path);

        if(m_current_image.isNull()) {
            QMessageBox::information(this, "Privacy Editor","Error Displaying image");
            return;
        }

        updateView();
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

bool MainWindow::exceedSize(const QImage& image) const
{
    bool result = false;

    if (image.width() > MAX_IMAGE_SIZE.width() || image.height() > MAX_IMAGE_SIZE.height())
        result = true;

    return result;
}

void MainWindow::scaleImage(QImage& image) const
{
    image = image.scaled(MAX_IMAGE_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::updateView()
{
    m_bg_item->setPixmap(QPixmap::fromImage(m_current_image));
    m_scene.setSceneRect(m_bg_item->boundingRect());
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Standard event processing
    if (event->type() != QEvent::GraphicsSceneMousePress && event->type() != QEvent::GraphicsSceneMouseMove
            && event->type() != QEvent::GraphicsSceneMouseRelease)
        return QObject::eventFilter(obj, event);

    // My own event handler
    QGraphicsSceneMouseEvent* paint_event = static_cast< QGraphicsSceneMouseEvent* >( event );
    qreal x = paint_event->scenePos().x();
    qreal y = paint_event->scenePos().y();

    if (event->type() == QEvent::GraphicsSceneMousePress)
    {
        QPainterPath pp = m_mask_item->path();

        if (pp.isEmpty()) {
            pp.moveTo(paint_event->scenePos());
            m_mask_item->setPath(pp);
            m_mask_item->setBrush(QBrush());
            m_last_pixel.setX(x);
            m_last_pixel.setY(y);
            qDebug() << "Last pixel" << m_last_pixel;
        } else {

        }
    }
    else if (event->type() == QEvent::GraphicsSceneMouseMove)
    {
        QPainterPath pp = m_mask_item->path();
        pp.lineTo(paint_event->scenePos());
        m_mask_item->setPath(pp);
        //m_scene.addLine(m_last_pixel.x(), m_last_pixel.y(), x, y, m_pen);
        //qDebug() << "Line" << m_last_pixel << "to" << x << y;
        //m_last_pixel.setX(x);
        //m_last_pixel.setY(y);
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease)
    {
        /*QPainterPath pp = m_mask_item->path();
        //pp.closeSubpath()
       // pp.lineTo(paint_event->scenePos());
        m_mask_item->setPath(pp);*/

    }

    return QObject::eventFilter(obj, event);
}

shared_ptr<dai::MaskFrame> MainWindow::create_mask(const QPainterPath& path)
{
    shared_ptr<dai::MaskFrame> mask = std::make_shared<dai::MaskFrame>(m_bg_item->pixmap().width(), m_bg_item->pixmap().height());
    QRectF search_region = path.boundingRect();

    for (int i = search_region.top(); i < search_region.bottom(); ++i)
    {
        for (int j= search_region.left(); j < search_region.right(); ++j)
        {
            if (path.contains(QPointF(j, i))) {
                mask->setItem(i, j, 1);
            }
        }
    }

    return mask;
}

void MainWindow::newFrames(const dai::QHashDataFrames dataFrames)
{
    qDebug() << "Debug!";
}
