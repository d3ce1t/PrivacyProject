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

QImage& Display::image()
{
    return m_image;
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
    m_image = image; // shallow copy
    update();
}

void Display::update()
{
    m_bg_item->setPixmap(QPixmap::fromImage(m_image));
    m_scene.setSceneRect(m_bg_item->boundingRect());
}

const QGraphicsPixmapItem* Display::background() const
{
    return m_bg_item;
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
    m_selected_joint = nullptr;

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

            QStringList filters = {"*_real.jpg", "*_real.jpeg", "*_real.bmp", "*_real.png"};
            m_fs_model.setRootPath(dirName);
            m_fs_model.setFilter(QDir::Files);
            m_fs_model.setNameFilters(filters);
            m_fs_model.setNameFilterDisables(false);
        }
    });

    // Action: Fit image (scale down)
    connect(m_ui->actionFit_image_to_screen, &QAction::triggered, [=]() {

        // Scale Image if needed
        //if (exceedSize(m_input.image())) {
            scaleImage(m_input.image());
            m_input.update();
        //}
    });

    // Action: Finish selection
    connect(m_ui->actionFinish_selection, &QAction::triggered, [=]() {

        if (m_input.image().isNull())
            return;

        QBrush brush(Qt::DiagCrossPattern);
        QPainterPath pp = m_mask_item->path();
        pp.closeSubpath();
        m_mask_item->setPath(pp);
        m_mask_item->setBrush(brush);

        dai::MaskFramePtr mask = create_mask(pp);
        dai::ColorFramePtr color = make_shared<dai::ColorFrame>(m_input.imageWidth(), m_input.imageHeight());
        dai::PrivacyFilter::convertQImage2ColorFrame(m_input.image(), color);
        dai::SkeletonFramePtr skeleton = create_skeleton_from_scene();

        dai::QHashDataFrames frames;
        frames.insert(dai::DataFrame::Color, color);
        frames.insert(dai::DataFrame::Mask, mask);
        frames.insert(dai::DataFrame::Skeleton, skeleton);

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

    // Select Display
    connect(m_ui->comboDisplaySelection, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        // Input Image
        if (index == 0) {
            m_ui->graphicsView->setScene(m_input.scene());
        }
        // Mask Image
        else if (index == 1) {
            m_ui->graphicsView->setScene(m_mask.scene());
        }
        // BG Image
        else if (index == 2) {
            m_ui->graphicsView->setScene(m_background.scene());
        }
        // Output Image
        else if (index == 3) {
            m_ui->graphicsView->setScene(m_output.scene());
        }
    });

    // Action: Skeleton Mode
    connect(m_ui->actionJointDrawingMode, &QAction::triggered, [=]() {
        m_ui->actionSilhouetteMode->setChecked(false);
        m_skeleton_root->setVisible(true);
    });

    // Action: Silhouette Mode
    connect(m_ui->actionSilhouetteMode, &QAction::triggered, [=]() {
        m_ui->actionJointDrawingMode->setChecked(false);
        m_skeleton_root->setVisible(false);
    });

    // Button: Next
    connect(m_ui->btnNext, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QModelIndex sibling = index.sibling(index.row()+1, 0);

            if (sibling.isValid()) {
                m_ui->listView->setCurrentIndex(sibling);
                load_selected_image();
                m_ui->comboDisplaySelection->setCurrentIndex(0);
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
                m_ui->comboDisplaySelection->setCurrentIndex(0);
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
        m_ui->comboDisplaySelection->setCurrentIndex(0);
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

    // Print Skeleton
    setup_skeleton();
}

void MainWindow::load_selected_image()
{
    QModelIndex index = m_ui->listView->currentIndex();

    if (index.isValid()) {

        m_current_image_path = m_fs_model.filePath(index);
        qDebug() << "Loading" << m_current_image_path;

        QImage image, mask, bg;
        QString tmpPath = m_current_image_path;

        image.load(tmpPath);
        mask.load(tmpPath.replace("_real.png", "_mask.png"));
        bg.load(tmpPath.replace("_mask.png", "_bg.png"));

        if(image.isNull()) {
            QMessageBox::information(this, "Privacy Editor","Error Displaying image");
            return;
        }

        qDebug() << "Current Image Size" << image.size() << float(image.width()) / float(image.height()) <<
                    std::sqrt(image.width()) << std::sqrt(image.height());

        scaleImage(image);
        scaleImage(mask);
        scaleImage(bg);
        m_input.setImage(image);
        m_mask.setImage(mask);
        m_background.setImage(bg);
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

    if (m_ui->actionSilhouetteMode->isChecked()) {
        eventSilhouette(event);
    } else {
        eventSkeleton(event);
    }

    return QObject::eventFilter(obj, event);
}

void MainWindow::eventSilhouette(QEvent *event)
{
    QGraphicsSceneMouseEvent* paint_event = static_cast<QGraphicsSceneMouseEvent*>( event );
    qreal x = paint_event->scenePos().x();
    qreal y = paint_event->scenePos().y();

    if (event->type() == QEvent::GraphicsSceneMousePress && paint_event->button() == Qt::LeftButton)
    {
        QPainterPath pp = m_mask_item->path();

        if (pp.isEmpty()) {
            pp.moveTo(paint_event->scenePos());
            m_mask_item->setPath(pp);
            m_mask_item->setBrush(QBrush());
            m_last_pixel.setX(x);
            m_last_pixel.setY(y);
            m_drawing = true;
            qDebug() << "Last pixel" << m_last_pixel;
        } else {

        }
    }
    else if (event->type() == QEvent::GraphicsSceneMouseMove && m_drawing)
    {
        QPainterPath pp = m_mask_item->path();
        pp.lineTo(paint_event->scenePos());
        m_mask_item->setPath(pp);
        //m_scene.addLine(m_last_pixel.x(), m_last_pixel.y(), x, y, m_pen);
        //qDebug() << "Line" << m_last_pixel << "to" << x << y;
        //m_last_pixel.setX(x);
        //m_last_pixel.setY(y);
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease && paint_event->button() == Qt::LeftButton)
    {
        m_drawing = false;
        /*QPainterPath pp = m_mask_item->path();
        //pp.closeSubpath()
       // pp.lineTo(paint_event->scenePos());
        m_mask_item->setPath(pp);*/
    }
}

void MainWindow::eventSkeleton(QEvent *event)
{
    QGraphicsSceneMouseEvent* paint_event = static_cast< QGraphicsSceneMouseEvent* >( event );

    qreal x = paint_event->scenePos().x();
    qreal y = paint_event->scenePos().y();

    if (event->type() == QEvent::GraphicsSceneMousePress && paint_event->button() == Qt::LeftButton) {

        QGraphicsItem* item = m_input.scene()->itemAt(x, y, QTransform());

        if (item != nullptr && item != m_input.background()) {
            m_selected_joint = static_cast<QGraphicsRectItem*>(item);
            m_selected_joint->setBrush({Qt::red});
            //m_selected_joint->setCursor(Qt::ClosedHandCursor);
        }
    }
    else if (event->type() == QEvent::GraphicsSceneMouseRelease && paint_event->button() == Qt::LeftButton) {

        if (m_selected_joint) {
            m_selected_joint->setBrush({Qt::green});
            //m_selected_joint->setCursor(Qt::OpenHandCursor);
            m_selected_joint = nullptr;
        }
    }
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
    m_ui->comboDisplaySelection->setCurrentIndex(1);
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

void MainWindow::setup_skeleton()
{
    QGraphicsItem *last_item, *center_shoulder, *center_hip;

    // Head
    m_skeleton_root = addJoint(160, 10, dai::SkeletonJoint::JOINT_HEAD);

    // Center Shoulder
    center_shoulder = addJoint(0, 100, dai::SkeletonJoint::JOINT_CENTER_SHOULDER, m_skeleton_root);

    // Left Shoulder
    last_item = addJoint(-100, 5, dai::SkeletonJoint::JOINT_LEFT_SHOULDER, center_shoulder);

    // Left Elbow
    last_item = addJoint(-10, 100, dai::SkeletonJoint::JOINT_LEFT_ELBOW, last_item);

    // Left Hand
    last_item = addJoint(-10, 100, dai::SkeletonJoint::JOINT_LEFT_HAND, last_item);

    // Right Shoulder
    last_item = addJoint(100, 5, dai::SkeletonJoint::JOINT_RIGHT_SHOULDER, center_shoulder);

    // Right Elbow
    last_item = addJoint(10, 100, dai::SkeletonJoint::JOINT_RIGHT_ELBOW, last_item);

    // Right Hand
    last_item = addJoint(10, 100, dai::SkeletonJoint::JOINT_RIGHT_HAND, last_item);

    // Center Hip
    center_hip = addJoint(0, 150, dai::SkeletonJoint::JOINT_SPINE, center_shoulder);

    // Left Hip
    last_item = addJoint(-50, 30, dai::SkeletonJoint::JOINT_LEFT_HIP, center_hip);

    // Left Knee
    last_item = addJoint(0, 150, dai::SkeletonJoint::JOINT_LEFT_KNEE, last_item);

    // Left Foot
    last_item = addJoint(0, 150, dai::SkeletonJoint::JOINT_LEFT_FOOT, last_item);

    // Right Hip
    last_item = addJoint(50, 30, dai::SkeletonJoint::JOINT_RIGHT_HIP, center_hip);

    // Right Knee
    last_item = addJoint(0, 150, dai::SkeletonJoint::JOINT_RIGHT_KNEE, last_item);

    // Right Foot
    last_item = addJoint(0, 150, dai::SkeletonJoint::JOINT_RIGHT_FOOT, last_item);

    m_skeleton_root->setVisible(false);
}

dai::SkeletonFramePtr MainWindow::create_skeleton_from_scene()
{
    dai::SkeletonFramePtr skeletonFrame = make_shared<dai::SkeletonFrame>();
    dai::SkeletonPtr skeleton = make_shared<dai::Skeleton>(dai::Skeleton::SKELETON_OPENNI);
    skeletonFrame->setSkeleton(1, skeleton);

    for (QGraphicsItem* item : m_input.scene()->items()) {

        if (item->type() == 3 && item->data(0).isValid()) {

            qDebug() << "Item.Pos:" << item->scenePos();

            dai::SkeletonJoint::JointType jointType = dai::SkeletonJoint::JointType(item->data(0).toInt());

            float w_x, w_y, w_z = 1.5f;
            dai::Skeleton::convertDepthCoordinatesToJoint(item->scenePos().x(), item->scenePos().y(), w_z, &w_x, &w_y);

            dai::SkeletonJoint joint(dai::Point3f(w_x, w_y, w_z), jointType);
            skeleton->setJoint(jointType, joint);
        }
    }

    return skeletonFrame;
}

QGraphicsItem* MainWindow::addJoint(int x, int y, dai::SkeletonJoint::JointType type, QGraphicsItem* parent)
{
    QGraphicsItem* item = m_input.scene()->addRect(0, 0, 9, 9,
                                   {Qt::blue, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin},
                                   {Qt::green});


    item->setPos(x, y);
    item->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    item->setCursor(Qt::OpenHandCursor);
    item->setParentItem(parent);
    item->setZValue(1.0);
    item->setData(0, QVariant(type));

    /*if (parent != nullptr) {
        qreal parent_x = parent->scenePos().x()+4;
        qreal parent_y = parent->scenePos().y()+4;
        qreal child_x = item->scenePos().x()+4;
        qreal child_y = item->scenePos().y()+4;
        qDebug() << parent->scenePos() << item->scenePos() << parent_x << parent_y << child_x << child_y;
        QGraphicsLineItem* line = m_input.scene()->addLine(parent_x, parent_y, child_x, child_y, {Qt::blue, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin});
        line->setParentItem(parent);
    }*/

    return item;
}
