#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>
#include <QDebug>

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
QMutex OpenNIRuntime::mutex;

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
    : m_userFrame(640, 480)
{
    m_newUserFrameGenerated = false;
    m_newSkeletonFrameGenerated = false;
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
   shutdownOpenNI();
}

SkeletonFrame OpenNIRuntime::readSkeletonFrame()
{
    waitForNewSkeletonFrame();
    QReadLocker locker(&m_lockUserTracker);
    return m_skeletonFrame;
}

UserFrame OpenNIRuntime::readUserFrame()
{
    waitForNewUserFrame();
    QReadLocker locker(&m_lockUserTracker);
    return m_userFrame;
}

openni::PlaybackControl* OpenNIRuntime::playbackControl()
{
    return m_device.getPlaybackControl();
}

void OpenNIRuntime::addNewColorListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniColorStream.addNewFrameListener(listener);
}

void OpenNIRuntime::removeColorListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniColorStream.removeNewFrameListener(listener);
}

void OpenNIRuntime::addNewDepthListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniDepthStream.addNewFrameListener(listener);
}

void OpenNIRuntime::addNewUserTrackerListener(nite::UserTracker::NewFrameListener* listener)
{
    m_oniUserTracker.addNewFrameListener(listener);
}

void OpenNIRuntime::removeDepthListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniDepthStream.removeNewFrameListener(listener);
}

void OpenNIRuntime::removeUserTrackerListener(nite::UserTracker::NewFrameListener* listener)
{
    m_oniUserTracker.removeNewFrameListener(listener);
}

void OpenNIRuntime::initOpenNI()
{
    //const char* deviceURI = openni::ANY_DEVICE;
    const char* deviceURI = "/files/capture/PrimeSense Short-Range (1.09) - 1 user.oni";

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 3;

        // Enable Depth to Color image registration
        if (m_device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
            if (m_device.getImageRegistrationMode() == openni::IMAGE_REGISTRATION_OFF)
                m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
        }

        // Enable Depth and Color sync
        if (!m_device.getDepthColorSyncEnabled())
            m_device.setDepthColorSyncEnabled(true);

        qDebug() << "Sync Enabled:" << m_device.getDepthColorSyncEnabled();

        // Create Color Stream and Setup Mode
        if (m_oniColorStream.create(m_device, openni::SENSOR_COLOR) != openni::STATUS_OK)
            throw 4;

        openni::VideoMode videoMode = m_oniColorStream.getVideoMode();
        videoMode.setResolution(640, 480);
        m_oniColorStream.setVideoMode(videoMode);

        // Create Depth Stream and Setup Mode
        if (m_oniDepthStream.create(m_device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
            throw 5;

        videoMode = m_oniDepthStream.getVideoMode();
        videoMode.setResolution(640, 480);
        m_oniDepthStream.setVideoMode(videoMode);

        // Start
        if (m_oniColorStream.start() != openni::STATUS_OK)
            throw 6;

        if (m_oniDepthStream.start() != openni::STATUS_OK)
            throw 7;

        if (m_oniUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 8;

        if (!m_oniUserTracker.isValid() || !m_oniColorStream.isValid() || !m_oniDepthStream.isValid())
            throw 9;

        m_oniUserTracker.addNewFrameListener(this);
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        throw ex;
    }
}

void OpenNIRuntime::shutdownOpenNI()
{
    m_oniUserTracker.removeNewFrameListener(this);

    // Release frame refs
    //m_oniColorFrame.release();
    /*m_oniDepthStream.stop();
    m_oniColorStream.stop();*/

    // Destroy streams and close device
    m_oniUserTracker.destroy();
    m_oniDepthStream.destroy();
    m_oniColorStream.destroy();
    m_device.close();

    // Shutdown library
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();
}

nite::UserTracker& OpenNIRuntime::getUserTracker()
{
    return m_oniUserTracker;
}

openni::VideoStream& OpenNIRuntime::getDepthStream()
{
    return m_oniDepthStream;
}

void OpenNIRuntime::onNewFrame(nite::UserTracker& oniUserTracker)
{
    nite::UserTrackerFrameRef oniUserTrackerFrame;

    if (oniUserTracker.readFrame(&oniUserTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    if (!oniUserTrackerFrame.isValid()) {
        throw 2;
    }

    m_lockUserTracker.lockForWrite();
    loadUser(oniUserTrackerFrame);
    loadSkeleton(oniUserTracker, oniUserTrackerFrame);
    m_lockUserTracker.unlock();

    notifyNewUserFrame();
    notifyNewSkeletonFrame();
}

void OpenNIRuntime::loadUser(nite::UserTrackerFrameRef& oniUserTrackerFrame)
{
    const nite::UserMap& userMap = oniUserTrackerFrame.getUserMap();

    int strideUser = userMap.getStride() / sizeof(nite::UserId) - userMap.getWidth();

    if (strideUser > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed user stride!!!";
        throw 3;
    }

    const nite::UserId* pLabel = userMap.getPixels();
    m_userFrame.setIndex(oniUserTrackerFrame.getFrameIndex());

    // Read Depth Frame and Labels
    for (int y=0; y < userMap.getHeight(); ++y) {
        for (int x=0; x < userMap.getWidth(); ++x) {
            uint8_t label = *pLabel;
            m_userFrame.setItem(y, x, label);
            pLabel++;
        }
        // Skip rest of row (in case it exists)
        //pLabel += strideUser;
    }
}

void OpenNIRuntime::loadSkeleton(nite::UserTracker& oniUserTracker, nite::UserTrackerFrameRef& oniUserTrackerFrame)
{
    const nite::Array<nite::UserData>& users = oniUserTrackerFrame.getUsers();
    m_skeletonFrame.clear();
    m_skeletonFrame.setIndex(oniUserTrackerFrame.getFrameIndex());

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            qDebug()<< "New user!" << user.getId();
            oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (!user.isLost())
        {
            const nite::Skeleton& oniSkeleton = user.getSkeleton();
            const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED && head.getPositionConfidence() > 0.5)
            {
                auto daiSkeleton = m_skeletonFrame.getSkeleton(user.getId());

                if (daiSkeleton == nullptr) {
                    daiSkeleton.reset(new dai::Skeleton(dai::Skeleton::SKELETON_OPENNI));
                    m_skeletonFrame.setSkeleton(user.getId(), daiSkeleton);
                }

                copySkeleton(oniSkeleton, *(daiSkeleton.get()));
                daiSkeleton->computeQuaternions();
            }
        }
        else if (user.isLost()) {
            qDebug() << "user lost" << user.getId();
        }
    } // End for
}

void OpenNIRuntime::copySkeleton(const nite::Skeleton& srcSkeleton, dai::Skeleton& dstSkeleton)
{
    for (int j=0; j<15; ++j)
    {
        // Load nite::SkeletonJoint
        const nite::SkeletonJoint& niteJoint = srcSkeleton.getJoint((nite::JointType) j);
        const nite::Point3f& nitePos = niteJoint.getPosition();
        const nite::Quaternion& niteOrientation = niteJoint.getOrientation();

        // Copy nite joint pos to my own Joint converting from nite milimeters to meters
        SkeletonJoint joint(Point3f(nitePos.x / 1000, nitePos.y / 1000, nitePos.z / 1000), staticMap[j]);
        joint.setOrientation(Quaternion(niteOrientation.w, niteOrientation.x,
                                                           niteOrientation.y,
                                                           niteOrientation.z));
        joint.setPositionConfidence(niteJoint.getPositionConfidence());
        joint.setOrientationConfidence(niteJoint.getOrientationConfidence());
        dstSkeleton.setJoint(staticMap[j], joint);
    }
}

void OpenNIRuntime::convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY) const
{
    m_oniUserTracker.convertDepthCoordinatesToJoint(x, y, distance * 1000, &outX, &outY);
}

void OpenNIRuntime::convertRealWorldCoordinatesToDepth(float x, float y, float z, float* pOutX, float* pOutY) const
{
    m_oniUserTracker.convertJointCoordinatesToDepth(x, y, z * 1000, pOutX, pOutY);
}

void OpenNIRuntime::notifyNewUserFrame()
{
    m_lockUserSync.lock();
    if (!m_newUserFrameGenerated) {
        m_newUserFrameGenerated = true;
        m_userSync.wakeOne();
    }
    m_lockUserSync.unlock();
}

void OpenNIRuntime::waitForNewUserFrame()
{
    m_lockUserSync.lock();
    while (!m_newUserFrameGenerated) {
        m_userSync.wait(&m_lockUserSync);
    }
    m_newUserFrameGenerated = false;
    m_lockUserSync.unlock();
}

void OpenNIRuntime::notifyNewSkeletonFrame()
{
    m_lockSkeletonSync.lock();
    if (!m_newSkeletonFrameGenerated) {
        m_newSkeletonFrameGenerated = true;
        m_skeletonSync.wakeOne();
    }
    m_lockSkeletonSync.unlock();
}

void OpenNIRuntime::waitForNewSkeletonFrame()
{
    m_lockSkeletonSync.lock();
    while (!m_newSkeletonFrameGenerated) {
        m_skeletonSync.wait(&m_lockSkeletonSync);
    }
    m_newSkeletonFrameGenerated = false;
    m_lockSkeletonSync.unlock();
}

} // End namespace
