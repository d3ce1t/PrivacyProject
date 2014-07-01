#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>
#include <QDebug>

namespace dai {

SkeletonJoint::JointType OpenNIRuntime::staticMap[15] = {
    SkeletonJoint::JOINT_HEAD,             // 0
    SkeletonJoint::JOINT_CENTER_SHOULDER,  // 1
    SkeletonJoint::JOINT_RIGHT_SHOULDER,    // 2
    SkeletonJoint::JOINT_LEFT_SHOULDER,   // 3
    SkeletonJoint::JOINT_RIGHT_ELBOW,       // 4
    SkeletonJoint::JOINT_LEFT_ELBOW,      // 5
    SkeletonJoint::JOINT_RIGHT_HAND,        // 6
    SkeletonJoint::JOINT_LEFT_HAND,       // 7
    SkeletonJoint::JOINT_SPINE,            // 8
    SkeletonJoint::JOINT_RIGHT_HIP,         // 9
    SkeletonJoint::JOINT_LEFT_HIP,        // 10
    SkeletonJoint::JOINT_RIGHT_KNEE,        // 11
    SkeletonJoint::JOINT_LEFT_KNEE,       // 12
    SkeletonJoint::JOINT_RIGHT_FOOT,        // 13
    SkeletonJoint::JOINT_LEFT_FOOT        // 14
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

    if (_instance_counter == 0) {
        delete _instance;
        _instance = nullptr;
    }
    mutex.unlock();
}

OpenNIRuntime::OpenNIRuntime()
{
    m_colorFrame = make_shared<ColorFrame>();
    m_depthFrame = make_shared<DepthFrame>(640, 480);

    // Videostreams
    m_depthStreams = new openni::VideoStream*[1];
    m_depthStreams[0] = &m_oniDepthStream;
    m_colorStreams = new openni::VideoStream*[1];
    m_colorStreams[0] = &m_oniColorStream;
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
    delete[] m_depthStreams;
    delete[] m_colorStreams;
    shutdownOpenNI();
}

openni::PlaybackControl* OpenNIRuntime::playbackControl()
{
    return m_device.getPlaybackControl();
}

void OpenNIRuntime::initOpenNI()
{
    //const char* deviceURI = openni::ANY_DEVICE;
    const char* deviceURI = "/opt/captures/PrimeSense Short-Range (1.09) - 1 user.oni";

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
        videoMode.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
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

        m_oniUserTracker.setSkeletonSmoothingFactor(0.4f);
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        throw ex;
    }
}

void OpenNIRuntime::shutdownOpenNI()
{
    // Destroy streams and close device
    m_oniUserTracker.destroy();
    m_oniDepthStream.destroy();
    m_oniColorStream.stop();
    m_oniColorStream.destroy();
    m_device.close();

    // Shutdown library
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();
}

shared_ptr<ColorFrame> OpenNIRuntime::readColorFrame()
{
    if (m_oniColorStream.readFrame(&m_oniColorFrame) != openni::STATUS_OK)
        throw 1;

    if (!m_oniColorFrame.isValid())
        throw 2;

    int stride = m_oniColorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel) - m_oniColorFrame.getWidth();

    if (stride > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed color stride!!!";
        throw 3;
    }

    m_colorFrame->setDataPtr(640, 480, (RGBColor*) m_oniColorFrame.getData());
    m_colorFrame->setIndex(m_oniColorFrame.getFrameIndex());
    return m_colorFrame;
}

shared_ptr<DepthFrame> OpenNIRuntime::readDepthFrame()
{
    int changedIndex;

    if (openni::OpenNI::waitForAnyStream(m_depthStreams, 1, &changedIndex) == openni::STATUS_OK)
    {
        if (changedIndex == 0)
        {
            if (m_oniDepthStream.readFrame(&m_oniDepthFrame) != openni::STATUS_OK) {
                throw 1;
            }

            if (!m_oniDepthFrame.isValid()) {
                throw 2;
            }

            int strideDepth = m_oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - m_oniDepthFrame.getWidth();

            if (strideDepth > 0) {
                qWarning() << "WARNING: OpenNIRuntime - Not managed depth stride!!!";
                throw 3;
            }

            // Hack: Make a copy
            const openni::DepthPixel* pDepth = (const openni::DepthPixel*) m_oniDepthFrame.getData();

            for (int y=0; y < m_oniDepthFrame.getHeight(); ++y) {
                for (int x=0; x < m_oniDepthFrame.getWidth(); ++x) {
                    m_depthFrame->setItem(y, x, *pDepth / 1000.0f);
                    pDepth++;
                }
            }

            m_depthFrame->setIndex(m_oniDepthFrame.getFrameIndex());
        }
    }

    return m_depthFrame;
}

nite::UserTrackerFrameRef OpenNIRuntime::readUserTrackerFrame()
{
    nite::UserTrackerFrameRef oniUserTrackerFrame;

    if (m_oniUserTracker.readFrame(&oniUserTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    if (!oniUserTrackerFrame.isValid()) {
        throw 2;
    }

    return oniUserTrackerFrame;
}

nite::UserTracker& OpenNIRuntime::getUserTracker()
{
    return m_oniUserTracker;
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

} // End namespace
