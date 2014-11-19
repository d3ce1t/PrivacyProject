#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>

#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 8);
    m_ui->splitter->setStretchFactor(1, 3);

    // Setup scene
    m_bg_item = new QGraphicsPixmapItem;
    m_scene.addItem(m_bg_item);
    m_ui->graphicsView->setScene(&m_scene);

    // Open Folder (setup model)
    connect(m_ui->actionOpen_folder, &QAction::triggered, [=]() {

        QString dirName = QFileDialog::getExistingDirectory(this,
                                                            tr("Open image folder"),
                                                            QDir::currentPath());
        if (!dirName.isEmpty()) {
            m_fs_model.setRootPath(dirName);
            m_fs_model.setFilter(QDir::Files);
        }
    });

    // Fit image (scale down)
    connect(m_ui->actionFit_image_to_screen, &QAction::triggered, [=]() {

        // Scale Image if needed
        if (m_current_image.width() > m_ui->graphicsView->viewport()->width() || m_current_image.height() > m_ui->graphicsView->viewport()->height()) {
            m_current_image = m_current_image.scaled(m_ui->graphicsView->viewport()->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_bg_item->setPixmap(QPixmap::fromImage(m_current_image));
            //m_ui->graphicsView->centerOn(m_bg_item);
            qDebug() << "Image has been scaled";
        }
    });

    // FS Model loaded (setup view)
    connect(&m_fs_model, &QFileSystemModel::directoryLoaded, [=](QString path) {
        qDebug() << "Directory" << path << "loaded";
        QModelIndex parentIndex = m_fs_model.index(m_fs_model.rootPath());
        m_ui->listView->setModel(&m_fs_model);
        m_ui->listView->setRootIndex(parentIndex);
        QTimer::singleShot(50, this, SLOT(first_setup())); // WORKAROUND to select the first child of the model
    });

    // Button Next
    connect(m_ui->btnNext, &QPushButton::clicked, [=]() {

        QModelIndex index = m_ui->listView->currentIndex();

        if (index.isValid()) {
            QString filePath = m_fs_model.filePath(index);
            qDebug() << filePath;
        }
    });

    // Button Prev
    connect(m_ui->btnPrev, &QPushButton::clicked, [=]() {

    });

    // Button Reload
    connect(m_ui->btnReload, &QPushButton::clicked, [=]() {
        load_current_image();
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
    load_current_image();
}

void MainWindow::load_current_image()
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

        m_bg_item->setPixmap(QPixmap::fromImage(m_current_image));
        qDebug() << m_bg_item->pos() << m_bg_item->boundingRect() << m_scene.sceneRect();

        //m_ui->graphicsView->centerOn(m_bg_item);
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}
