#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>
#include "dataset/DataInstance.h"

namespace dai {

SkeletonJoint::JointType OpenNIRuntime::staticMap[15] = {
    SkeletonJoint::JOINT_HEAD,             // 0
    SkeletonJoint::JOINT_CENTER_SHOULDER,  // 1
    SkeletonJoint::JOINT_LEFT_SHOULDER,    // 2
    SkeletonJoint::JOINT_RIGHT_SHOULDER,   // 3
    SkeletonJoint::JOINT_LEFT_ELBOW,       // 4
    SkeletonJoint::JOINT_RIGHT_ELBOW,      // 5
    SkeletonJoint::JOINT_LEFT_HAND,        // 6
    SkeletonJoint::JOINT_RIGHT_HAND,       // 7
    SkeletonJoint::JOINT_SPINE,            // 8
    SkeletonJoint::JOINT_LEFT_HIP,         // 9
    SkeletonJoint::JOINT_RIGHT_HIP,        // 10
    SkeletonJoint::JOINT_LEFT_KNEE,        // 11
    SkeletonJoint::JOINT_RIGHT_KNEE,       // 12
    SkeletonJoint::JOINT_LEFT_FOOT,        // 13
    SkeletonJoint::JOINT_RIGHT_FOOT        // 14
};

OpenNIRuntime* OpenNIRuntime::_instance = nullptr;
int OpenNIRuntime::_instance_counter = 0;
QMutex             OpenNIRuntime::mutex;

OpenNIRuntime* OpenNIRuntime::getInstance()
{
    mutex.lock();
    if (_instance == nullptr) {
        _instance = new OpenNIRuntime();
    }
    _instance_counter++;
    mutex.unlock();
    return _instance;
}

void OpenNIRuntime::releaseInstance()
{
    mutex.lock();
    _instance_counter--;

    if (_instance_counter == 0)
        delete this;
    mutex.unlock();
}

OpenNIRuntime::OpenNIRuntime()
    : m_colorFrame(640, 480), m_depthFrame(640, 480), m_userFrame(640, 480)
{
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
   shutdownOpenNI();
}

DepthFrame OpenNIRuntime::readDepthFrame()
{
    QReadLocker locker(&m_lockDepth);
    return m_depthFrame;
}

SkeletonFrame OpenNIRuntime::readSkeletonFrame()
{
    QReadLocker locker(&m_lockDepth);
    return m_skeletonFrame;
}

UserFrame OpenNIRuntime::readUserFrame()
{
    QReadLocker locker(&m_lockDepth);
    return m_userFrame;
}

ColorFrame OpenNIRuntime::readColorFrame()
{
    QReadLocker locker(&m_lockColor);
    return m_colorFrame;
}

void OpenNIRuntime::initOpenNI()
{
    const char* deviceURI = openni::ANY_DEVICE;

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;

        if (m_oniColorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        if (m_oniColorStream.start() != openni::STATUS_OK)
            throw 5;

        if (m_oniUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 6;

        if (!m_oniUserTracker.isValid() || !m_oniColorStream.isValid())
            throw 7;


        m_oniColorStream.addNewFrameListener(this);
        m_oniUserTracker.addNewFrameListener(this);
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        shutdownOpenNI();
        throw ex;
    }
}

void OpenNIRuntime::shutdownOpenNI()
{
    // Remove listeners
    m_oniUserTracker.removeNewFrameListener(this);
    m_oniColorStream.removeNewFrameListener(this);

    // Release frame refs
    m_oniColorFrame.release();

    // Destroy streams and close device
    m_oniUserTracker.destroy();
    m_oniColorStream.destroy();
    m_device.close();

    // Shutdown library
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();
}

void OpenNIRuntime::onNewFrame(openni::VideoStream& stream)
{
    QWriteLocker locker(&m_lockColor);
    openni::SensorType type = stream.getSensorInfo().getSensorType();

    if (type == openni::SENSOR_COLOR)
    {
        if (stream.readFrame(&m_oniColorFrame) != openni::STATUS_OK) {
            throw 1;
        }

        if (!m_oniColorFrame.isValid()) {
            throw 2;
        }

        int stride = m_oniColorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel) - m_oniColorFrame.getWidth();

        if (stride > 0) {
            qWarning() << "WARNING: OpenNIRuntime - Not managed color stride!!!";
            throw 3;
        }

        memcpy((void*) m_colorFrame.getDataPtr(), m_oniColorFrame.getData(), 640 * 480 * sizeof(openni::RGB888Pixel));
    }
}

void OpenNIRuntime::onNewFrame(nite::UserTracker& userTracker)
{
    QWriteLocker locker(&m_lockDepth);

    if (userTracker.readFrame(&m_oniUserTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    if (!m_oniUserTrackerFrame.isValid()) {
        throw 2;
    }

    //m_pUserTracker.setSkeletonSmoothingFactor(0.7);
    openni::VideoFrameRef oniDepthFrame = m_oniUserTrackerFrame.getDepthFrame();
    const nite::UserMap& userMap = m_oniUserTrackerFrame.getUserMap();

    int strideDepth = oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - oniDepthFrame.getWidth();
    int strideUser = userMap.getStride() / sizeof(nite::UserId) - userMap.getWidth();

    if (strideDepth > 0 || strideUser > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed depth or user stride!!!";
        throw 3;
    }

    const openni::DepthPixel* pDepth = (const openni::DepthPixel*) oniDepthFrame.getData();
    const nite::UserId* pLabel = userMap.getPixels();

    // Read Depth Frame and Labels
    for (int y=0; y < oniDepthFrame.getHeight(); ++y) {
        for (int x=0; x < oniDepthFrame.getWidth(); ++x) {
            u_int8_t label = *pLabel;
            m_depthFrame.setItem(y, x, *pDepth / 1000.0f);
            m_userFrame.setItem(y, x, label);
            pDepth++;
            pLabel++;
        }
        // Skip rest of row (in case it exists)
        pDepth += strideDepth;
        pLabel += strideUser;
    }

    // Read Skeleton
    oniLoadSkeleton();
}

void OpenNIRuntime::oniLoadSkeleton()
{
    const nite::Array<nite::UserData>& users = m_oniUserTrackerFrame.getUsers();
    m_skeletonFrame.clear();
    int trackedSkeletons = 0;

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            qDebug() << "New User!";
            m_oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (!user.isLost())
        {
            //qDebug() << "No user lost!";
            const nite::Skeleton& oniSkeleton = user.getSkeleton();

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED)
            {
                trackedSkeletons++;
                shared_ptr<dai::Skeleton> daiSkeleton = m_skeletonFrame.getSkeleton(user.getId());

                if (daiSkeleton == nullptr) {
                    daiSkeleton.reset(new Skeleton(Skeleton::SKELETON_OPENNI));
                    m_skeletonFrame.setSkeleton(user.getId(), daiSkeleton);
                }

                for (int j=0; j<15; ++j)
                {
                    // Load nite::SkeletonJoint
                    const nite::SkeletonJoint& niteJoint = oniSkeleton.getJoint((nite::JointType) j);
                    nite::Point3f nitePos = niteJoint.getPosition();

                    // Copy nite joint pos to my own Joint
                    SkeletonJoint joint(Point3f(nitePos.x / 1000, nitePos.y / 1000, nitePos.z / 1000), staticMap[j]);
                    daiSkeleton->setJoint(staticMap[j], joint);
                }

                daiSkeleton->computeQuaternions();
            }
        }
        else {
            m_oniUserTracker.stopSkeletonTracking(user.getId());
        }
    } // End for

    qDebug() << "Tracked Skeletons" << trackedSkeletons << "Users" << users.getSize();
}

} // End namespace
