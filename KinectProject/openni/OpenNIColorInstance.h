#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include <QFile>

namespace dai {

class OpenNIColorInstance : public StreamInstance<ColorFrame>
{
public:
    OpenNIColorInstance();
    virtual ~OpenNIColorInstance();
    bool is_open() const override;
    void setOutputFile(QString file);

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
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
