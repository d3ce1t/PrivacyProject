#ifndef PRIVACYFILTER_H
#define PRIVACYFILTER_H

#include "playback/NodeListener.h"
#include "playback/NodeProducer.h"

namespace dai {

class PrivacyFilter : public NodeListener, public NodeProducer
{
public:
    PrivacyFilter();
    ~PrivacyFilter();
    void newFrames(const QHashDataFrames dataFrames) override;

protected:
    QHashDataFrames produceFrames() override;

private:
    void dilateUserMask(uint8_t *labels);
    QHashDataFrames m_frames;
};

} // End Namespace

#endif // PRIVACYFILTER_H
