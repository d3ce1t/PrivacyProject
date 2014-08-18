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
    m_frameSkeleton = make_shared<SkeletonFrame>();
    m_frameMetadata = make_shared<MetadataFrame>();
}

OpenNIUserTrackerInstance::OpenNIUserTrackerInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Depth | DataFrame::Mask | DataFrame::Skeleton | DataFrame::Metadata)
{
    m_device = device;
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

QList<shared_ptr<DataFrame>> OpenNIUserTrackerInstance::nextFrame()
{
    QList<shared_ptr<DataFrame>> result;
    nite::UserTrackerFrameRef oniUserTrackerFrame = m_device->readUserTrackerFrame();

    shared_ptr<DepthFrame> depthFrame = m_device->depthFrame();
    result.append(depthFrame);

    shared_ptr<MaskFrame> maskFrame = m_device->maskFrame();
    result.append(maskFrame);

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

    //oniUserTrackerFrame.release(); // I have to hold the frame as long as I can
    result.append(m_frameSkeleton);
    result.append(m_frameMetadata);
    return result;
}

} // End Namespace
