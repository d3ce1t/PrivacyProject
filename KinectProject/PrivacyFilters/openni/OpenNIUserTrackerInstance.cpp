#include "OpenNIUserTrackerInstance.h"
#include "exceptions/NotOpenedInstanceException.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance()
    : BaseInstance(DataFrame::Depth | DataFrame::User | DataFrame::Skeleton)
{
    this->m_title = "UserTracker Live Stream";
    m_frameBuffer[0] = new UserTrackerFrame(640, 480);
    m_frameBuffer[1] = new UserTrackerFrame(640, 480);
    m_writeFrame = m_frameBuffer[0];
    m_readFrame = m_frameBuffer[1];
    m_openni = nullptr;
}

OpenNIUserTrackerInstance::~OpenNIUserTrackerInstance()
{
    delete m_frameBuffer[0];
    delete m_frameBuffer[1];
    m_writeFrame = nullptr;
    m_readFrame = nullptr;
    close();
}

void OpenNIUserTrackerInstance::open()
{
    if (!is_open()) {
        m_openni = OpenNIRuntime::getInstance();
    }
}

void OpenNIUserTrackerInstance::close()
{
    if (is_open()) {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIUserTrackerInstance::restart()
{

}

bool OpenNIUserTrackerInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIUserTrackerInstance::hasNext() const
{
    return true;
}

void OpenNIUserTrackerInstance::swapBuffer()
{
    QWriteLocker locker(&m_locker);
    UserTrackerFrame* tmpPtr = m_readFrame;
    m_readFrame = m_writeFrame;
    m_writeFrame = tmpPtr;
}

void OpenNIUserTrackerInstance::readNextFrame()
{
    if (!is_open()) {
        throw NotOpenedInstanceException();
    }

    if (hasNext()) {
        nextFrame(*m_writeFrame);
    }
}

void OpenNIUserTrackerInstance::nextFrame(UserTrackerFrame &frame)
{
    nite::UserTrackerFrameRef oniUserTrackerFrame = m_openni->readUserTrackerFrame();

    if (!oniUserTrackerFrame.isValid()) {
        qDebug() << "UserTracker Frame isn't valid";
        return;
    }

    // Load Depth and User Labels
    openni::VideoFrameRef oniDepthFrame = oniUserTrackerFrame.getDepthFrame();
    const openni::DepthPixel* pDepth = (const openni::DepthPixel*) oniDepthFrame.getData();
    const nite::UserMap& userMap = oniUserTrackerFrame.getUserMap();

    int strideUser = userMap.getStride() / sizeof(nite::UserId) - userMap.getWidth();

    if (strideUser > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed user stride!!!";
        throw 1;
    }

    const nite::UserId* pLabel = userMap.getPixels();
    frame.depthFrame->setIndex(oniDepthFrame.getFrameIndex());
    frame.userFrame->setIndex(oniUserTrackerFrame.getFrameIndex());

    for (int y=0; y < userMap.getHeight(); ++y) {
        for (int x=0; x < userMap.getWidth(); ++x) {
            uint8_t label = *pLabel;
            frame.depthFrame->setItem(y, x, *pDepth / 1000.0f);
            frame.userFrame->setItem(y, x, label);
            pDepth++;
            pLabel++;
        }
    }

    // Load Skeleton
    nite::UserTracker& oniUserTracker = m_openni->getUserTracker();
    const nite::Array<nite::UserData>& users = oniUserTrackerFrame.getUsers();

    frame.skeletonFrame->clear();
    frame.skeletonFrame->setIndex(oniUserTrackerFrame.getFrameIndex());

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (!user.isLost())
        {
            const nite::Skeleton& oniSkeleton = user.getSkeleton();
            const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED && head.getPositionConfidence() > 0.5)
            {
                auto daiSkeleton = frame.skeletonFrame->getSkeleton(user.getId());

                if (daiSkeleton == nullptr) {
                    daiSkeleton.reset(new dai::Skeleton(dai::Skeleton::SKELETON_OPENNI));
                    frame.skeletonFrame->setSkeleton(user.getId(), daiSkeleton);
                }

                m_openni->copySkeleton(oniSkeleton, *(daiSkeleton.get()));
                daiSkeleton->computeQuaternions();
            }
        }
    } // End for

    oniUserTrackerFrame.release();
}

QList< shared_ptr<DataFrame> > OpenNIUserTrackerInstance::frames()
{
    QReadLocker locker(&m_locker);
    QList<shared_ptr<DataFrame>> result;
    result.append( static_pointer_cast<DataFrame>(m_readFrame->depthFrame) );
    result.append( static_pointer_cast<DataFrame>(m_readFrame->userFrame) );
    result.append( static_pointer_cast<DataFrame>(m_readFrame->skeletonFrame) );
    return result;
}

} // End Namespace
