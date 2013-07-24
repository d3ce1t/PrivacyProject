#ifndef OPENNIUSERINSTANCE_H
#define OPENNIUSERINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include <QFile>

namespace dai {

class OpenNIUserInstance : public StreamInstance
{
public:
    OpenNIUserInstance();
    virtual ~OpenNIUserInstance();
    bool is_open() const override;
    void setOutputFile(QString file);

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DataFrame& frame) override;

private:
    OpenNIRuntime*         m_openni;
    shared_ptr<ColorFrame> m_frameBuffer[2];
    QFile                  m_of;
    QString                m_outputFile;
};

} // End namespace

#endif // OPENNIUSERINSTANCE_H
