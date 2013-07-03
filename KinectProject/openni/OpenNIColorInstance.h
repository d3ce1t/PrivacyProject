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
    bool is_open() const Q_DECL_OVERRIDE;
    ColorFrame& frame() Q_DECL_OVERRIDE;
    void setOutputFile(QString file);

protected:
    void openInstance() Q_DECL_OVERRIDE;
    void closeInstance() Q_DECL_OVERRIDE;
    void restartInstance() Q_DECL_OVERRIDE;
    void nextFrame(DataFrame& frame);

private:
    OpenNIRuntime*       m_openni;
    ColorFrame           m_frameBuffer[2];
    QFile                m_of;
    QString              m_outputFile;
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
