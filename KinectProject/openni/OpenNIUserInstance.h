#ifndef OPENNIUSERINSTANCE_H
#define OPENNIUSERINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/UserFrame.h"
#include <QFile>
#include <QMutex>

namespace dai {

class OpenNIUserInstance : public StreamInstance<UserFrame>
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
    void nextFrame(UserFrame& frame) override;

private:
    OpenNIRuntime*            m_openni;
    shared_ptr<UserFrame>     m_frameBuffer[2];
    QFile                     m_of;
    QString                   m_outputFile;
    nite::UserTrackerFrameRef m_oniUserTrackerFrame;
    QMutex                    m_lockFrame;
};

} // End namespace

#endif // OPENNIUSERINSTANCE_H
