#include "MSRDailyColorInstance.h"
#include "dataset/DatasetMetadata.h"
#include <QThread>

namespace dai {

MSRDailyColorInstance::MSRDailyColorInstance(const InstanceInfo &info)
    : DataInstance(info), m_readBuffer(640, 480)
{
    m_frameBuffer[0].reset(new ColorFrame(640, 480));
    m_frameBuffer[1].reset(new ColorFrame(640, 480));
    initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
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
    QString instancePath = m_info.parent().getPath() + "/" + m_info.getFileName();
    m_player.setMedia(QUrl::fromLocalFile(instancePath));
    //m_player.setPlaybackRate(0.33);
    m_player.setVideoOutput(this);
    m_player.play();
    return true;
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

void MSRDailyColorInstance::nextFrame(ColorFrame &frame)
{
    // Wait for new frame
    waitForNewFrame();

    // Read this frame
    QMutexLocker locker(&m_lockFrame);
    frame = m_readBuffer;
}

bool MSRDailyColorInstance::present(const QVideoFrame &frame)
{
    m_lockFrame.lock();

    QVideoFrame* readFrame = const_cast<QVideoFrame*>(&frame);

    if (readFrame->isValid())
    {
        readFrame->map(QAbstractVideoBuffer::ReadOnly);

        if (readFrame->isMapped()) {
            memcpy( (void*) m_readBuffer.getDataPtr(), readFrame->bits(), readFrame->mappedBytes());
            readFrame->unmap();
        }
    }

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
