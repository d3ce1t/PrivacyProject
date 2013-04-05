#include "basicviewer.h"
#include "ui_basicviewer.h"

BasicViewer::BasicViewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BasicViewer)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readNextFrame()));
}

BasicViewer::~BasicViewer()
{
    delete ui;
}

void BasicViewer::play(dai::MSRDailyActivity3DInstance* handler)
{
    m_handler = handler;
    m_handler->open();
    timer->start(100);
}

void BasicViewer::readNextFrame()
{
    if (m_handler->hasNext())
    {
        dai::DepthFrame* frame = m_handler->nextFrame();
        ui->viewer->setDepthMap(*frame);
        ui->viewer->update();
        delete frame;
    } else {
        timer->stop();
        m_handler->close();
    }
}
