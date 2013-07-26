#include "TestListener.h"
#include <QDebug>
#include "types/SkeletonFrame.h"
#include "types/DepthFrame.h"

namespace dai {

void TestListener::onNewFrame(const QList<shared_ptr<DataFrame>>& frames)
{
    shared_ptr<SkeletonFrame> skeleton = static_pointer_cast<SkeletonFrame>(frames.at(0));
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
