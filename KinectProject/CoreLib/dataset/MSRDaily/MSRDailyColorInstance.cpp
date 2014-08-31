#include "MSRDailyColorInstance.h"
#include "dataset/DatasetMetadata.h"
#include <QThread>
#include <QFile>

namespace dai {

MSRDailyColorInstance::MSRDailyColorInstance(const InstanceInfo &info)
    : DataInstance(info, DataFrame::Color, 640, 480)
{
    m_width = 0;
    m_height = 0;
    m_newFrameGenerated = false;
}

MSRDailyColorInstance::~MSRDailyColorInstance()
{
    m_width = 0;
    m_height = 0;
    closeInstance();
}

bool MSRDailyColorInstance::is_open() const
{
    return m_player.mediaStatus() != QMediaPlayer::NoMedia && m_player.mediaStatus() != QMediaPlayer::InvalidMedia;
}

bool MSRDailyColorInstance::hasNext() const
{
    return m_player.state() == QMediaPlayer::PlayingState;
}

bool MSRDailyColorInstance::openInstance()
{
    bool result = false;
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName(DataFrame::Color);
    QFile file(instancePath);

    if (file.exists()) {
        m_player.setMedia(QUrl::fromLocalFile(instancePath));
        //m_player.setPlaybackRate(0.33);
        m_player.setVideoOutput(this);
        m_player.play();
        result = true;
    }

    return result;
}

void MSRDailyColorInstance::closeInstance()
{
    m_player.stop();
}

void MSRDailyColorInstance::restartInstance()
{
    m_player.stop();
    m_player.play();
    QThread::msleep(200);
}

void MSRDailyColorInstance::nextFrame(QHashDataFrames &output)
{
    Q_ASSERT(output.size() > 0);
    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));

    // Wait for new frame
    waitForNewFrame();

    // Read this frame
    QMutexLocker locker(&m_lockFrame);

    if (m_readFrame->isValid()) {
        m_readFrame->map(QAbstractVideoBuffer::ReadOnly);
        if (m_readFrame->isMapped()) {
            memcpy( (void*) colorFrame->getDataPtr(), m_readFrame->bits(), m_readFrame->mappedBytes());
            m_readFrame->unmap();
        }
    }
}

bool MSRDailyColorInstance::present(const QVideoFrame &frame)
{
    m_lockFrame.lock();
    m_readFrame = const_cast<QVideoFrame*>(&frame);
    m_lockFrame.unlock();
    notifyNewFrame();
    return true;
}

QList<QVideoFrame::PixelFormat> MSRDailyColorInstance::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);

    // Return the formats you will support
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB24;
}

void MSRDailyColorInstance::notifyNewFrame()
{
    m_lockSync.lock();
    if (!m_newFrameGenerated) {
        m_newFrameGenerated = true;
        m_sync.wakeOne();
    }
    m_lockSync.unlock();
}

void MSRDailyColorInstance::waitForNewFrame()
{
    m_lockSync.lock();
    while (!m_newFrameGenerated) {
        m_sync.wait(&m_lockSync);
    }
    m_newFrameGenerated = false;
    m_lockSync.unlock();
}

} // End namespace
