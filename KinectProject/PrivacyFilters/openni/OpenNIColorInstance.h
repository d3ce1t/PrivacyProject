#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"

namespace dai {

class OpenNIColorInstance : public StreamInstance
{
public:
    OpenNIColorInstance();
    virtual ~OpenNIColorInstance();
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

#endif // OPENNICOLORINSTANCE_H
