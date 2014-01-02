#include "OpenNIUserTrackerInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance()
{
    this->m_type = INSTANCE_USERTRACKER;
    this->m_title = "UserTracker Live Stream";
    m_frameBuffer[0].reset(new UserTrackerFrame(640, 480));
    m_frameBuffer[1].reset(new UserTrackerFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIUserTrackerInstance::~OpenNIUserTrackerInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIUserTrackerInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIUserTrackerInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIUserTrackerInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIUserTrackerInstance::restartInstance()
{

}

void OpenNIUserTrackerInstance::nextFrame(UserTrackerFrame &frame)
{
    nite::UserTrackerFrameRef oniUserTrackerFrame = m_openni->readUserTrackerFrame();

    // Load User
    const nite::UserMap& userMap = oniUserTrackerFrame.getUserMap();

    int strideUser = userMap.getStride() / sizeof(nite::UserId) - userMap.getWidth();

    if (strideUser > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed user stride!!!";
        throw 1;
    }

    const nite::UserId* pLabel = userMap.getPixels();
    frame.userFrame->setIndex(oniUserTrackerFrame.getFrameIndex());

    // Read Depth Frame and Labels
    for (int y=0; y < userMap.getHeight(); ++y) {
        for (int x=0; x < userMap.getWidth(); ++x) {
            uint8_t label = *pLabel;
            frame.userFrame->setItem(y, x, label);
            pLabel++;
        }
        // Skip rest of row (in case it exists)
        //pLabel += strideUser;
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
}

QList< shared_ptr<DataFrame> > OpenNIUserTrackerInstance::frames()
{
    QReadLocker locker(&m_locker);
    QList<shared_ptr<DataFrame>> result;
    result.append(static_pointer_cast<DataFrame>(m_readFrame->userFrame));
    result.append(static_pointer_cast<DataFrame>(m_readFrame->skeletonFrame));
    return result;
}

} // End Namespace
