#ifndef OPENNIUSERTRACKERINSTANCE_H
#define OPENNIUSERTRACKERINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/SkeletonFrame.h"
#include "types/MaskFrame.h"
#include "types/DepthFrame.h"

namespace dai {

class OpenNIUserTrackerInstance : public StreamInstance
{
public:
    OpenNIUserTrackerInstance();
    virtual ~OpenNIUserTrackerInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    QList<shared_ptr<DataFrame>> nextFrames() override;

private:
    OpenNIRuntime* m_openni;
    shared_ptr<DepthFrame> m_frameDepth;
    shared_ptr<MaskFrame> m_frameUser;
    shared_ptr<SkeletonFrame> m_frameSkeleton;
};

} // End Namespace

#endif // OPENNIUSERTRACKERINSTANCE_H
