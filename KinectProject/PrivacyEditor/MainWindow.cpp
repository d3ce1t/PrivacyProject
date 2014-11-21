#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    //m_ui->splitter->setStretchFactor(0, 8);
    //m_ui->splitter->setStretchFactor(1, 3);

    // Setup scene
    m_pen = {Qt::green, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin};
    m_bg_item = new QGraphicsPixmapItem;
    m_scene.addItem(m_bg_item);
    m_scene.installEventFilter(this);
    m_ui->graphicsView->setScene(&m_scene);

    // FS Model loaded (setup view)
    connect(&m_fs_model, &QFileSystemModel::directoryLoaded, [=](QString path) {
        qDebug() << "Directory" << path << "loaded";
        QModelIndex parentIndex = m_fs_model.index(m_fs_model.rootPath());
        m_ui->listView->setModel(&m_fs_model);
        m_ui->listView->setRootIndex(parentIndex);
        QTimer::singleShot(50, this, SLOT(first_setup())); // WORKAROUND to select the first child of the model
    });

    // Action: Open Folder (setup model)
    connect(m_ui->actionOpen_folder, &QAction::triggered, [=]() {

        QString dirName = QFileDialog::getExistingDirectory(this,
                                                            tr("Open image folder"),
                                                            QDir::currentPath());
        if (!dirName.isEmpty()) {
            m_fs_model.setRootPath(dirName);
            m_fs_model.setFilter(QDir::Files);
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

    // Action: Print info
    connect(m_ui->actionPrint_Scene_Info, &QAction::triggered, [=]() {

        qDebug() << "Number of Items:" << m_scene.items().size();

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

        QString filePath = m_fs_model.filePath(index);
        qDebug() << "Loading" << filePath;

        m_current_image.load(filePath);

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

    if (m_draw_status == NO_DRAW && event->type() == QEvent::GraphicsSceneMousePress) {
        m_last_pixel.setX(x);
        m_last_pixel.setY(y);
        m_draw_status = READY_TO_DRAW;
        qDebug() << "Last pixel" << m_last_pixel;
    }
    else if (m_draw_status == READY_TO_DRAW && event->type() == QEvent::GraphicsSceneMouseMove)
        m_draw_status = DRAWING;
    else if (m_draw_status == READY_TO_DRAW && event->type() == QEvent::GraphicsSceneMouseRelease)
        m_draw_status = END_DRAWING;
    else if (m_draw_status == DRAWING && event->type() == QEvent::GraphicsSceneMouseMove)
        m_draw_status = DRAWING;
    else if (m_draw_status == DRAWING && event->type() == QEvent::GraphicsSceneMouseRelease)
        m_draw_status = END_DRAWING;
    else if (m_draw_status == DRAWING && event->type() == QEvent::GraphicsSceneMousePress)
        m_draw_status = NO_DRAW;

    if (m_draw_status == DRAWING)
    {
        m_scene.addLine(m_last_pixel.x(), m_last_pixel.y(), x, y, m_pen);
        qDebug() << "Line" << m_last_pixel << "to" << x << y;
        m_last_pixel.setX(x);
        m_last_pixel.setY(y);
    }

    if (m_draw_status == READY_TO_DRAW || m_draw_status == END_DRAWING) {
        m_scene.addRect(x,y,1,1,m_pen, QBrush(Qt::SolidPattern));

        if (m_draw_status == END_DRAWING)
            m_draw_status = NO_DRAW;
    }

    return true;
    //return QObject::eventFilter(obj, event);
}
