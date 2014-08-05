#include "OpenNIUserTrackerInstance.h"
#include <exception>
#include <iostream>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance()
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata)
{
    m_device = nullptr;
    m_frameDepth = make_shared<DepthFrame>(640, 480);
    m_frameDepth->setDistanceUnits(DepthFrame::MILIMETERS);
    m_frameUser = make_shared<MaskFrame>(640, 480);
    m_frameSkeleton = make_shared<SkeletonFrame>();
    m_frameMetadata = make_shared<MetadataFrame>();
}

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata)
{
    m_device = device;
    m_frameDepth = make_shared<DepthFrame>(640, 480);
    m_frameDepth->setDistanceUnits(DepthFrame::MILIMETERS);
    m_frameUser = make_shared<MaskFrame>(640, 480);
    m_frameSkeleton = make_shared<SkeletonFrame>();
    m_frameMetadata = make_shared<MetadataFrame>();
}

OpenNIUserTrackerInstance::~OpenNIUserTrackerInstance()
{
    closeInstance();
}

bool OpenNIUserTrackerInstance::is_open() const
{
    return m_device->is_open();
}

bool OpenNIUserTrackerInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        if (m_device == nullptr)
            m_device = OpenNIDevice::create("ANY_DEVICE");
        m_device->open();
        result = true;
    }

    return result;
}

void OpenNIUserTrackerInstance::closeInstance()
{
    if (is_open()) {
        m_device->close();
    }
}

void OpenNIUserTrackerInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIUserTrackerInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;
    nite::UserTrackerFrameRef oniUserTrackerFrame = m_device->readUserTrackerFrame();

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
            m_frameDepth->setItem(y, x, *pDepth);
            m_frameUser->setItem(y, x, *pLabel);
            pDepth++;
            pLabel++;
        }
    }

    // Process Users
    m_frameSkeleton->clear();
    m_frameSkeleton->setIndex(oniUserTrackerFrame.getFrameIndex());
    m_frameMetadata->boundingBoxes().clear();
    m_frameMetadata->setIndex(oniUserTrackerFrame.getFrameIndex());

    nite::UserTracker& oniUserTracker = m_device->getUserTracker();
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

                OpenNIDevice::copySkeleton(oniSkeleton, *(daiSkeleton.get()));
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