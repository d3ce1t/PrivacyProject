#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include <QFile>

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
    ColorFrame& frame();
    void setOutputFile(QString file);

private:
    OpenNIRuntime*       m_openni;
    ColorFrame           m_currentFrame;
    unsigned int         m_frameIndex;
    QFile                m_of;
    QString              m_outputFile;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
