#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <opencv2/opencv.hpp>

Display::Display()
{
    //m_pen = {Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    m_bg_item = m_scene.addPixmap(QPixmap());
}

QGraphicsScene* Display::scene()
{
    return &m_scene;
}

int Display::imageWidth() const
{
    return m_bg_item->pixmap().width();
}

int Display::imageHeight() const
{
    return m_bg_item->pixmap().height();
}

void Display::setImage(const QImage& image)
{
    m_bg_item->setPixmap(QPixmap::fromImage(image));
    m_scene.setSceneRect(m_bg_item->boundingRect());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);

    // Setup scene
    m_pen = {Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    m_mask_item = m_input.scene()->addPath(QPainterPath(), m_pen);
    m_input.scene()->installEventFilter(this);
    m_ui->graphicsView->setScene(m_input.scene());
    m_privacy.addListener(this);

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
        //if (exceedSize(m_current_image)) {
            scaleImage(m_current_image);
            m_input.setImage(m_current_image);
        //}
    });

    // Action: Finish selection
    connect(m_ui->actionFinish_selection, &QAction::triggered, [=]() {

        if (m_current_image.isNull())
            return;

        QBrush brush(Qt::DiagCrossPattern);
        QPainterPath pp = m_mask_item->path();
        pp.closeSubpath();
        m_mask_item->setPath(pp);
        m_mask_item->setBrush(brush);

        dai::MaskFramePtr mask = create_mask(pp);
        dai::ColorFramePtr color = make_shared<dai::ColorFrame>(m_current_image.width(), m_current_image.height());
        dai::PrivacyFilter::convertQImage2ColorFrame(m_current_image, color);

        dai::QHashDataFrames frames;
        frames.insert(dai::DataFrame::Color, color);
        frames.insert(dai::DataFrame::Mask, mask);

        m_privacy.enableFilter(dai::ColorFilter(m_ui->comboFilter->currentIndex()));
        m_privacy.singleFrame(frames, color->width(), color->height());
    });

    // Action: Clear selection
    connect(m_ui->actionClearh_selection, &QAction::triggered, [=]() {
       m_mask_item->setPath(QPainterPath());
    });

    // Action: Print info
    connect(m_ui->actionPrint_Scene_Info, &QAction::triggered, [=]() {
        QPainterPath pp = m_mask_item->path();
        qDebug() << "Number of Items:" << m_input.scene()->items().size();
        qDebug() << "Path Lenght:" << pp.length();
    });

    // Action: Select Display
    connect(m_ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        // Input Image
        if (index == 0) {
            m_ui->graphicsView->setScene(m_input.scene());
        }
        // Output Image
        else if (index == 1) {
            m_ui->graphicsView->setScene(m_output.scene());
        }
    });

    // Button: Next
    connect(m_ui->btnNext, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QModelIndex sibling = index.sibling(index.row()+1, 0);

            if (sibling.isValid()) {
                m_ui->listView->setCurrentIndex(sibling);
                load_selected_image();
                m_ui->comboBox->setCurrentIndex(0);
            }
        }
    });

    // Button: Prev
    connect(m_ui->btnPrev, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QModelIndex sibling = index.sibling(index.row()-1, 0);

            if (sibling.isValid()) {
                m_ui->listView->setCurrentIndex(sibling);
                load_selected_image();
                m_ui->comboBox->setCurrentIndex(0);
            }
        }
    });

    // Button: Reload
    connect(m_ui->btnReload, &QPushButton::clicked, [=]() {
        load_selected_image();
    });

    // List View: Activate
    connect(m_ui->listView, &QListView::activated, [=]() {
        load_selected_image();
        m_ui->comboBox->setCurrentIndex(0);
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

        qDebug() << "Current Image Size" << m_current_image.size() << float(m_current_image.width()) / float(m_current_image.height()) <<
                    std::sqrt(m_current_image.width()) << std::sqrt(m_current_image.height());

        scaleImage(m_current_image);
        //m_input.setImage(m_current_image);
        m_input.setImage(m_current_image);
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
    image = image.scaled(MAX_IMAGE_SIZE, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QString str1 = image.width() % 2 == 0 ? "width mod 8" : "";
    QString str2 = image.height() % 2 == 0 ? "height mod 8" : "";
    qDebug() << "Image has been scaled -> New Size" << image.width() << image.height() << str1 << str2;
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
    shared_ptr<dai::MaskFrame> mask = std::make_shared<dai::MaskFrame>(m_input.imageWidth(), m_input.imageHeight());
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

void MainWindow::setOutputImage(QImage image)
{
    m_output.setImage(image);
    m_ui->comboBox->setCurrentIndex(1);
}

// It's called from the notifier thread
void MainWindow::newFrames(const dai::QHashDataFrames dataFrames)
{
    dai::ColorFramePtr color = static_pointer_cast<dai::ColorFrame>(dataFrames.value(dai::DataFrame::Color));
    QImage output_image((uchar*)color->getDataPtr(), color->width(), color->height(), color->getStride(), QImage::Format_RGB888);
    QMetaObject::invokeMethod(this, "setOutputImage", Q_ARG(QImage, output_image));
    //cv::Mat color_mat(color->height(), color->width(), CV_8UC3, (void*) color->getDataPtr(), color->getStride());
    //cv::imshow("Image Me", color_mat);
    //qDebug() << "Stride!" << color->getStride();
    //cv::waitKey(0);
}
