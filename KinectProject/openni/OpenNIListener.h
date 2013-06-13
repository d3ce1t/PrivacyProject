#ifndef OPENNILISTENER_H
#define OPENNILISTENER_H

#include <OpenNI.h>
#include <NiTE.h>
#include "OpenNIRuntime.h"

namespace dai {

class OpenNIListener : public openni::VideoStream::NewFrameListener, public nite::UserTracker::Listener
{
public:
    OpenNIListener();
    void onNewFrame(openni::VideoStream& stream);
    void onNewFrame(nite::UserTracker& userTracker);

private:
    OpenNIRuntime* m_openni;
};

} // End namespace

#endif // OPENNILISTENER_H
