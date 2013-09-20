#ifndef OPENNISKELETONINSTANCE_H
#define OPENNISKELETONINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include <QTextStream>
#include "types/SkeletonFrame.h"

namespace dai {

class OpenNISkeletonInstance : public StreamInstance<SkeletonFrame>
{
public:
    OpenNISkeletonInstance();
    virtual ~OpenNISkeletonInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(SkeletonFrame& frame) override;

private:
    OpenNIRuntime*            m_openni;
    shared_ptr<SkeletonFrame> m_frameBuffer[2];
};

} // End Namespace

#endif // OPENNISKELETONINSTANCE_H
