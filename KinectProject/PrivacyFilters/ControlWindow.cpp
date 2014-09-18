#include "ControlWindow.h"
#include "ui_ControlWindow.h"
#include "filters/PrivacyFilter.h"

ControlWindow::ControlWindow(dai::PrivacyFilter *privacy, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ControlWindow)
{
    ui->setupUi(this);
    m_privacy = privacy;
}

ControlWindow::~ControlWindow()
{
    delete ui;
}

void ControlWindow::on_btnNoPrivacy_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_DISABLED);
}

void ControlWindow::on_btnBlurring_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_BLUR);
}

void ControlWindow::on_btnPixelating_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_PIXELATION);
}

void ControlWindow::on_btnEmboss_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_EMBOSS);
}

void ControlWindow::on_btnSilhouette_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_SILHOUETTE);
}

void ControlWindow::on_btnSkeleton_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_SKELETON);
}

void ControlWindow::on_btnAvatar_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_3DMODEL);
}

void ControlWindow::on_btnInvisibility_clicked()
{
    m_privacy->enableFilter(dai::ColorFilter::FILTER_INVISIBILITY);
}
