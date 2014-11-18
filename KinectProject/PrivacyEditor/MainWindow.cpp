#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QGraphicsPixmapItem>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_ui->splitter->setStretchFactor(0, 8);
    m_ui->splitter->setStretchFactor(1, 3);
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

        QImage image(filePath);

        if(image.isNull()) {
            QMessageBox::information(this, "Privacy Editor","Error Displaying image");
            return;
        }

        QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        m_scene.clear();
        m_scene.addItem(item);
    }
}

MainWindow::~MainWindow()
{
    delete m_ui;
}
