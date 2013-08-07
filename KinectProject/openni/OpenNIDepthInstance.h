#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <QFile>

namespace dai {

class OpenNIDepthInstance : public StreamInstance<DepthFrame>
{
public:
    OpenNIDepthInstance();
    virtual ~OpenNIDepthInstance();
    bool is_open() const override;
    void setOutputFile(QString file);

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DepthFrame& frame) override;

private:
    OpenNIRuntime*          m_openni;
    shared_ptr<DepthFrame>  m_frameBuffer[2];
    QFile                   m_of;
    QString                 m_outputFile;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
