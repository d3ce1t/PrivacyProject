#ifndef OPENNIDEPTHINSTANCE_H
#define OPENNIDEPTHINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/DepthFrame.h"
#include <QMutex>

namespace dai {

class OpenNIDepthInstance : public StreamInstance
{
public:
    OpenNIDepthInstance();
    virtual ~OpenNIDepthInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    QList<shared_ptr<DataFrame>> nextFrames() override;

private:
    OpenNIRuntime* m_openni;
};

} // End namespace

#endif // OPENNIDEPTHINSTANCE_H
