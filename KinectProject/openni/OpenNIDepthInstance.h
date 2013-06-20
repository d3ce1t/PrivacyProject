#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <QFile>

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
    DepthFrame& frame();
    void setOutputFile(QString file);

private:
    OpenNIRuntime*          m_openni;
    openni::VideoMode       videoMode;
    DepthFrame              m_currentFrame;
    unsigned int            m_frameIndex;
    QFile                   m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
