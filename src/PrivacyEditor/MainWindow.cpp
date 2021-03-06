#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include <QStack>

const QSize MainWindow::MAX_IMAGE_SIZE = {512, 512};

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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
    , m_skeleton_root(nullptr)
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
                m_output_path = path;
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

    // Action: Load skeleton
    connect(m_ui->actionLoad_skeleton, &QAction::triggered, [=]() {

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        m_fs_model.rootPath(),
                                                        tr("Skeleton Binary (*.bin)"));

        dai::SkeletonPtr skeleton = load_skeleton(fileName);

        if (skeleton) {
            setup_skeleton(skeleton);
            m_ui->actionJointDrawingMode->trigger();
        }
    });

    // Action: Save skeleton
    connect(m_ui->actionSave_skeleton, &QAction::triggered, [=]() {

        dai::SkeletonFramePtr skeletonFrame = create_skeleton_from_scene();

        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"),
                                                        m_fs_model.rootPath(),
                                                        tr("Skeleton Binary (*.bin)"));

        if (!filePath.isEmpty()) {
            QFile skeletonFile(filePath);
            QByteArray data = skeletonFrame->toBinary();
            skeletonFile.open(QIODevice::WriteOnly);
            skeletonFile.write(data);
            skeletonFile.close();
        }
    });

    // Action: Save output
    connect(m_ui->actionSave_output, &QAction::triggered, [=]() {

        QString filePath = QFileDialog::getSaveFileName(this,
                                                        tr("Save File"),
                                                        m_output_path,
                                                        tr("PNG images (*.png)"));
        if (!filePath.isEmpty()) {
            const QImage& output_image = m_output.image();
            output_image.save(filePath);
            int end = filePath.lastIndexOf("/");
            m_output_path = filePath.mid(0, end);
        }
    });

    // Action: Apply filter
    connect(m_ui->actionApply_filter, &QAction::triggered, [=]() {

        if (m_input.image().isNull() || m_mask.image().isNull() || m_background.image().isNull())
            return;

        // Scale all images before sending it to our framework
        QImage maskTmp = m_mask.image();
        QImage colorTmp = m_input.image();
        QImage bgTmp = m_background.image();

        //scaleImage(maskTmp, QSize(640, 640), Qt::KeepAspectRatio);
        //scaleImage(colorTmp, QSize(640, 640), Qt::KeepAspectRatio);
        //scaleImage(bgTmp, QSize(640, 640), Qt::KeepAspectRatio);

        //qDebug() << "Image Size" << colorTmp.size();

        // Convert to our structures
        dai::MaskFramePtr empty_mask = make_shared<dai::MaskFrame>(colorTmp.width(), colorTmp.height());
        dai::MaskFramePtr mask = create_mask(maskTmp);
        dai::ColorFramePtr color = make_shared<dai::ColorFrame>(colorTmp.width(), colorTmp.height());
        dai::ColorFramePtr bg = make_shared<dai::ColorFrame>(bgTmp.width(), bgTmp.height());
        dai::PrivacyFilter::convertQImage2ColorFrame(colorTmp, color);
        dai::PrivacyFilter::convertQImage2ColorFrame(bgTmp, bg);
        dai::SkeletonFramePtr skeleton = create_skeleton_from_scene();

        // Patch to have a proper background inside PrivacyFilter class
        m_privacy.enableFilter(dai::ColorFilter(m_ui->comboFilter->currentIndex()));

        dai::QHashDataFrames frames;
        frames.insert(dai::DataFrame::Color, bg);
        frames.insert(dai::DataFrame::Mask, empty_mask);
        m_privacy.singleFrame(frames, color->width(), color->height()); // Send BG frame with no mask

        frames.insert(dai::DataFrame::Color, color);
        frames.insert(dai::DataFrame::Mask, mask);
        frames.insert(dai::DataFrame::Skeleton, skeleton);
        m_privacy.singleFrame(frames, color->width(), color->height()); // Send FG frame with mask
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
       m_input.scene()->removeItem(m_skeleton_root);
       m_skeleton_root = nullptr;
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
    connect(m_ui->actionJointDrawingMode, &QAction::triggered, [=](bool checked) {

        if (!checked) {
            m_ui->actionJointDrawingMode->setChecked(true);
        }

        m_ui->actionSilhouetteMode->setChecked(false);
        if (!m_skeleton_root) setup_skeleton();
        m_skeleton_root->setVisible(true);
    });

    // Action: Silhouette Mode
    connect(m_ui->actionSilhouetteMode, &QAction::triggered, [=]() {
        m_ui->actionJointDrawingMode->setChecked(false);
        if (m_skeleton_root) m_skeleton_root->setVisible(false);
    });

    // Button: Next
    connect(m_ui->btnNext, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QModelIndex sibling = index.sibling(index.row()+1, 0);

            if (sibling.isValid()) {
                m_ui->listView->setCurrentIndex(sibling);
                load_selected_image();
                if (m_ui->comboDisplaySelection->currentIndex() == 3)
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
                if (m_ui->comboDisplaySelection->currentIndex() == 3)
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
        if (m_ui->comboDisplaySelection->currentIndex() == 3)
            m_ui->comboDisplaySelection->setCurrentIndex(0);
    });

    // Quit
    connect(m_ui->actionQuit, &QAction::triggered, [=]() {
       QApplication::quit();
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

        QImage image, mask, bg;
        QString tmpPath = m_current_image_path;

        // Load images and skeleton
        image.load(tmpPath);
        mask.load(tmpPath.replace("_real.png", "_mask.png"));
        bg.load(tmpPath.replace("_mask.png", "_bg.png"));
        dai::SkeletonPtr skeleton = load_skeleton(tmpPath.replace("_bg.png", ".bin"));

        if(image.isNull() || mask.isNull() || bg.isNull()) {
            QMessageBox::information(this, "Privacy Editor","Error Displaying image");
            return;
        }

        qDebug() << "Current Image Size" << image.size() << float(image.width()) / float(image.height()) <<
                    std::sqrt(image.width()) << std::sqrt(image.height());

        // Scale images
        scaleImage(image);
        scaleImage(mask);
        scaleImage(bg);

        // Display Images
        m_input.setImage(image);
        m_mask.setImage(mask);
        m_background.setImage(bg);

        // Display skeleton
        if (skeleton) {
            setup_skeleton(skeleton);
            m_ui->actionJointDrawingMode->trigger();
        } else {
            setup_skeleton();
        }
    }
}

dai::SkeletonPtr MainWindow::load_skeleton(QString fileName) const
{
    dai::SkeletonPtr skeleton = nullptr;
    QFile skeletonFile(fileName);

    if (!skeletonFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Skeleton file does not exist";
        return nullptr;
    }

    qDebug() << "Loading skeleton" << fileName;

    QByteArray data = skeletonFile.readAll();
    skeletonFile.close();

    if (!data.isEmpty()) {
        dai::SkeletonFramePtr skeletonFrame = dai::SkeletonFrame::fromBinary(data);
        skeleton = skeletonFrame->getSkeleton(1);
    }

    return skeleton;
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

void MainWindow::scaleImage(QImage& image, const QSize size, Qt::AspectRatioMode aspectMode) const
{
    image = image.scaled(size, aspectMode, Qt::SmoothTransformation);
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

dai::MaskFramePtr MainWindow::create_mask(const QPainterPath& path)
{
    dai::MaskFramePtr mask = make_shared<dai::MaskFrame>(m_input.imageWidth(), m_input.imageHeight());
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

dai::MaskFramePtr MainWindow::create_mask(const QImage& image)
{
    Q_ASSERT(!image.isNull());

    dai::MaskFramePtr mask = make_shared<dai::MaskFrame>(image.width(), image.height());
    cv::Mat mat(image.height(), image.width(), CV_8UC4, (uchar*) image.constBits(), image.bytesPerLine());

    for (int i=0; i<mat.rows; ++i)
    {
        cv::Vec4b* in_pixel = mat.ptr<cv::Vec4b>(i);
        uint8_t* out_pixel = mask->getRowPtr(i);

        for (int j=0; j<mat.cols; ++j)
        {
            if (in_pixel[j][2] > 0) // Red channel
                out_pixel[j] = 1;
        }
    }

    return mask;
}

// It's called from the notifier thread
void MainWindow::newFrames(const dai::QHashDataFrames dataFrames)
{
    dai::ColorFramePtr color = static_pointer_cast<dai::ColorFrame>(dataFrames.value(dai::DataFrame::Color));
    QImage output_image((uchar*)color->getDataPtr(), color->width(), color->height(), color->getStride(), QImage::Format_RGB888);
    scaleImage(output_image, QSize(512, 512), Qt::KeepAspectRatio);
    QMetaObject::invokeMethod(this, "setOutputImage", Q_ARG(QImage, output_image));
}

void MainWindow::setOutputImage(QImage image)
{
    m_output.setImage(image);
    m_ui->comboDisplaySelection->setCurrentIndex(3);
}

void MainWindow::setup_skeleton(dai::SkeletonPtr skeleton)
{
    MapData coords[15] = {
        160.0f, 10.0f, dai::SkeletonJoint::JOINT_HEAD, -1,          // 0) Head
        0.0f, 100.0f, dai::SkeletonJoint::JOINT_CENTER_SHOULDER, 0, // 1) Center Shoulder
        -100.0f, 5.0f, dai::SkeletonJoint::JOINT_LEFT_SHOULDER, 1,  // 2) Left Shoulder
        -10.0f, 100.0f, dai::SkeletonJoint::JOINT_LEFT_ELBOW, 2,    // 3) Left Elbow
        -10.0f, 100.0f, dai::SkeletonJoint::JOINT_LEFT_HAND, 3,     // 4) Left Hand
        100.0f, 5.0f, dai::SkeletonJoint::JOINT_RIGHT_SHOULDER, 1,  // 5) Right Shoulder
        10.0f, 100.0f, dai::SkeletonJoint::JOINT_RIGHT_ELBOW, 5,    // 6) Right Elbow
        10.0f, 100.0f, dai::SkeletonJoint::JOINT_RIGHT_HAND, 6,     // 7) Right Hand
        0.0f, 150.0f, dai::SkeletonJoint::JOINT_SPINE, 1,           // 8) Center HIp
        -50.0f, 30.0f, dai::SkeletonJoint::JOINT_LEFT_HIP, 8,       // 9) Left Hip
        0.0f, 150.0f, dai::SkeletonJoint::JOINT_LEFT_KNEE, 9,       // 10) Left Knee
        0.0f, 150.0f, dai::SkeletonJoint::JOINT_LEFT_FOOT, 10,      // 11) Left Foot
        50.0f, 30.0f, dai::SkeletonJoint::JOINT_RIGHT_HIP, 8,       // 12) Right Hip
        0.0f, 150.0f, dai::SkeletonJoint::JOINT_RIGHT_KNEE, 12,     // 13) Right Knee
        0.0f, 150.0f, dai::SkeletonJoint::JOINT_RIGHT_FOOT, 13,     // 14) Right Foot
    };

    if (skeleton) {
        MapData coords_tmp[15];
        convertJointToMapData(coords_tmp[0], skeleton->getJoint(dai::SkeletonJoint::JOINT_HEAD), -1, nullptr);
        convertJointToMapData(coords_tmp[1], skeleton->getJoint(dai::SkeletonJoint::JOINT_CENTER_SHOULDER), 0, coords_tmp);
        convertJointToMapData(coords_tmp[2], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_SHOULDER), 1, coords_tmp);
        convertJointToMapData(coords_tmp[3], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_ELBOW), 2, coords_tmp);
        convertJointToMapData(coords_tmp[4], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_HAND), 3, coords_tmp);
        convertJointToMapData(coords_tmp[5], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_SHOULDER), 1, coords_tmp);
        convertJointToMapData(coords_tmp[6], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_ELBOW), 5, coords_tmp);
        convertJointToMapData(coords_tmp[7], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_HAND), 6, coords_tmp);
        convertJointToMapData(coords_tmp[8], skeleton->getJoint(dai::SkeletonJoint::JOINT_SPINE), 1, coords_tmp);
        convertJointToMapData(coords_tmp[9], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_HIP), 8, coords_tmp);
        convertJointToMapData(coords_tmp[10], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_KNEE), 9, coords_tmp);
        convertJointToMapData(coords_tmp[11], skeleton->getJoint(dai::SkeletonJoint::JOINT_LEFT_FOOT), 10, coords_tmp);
        convertJointToMapData(coords_tmp[12], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_HIP), 8, coords_tmp);
        convertJointToMapData(coords_tmp[13], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_KNEE), 12, coords_tmp);
        convertJointToMapData(coords_tmp[14], skeleton->getJoint(dai::SkeletonJoint::JOINT_RIGHT_FOOT), 13, coords_tmp);
        memcpy(coords, coords_tmp, sizeof(coords_tmp));
    }

    if (m_skeleton_root) {
        m_input.scene()->removeItem(m_skeleton_root);
        m_skeleton_root = nullptr;
    }

    // Create skeleton
    QGraphicsItem* items[15];

    for (int i=0; i<15; ++i)
    {
        QGraphicsItem* parent = nullptr;

        if (coords[i].parent != -1) {
            parent = items[coords[i].parent];
        }

        items[i] = addJoint(coords[i].x, coords[i].y, coords[i].joint, parent);
    }

    m_skeleton_root = items[0];
    m_skeleton_root->setVisible(false);
}

void MainWindow::convertJointToMapData(MapData& data, const dai::SkeletonJoint& joint, int parent, MapData* coords)
{
    QStack<int> stack;
    int tmp_parent = parent;

    while (tmp_parent != -1) {
        stack.push(tmp_parent);
        tmp_parent = coords[tmp_parent].parent;
    }

    // Coordinates are relative to parent
    float pos_x = joint.getPosition()[0];
    float pos_y = joint.getPosition()[1];

    while (!stack.isEmpty()) {
        tmp_parent = stack.pop();
        pos_x -= coords[tmp_parent].x;
        pos_y -= coords[tmp_parent].y;
    }

    data.joint = joint.getType();
    data.parent = parent;
    data.x = pos_x;
    data.y = pos_y;
}

dai::SkeletonFramePtr MainWindow::create_skeleton_from_scene()
{
    dai::SkeletonFramePtr skeletonFrame = make_shared<dai::SkeletonFrame>(m_input.imageWidth(), m_input.imageHeight());
    dai::SkeletonPtr skeleton = make_shared<dai::Skeleton>(dai::Skeleton::SKELETON_OPENNI);
    skeletonFrame->setSkeleton(1, skeleton);
    skeleton->setDistanceUnits(dai::DISTANCE_PIXELS);

    for (QGraphicsItem* item : m_input.scene()->items()) {

        if (item->type() == 3 && item->data(0).isValid()) {

            dai::SkeletonJoint::JointType jointType = dai::SkeletonJoint::JointType(item->data(0).toInt());

            float w_x = float(item->scenePos().x());
            float w_y = float(item->scenePos().y());
            float w_z = 0.0f;
            //dai::Skeleton::convertDepthCoordinatesToJoint(item->scenePos().x(), item->scenePos().y(), w_z, &w_x, &w_y);

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

    return item;
}
