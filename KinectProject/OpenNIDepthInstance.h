#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <NiTE.h>
#include <OpenNI.h>

namespace dai {

class OpenNIDepthInstance : public StreamInstance
{
public:
    OpenNIDepthInstance();
    virtual ~OpenNIDepthInstance();
    void open();
    void close();
    bool hasNext() const;
    const DepthFrame& nextFrame();

private:
    openni::Device			m_device;
    openni::VideoMode       videoMode;
    openni::VideoStream 	m_colorStream;
    nite::UserTracker       m_pUserTracker;
    openni::VideoFrameRef	m_colorFrame;
    DepthFrame              m_currentFrame;
    int                     m_frameIndex;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
