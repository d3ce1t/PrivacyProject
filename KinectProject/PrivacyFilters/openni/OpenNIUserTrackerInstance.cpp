#include "OpenNIUserTrackerInstance.h"
#include <exception>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance()
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata)
{
    m_frameDepth = make_shared<DepthFrame>(640, 480);
    m_frameUser = make_shared<MaskFrame>(640, 480);
    m_frameSkeleton = make_shared<SkeletonFrame>();
    m_frameMetadata = make_shared<MetadataFrame>();
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
    if (is_open()) {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIUserTrackerInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIUserTrackerInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;
    nite::UserTrackerFrameRef oniUserTrackerFrame = m_openni->readUserTrackerFrame();

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
    m_frameDepth->setIndex(oniDepthFrame.getFrameIndex());
    m_frameUser->setIndex(oniUserTrackerFrame.getFrameIndex());

    for (int y=0; y < userMap.getHeight(); ++y) {
        for (int x=0; x < userMap.getWidth(); ++x) {
            uint8_t label = *pLabel;
            m_frameDepth->setItem(y, x, *pDepth / 1000.0f);
            m_frameUser->setItem(y, x, label);
            pDepth++;
            pLabel++;
        }
    }

    // Process Users
    m_frameSkeleton->clear();
    m_frameSkeleton->setIndex(oniUserTrackerFrame.getFrameIndex());
    m_frameMetadata->boundingBoxes().clear();
    m_frameMetadata->setIndex(oniUserTrackerFrame.getFrameIndex());

    nite::UserTracker& oniUserTracker = m_openni->getUserTracker();
    const nite::Array<nite::UserData>& users = oniUserTrackerFrame.getUsers();

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (user.isVisible())
        {
            // Get Boundingbox
            const nite::BoundingBox niteBoundingBox = user.getBoundingBox();
            const NitePoint3f bbMin = niteBoundingBox.min;
            const NitePoint3f bbMax = niteBoundingBox.max;
            dai::BoundingBox boundingBox(dai::Point3f(bbMin.x, bbMin.y, bbMin.z),
                                         dai::Point3f(bbMax.x, bbMax.y, bbMax.z));
            m_frameMetadata->boundingBoxes().append(boundingBox);

            // Get Skeleton
            const nite::Skeleton& oniSkeleton = user.getSkeleton();
            const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED && head.getPositionConfidence() > 0.5)
            {
                auto daiSkeleton = m_frameSkeleton->getSkeleton(user.getId());

                if (daiSkeleton == nullptr) {
                    daiSkeleton = make_shared<dai::Skeleton>(dai::Skeleton::SKELETON_OPENNI);
                    m_frameSkeleton->setSkeleton(user.getId(), daiSkeleton);
                }

                m_openni->copySkeleton(oniSkeleton, *(daiSkeleton.get()));
                daiSkeleton->computeQuaternions();
            }
        }
        else if (user.isLost()) {
            oniUserTracker.stopSkeletonTracking(user.getId());
        }
    } // End for

    oniUserTrackerFrame.release();
    result.append(m_frameDepth);
    result.append(m_frameUser);
    result.append(m_frameSkeleton);
    result.append(m_frameMetadata);
    return result;
}

} // End Namespace
