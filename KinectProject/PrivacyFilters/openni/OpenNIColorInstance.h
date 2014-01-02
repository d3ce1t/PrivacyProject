#ifndef OPENNICOLORINSTANCE_H
#define OPENNICOLORINSTANCE_H

#include "OpenNIRuntime.h"
#include "types/StreamInstance.h"
#include "types/ColorFrame.h"
#include "openni/OpenNIBaseInstance.h"
#include <QMutex>

namespace dai {

class OpenNIColorInstance : public StreamInstance<ColorFrame>,
                            public OpenNIBaseInstance
{
public:
    OpenNIColorInstance();
    virtual ~OpenNIColorInstance();
    bool is_open() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(ColorFrame& frame) override;

private:
    OpenNIRuntime*         m_openni;
    shared_ptr<ColorFrame> m_frameBuffer[2];
};

} // End namespace

#endif // OPENNICOLORINSTANCE_H
