#ifndef MSRDAILYCOLORINSTANCE_H
#define MSRDAILYCOLORINSTANCE_H

#include <fstream>
#include "dataset/DataInstance.h"
#include "types/ColorFrame.h"
#include <QMediaPlayer>
#include <QAbstractVideoSurface>
#include <QMutex>
#include <QWaitCondition>

namespace dai {

class MSRDailyColorInstance : public DataInstance,
                              public QAbstractVideoSurface
{
public:
    explicit MSRDailyColorInstance(const InstanceInfo& info);
    virtual ~MSRDailyColorInstance();
    bool is_open() const override;
    bool hasNext() const override;
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override;
    bool present(const QVideoFrame &frame) override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;

private:
    void notifyNewFrame();
    void waitForNewFrame();


    QMediaPlayer            m_player;
    int                     m_width;
    int                     m_height;
    QVideoFrame*            m_readFrame;
    QMutex                  m_lockFrame;
    QMutex                  m_lockSync;
    QWaitCondition          m_sync;
    bool                    m_newFrameGenerated;
};

} // End namespace

#endif // MSRDAILYCOLORINSTANCE_H
