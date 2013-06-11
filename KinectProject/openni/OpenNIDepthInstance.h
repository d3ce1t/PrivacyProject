#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <NiTE.h>
#include <OpenNI.h>
#include <fstream>

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
    void setOutputFile(QString file);

private:
    openni::Device			m_device;
    openni::VideoMode       videoMode;
    openni::VideoStream 	m_colorStream;
    nite::UserTracker       m_pUserTracker;
    openni::VideoFrameRef	m_colorFrame;
    DepthFrame              m_currentFrame;
    int                     m_frameIndex;
    std::ofstream           m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
