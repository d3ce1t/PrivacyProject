#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
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
    OpenNIRuntime*          m_openni;
    openni::VideoMode       videoMode;
    DepthFrame              m_currentFrame;
    int                     m_frameIndex;
    std::ofstream           m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
