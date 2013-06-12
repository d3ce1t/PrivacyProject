#include "OpenNICoreShared.h"

namespace dai {

int OpenNICoreShared::instancesCounter = 0;
openni::Device OpenNICoreShared::device;

void OpenNICoreShared::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (OpenNICoreShared::device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
        throw ex;
    }
}

OpenNICoreShared::OpenNICoreShared()
{
    if (instancesCounter == 0) {
        initOpenNI();
    }

    instancesCounter++;
}

OpenNICoreShared::~OpenNICoreShared()
{
    instancesCounter--;

    if (instancesCounter == 0) {
        OpenNICoreShared::device.close();
        nite::NiTE::shutdown();
        openni::OpenNI::shutdown();
    }
}

} // End namespace
