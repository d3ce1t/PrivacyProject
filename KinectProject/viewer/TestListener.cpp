#include "TestListener.h"
#include <QDebug>
#include "types/Skeleton.h"
#include "types/DepthFrame.h"

namespace dai {

void TestListener::onNewFrame(const QList<shared_ptr<DataFrame>>& frames)
{
    shared_ptr<Skeleton> skeleton = static_pointer_cast<Skeleton>(frames.at(0));
    shared_ptr<DepthFrame> depth = static_pointer_cast<DepthFrame>(frames.at(1));
    qDebug() << "New Frames";
}

void TestListener::onPlaybackStart()
{

}

void TestListener::onPlaybackStop()
{

}

} // End Namespace
