#ifndef OPENNIUSERTRACKERINSTANCE_H
#define OPENNIUSERTRACKERINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/UserTrackerFrame.h"
#include "types/StreamInstance.h"

namespace dai {

class OpenNIUserTrackerInstance : public StreamInstance<UserTrackerFrame>
{
public:
    OpenNIUserTrackerInstance();
    virtual ~OpenNIUserTrackerInstance();
    bool is_open() const override;
    QList< shared_ptr<DataFrame> > frames() override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(UserTrackerFrame& frame) override;

private:
    OpenNIRuntime*            m_openni;
    shared_ptr<UserTrackerFrame> m_frameBuffer[2];
};

} // End Namespace

#endif // OPENNIUSERTRACKERINSTANCE_H
