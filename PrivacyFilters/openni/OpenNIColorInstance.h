#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include "openni/OpenNIBaseInstance.h"
#include <QFile>
#include <QMutex>

namespace dai {

class OpenNIColorInstance : public StreamInstance<ColorFrame>,
                            public openni::VideoStream::NewFrameListener,
                            public OpenNIBaseInstance
{
public:
    OpenNIColorInstance();
    virtual ~OpenNIColorInstance();
    bool is_open() const override;
    void setOutputFile(QString file);
    void onNewFrame(openni::VideoStream& stream);

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(ColorFrame& frame) override;

private:
    OpenNIRuntime*         m_openni;
    shared_ptr<ColorFrame> m_frameBuffer[2];
    QFile                  m_of;
    QString                m_outputFile;
    openni::VideoFrameRef  m_oniColorFrame;
    QMutex                 m_lockFrame;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
