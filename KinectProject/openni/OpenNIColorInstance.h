#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include <NiTE.h>
#include <OpenNI.h>
#include <fstream>

namespace dai {

class OpenNIColorInstance : public StreamInstance
{
public:
    OpenNIColorInstance();
    virtual ~OpenNIColorInstance();
    void open();
    void close();
    bool hasNext() const;
    const ColorFrame& nextFrame();
    void setOutputFile(QString file);

private:
    openni::Device			m_device;
    openni::VideoStream 	m_colorStream;
    openni::VideoFrameRef	m_colorFrame;
    ColorFrame              m_currentFrame;
    int                     m_frameIndex;
    std::ofstream           m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
