#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
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
    OpenNIRuntime*       m_openni;
    ColorFrame           m_currentFrame;
    int                  m_frameIndex;
    std::ofstream        m_of;
    QString              m_outputFile;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
