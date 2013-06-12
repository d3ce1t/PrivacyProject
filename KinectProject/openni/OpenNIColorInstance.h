#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNICoreShared.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include <NiTE.h>
#include <OpenNI.h>
#include <fstream>

namespace dai {

class OpenNIColorInstance : public OpenNICoreShared, public StreamInstance
{
public:
    OpenNIColorInstance();
    void open();
    void close();
    bool hasNext() const;
    const ColorFrame& nextFrame();
    void setOutputFile(QString file);

private:
    openni::VideoStream 	m_colorStream;
    openni::VideoFrameRef	m_colorFrame;
    ColorFrame              m_currentFrame;
    int                     m_frameIndex;
    std::ofstream           m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
