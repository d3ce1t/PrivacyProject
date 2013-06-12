#ifndef OPENNICORESHARED_H
#define OPENNICORESHARED_H

#include <NiTE.h>
#include <OpenNI.h>

namespace dai {

class OpenNICoreShared
{
public:
    OpenNICoreShared();
    virtual ~OpenNICoreShared();

protected:
    static openni::Device device;

private:
    static void initOpenNI();
    static int instancesCounter;

};

} // End namespace

#endif // OPENNICORESHARED_H
