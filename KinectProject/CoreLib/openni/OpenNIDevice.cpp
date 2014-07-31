#include "OpenNIDevice.h"
#include <QDebug>
#include <iostream>

using namespace std;

namespace dai {

SkeletonJoint::JointType OpenNIDevice::_staticMap[15] = {
    SkeletonJoint::JOINT_HEAD,             // 0
    SkeletonJoint::JOINT_CENTER_SHOULDER,  // 1
    SkeletonJoint::JOINT_RIGHT_SHOULDER,   // 2
    SkeletonJoint::JOINT_LEFT_SHOULDER,    // 3
    SkeletonJoint::JOINT_RIGHT_ELBOW,      // 4
    SkeletonJoint::JOINT_LEFT_ELBOW,       // 5
    SkeletonJoint::JOINT_RIGHT_HAND,       // 6
    SkeletonJoint::JOINT_LEFT_HAND,        // 7
    SkeletonJoint::JOINT_SPINE,            // 8
    SkeletonJoint::JOINT_RIGHT_HIP,        // 9
    SkeletonJoint::JOINT_LEFT_HIP,         // 10
    SkeletonJoint::JOINT_RIGHT_KNEE,       // 11
    SkeletonJoint::JOINT_LEFT_KNEE,        // 12
    SkeletonJoint::JOINT_RIGHT_FOOT,       // 13
    SkeletonJoint::JOINT_LEFT_FOOT         // 14
};

QHash<QString, OpenNIDevice*> OpenNIDevice::_created_instances;
int OpenNIDevice::_instance_counter = 0;
bool OpenNIDevice::_initialised = false;
QMutex OpenNIDevice::_mutex_counter;

OpenNIDevice::OpenNIDevice(const QString devicePath)
    : m_devicePath(devicePath)
    , m_opened(false)
{
    // Init OpenNI
    _mutex_counter.lock();
    _instance_counter++;
    if (!_initialised) {
        initOpenNI();
    }
    _mutex_counter.unlock();

    // Create buffers for data
    m_colorFrame = make_shared<ColorFrame>();
    m_depthFrame = make_shared<DepthFrame>(640, 480);

    // Videostreams
    m_depthStreams = new openni::VideoStream*[1];
    m_depthStreams[0] = &m_oniDepthStream;
    m_colorStreams = new openni::VideoStream*[1];
    m_colorStreams[0] = &m_oniColorStream;
}

OpenNIDevice::~OpenNIDevice()
{
    close();

    // Free Memory
    delete[] m_depthStreams;
    delete[] m_colorStreams;

    // Update counters
    _mutex_counter.lock();
    _instance_counter--;

    if (_instance_counter == 0) {
        shutdownOpenNI();
    }
    _mutex_counter.unlock();
}

void OpenNIDevice::open()
{
    // Open device and create streams
    try {
        QMutexLocker locker(&m_mutex);

        if (m_opened)
            return;

        const char* deviceURI = openni::ANY_DEVICE;
        std::string deviceURIStr = m_devicePath.toStdString();

        if (m_devicePath != "ANY_DEVICE")
            deviceURI = deviceURIStr.c_str();

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

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
        m_opened = true;
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        throw ex;
    }
}

void OpenNIDevice::close()
{
    QMutexLocker locker(&m_mutex);
    // Destroy streams and close device
    m_oniUserTracker.destroy();
    m_oniDepthStream.destroy();
    m_oniColorStream.stop();
    m_oniColorStream.destroy();
    m_device.close();
    m_opened = false;
}

bool OpenNIDevice::is_open() const
{
    return m_device.isValid();
}

openni::PlaybackControl* OpenNIDevice::playbackControl()
{
    return m_device.getPlaybackControl();
}

shared_ptr<ColorFrame> OpenNIDevice::readColorFrame()
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

shared_ptr<DepthFrame> OpenNIDevice::readDepthFrame()
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

nite::UserTrackerFrameRef OpenNIDevice::readUserTrackerFrame()
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

nite::UserTracker& OpenNIDevice::getUserTracker()
{
    return m_oniUserTracker;
}

void OpenNIDevice::copySkeleton(const nite::Skeleton& srcSkeleton, dai::Skeleton& dstSkeleton)
{
    for (int j=0; j<15; ++j)
    {
        // Load nite::SkeletonJoint
        const nite::SkeletonJoint& niteJoint = srcSkeleton.getJoint((nite::JointType) j);
        const nite::Point3f& nitePos = niteJoint.getPosition();
        const nite::Quaternion& niteOrientation = niteJoint.getOrientation();

        // Copy nite joint pos to my own Joint converting from nite milimeters to meters
        SkeletonJoint joint(Point3f(nitePos.x / 1000, nitePos.y / 1000, nitePos.z / 1000), _staticMap[j]);
        joint.setOrientation(Quaternion(niteOrientation.w, niteOrientation.x,
                                                           niteOrientation.y,
                                                           niteOrientation.z));

        joint.setPositionConfidence(niteJoint.getPositionConfidence());
        joint.setOrientationConfidence(niteJoint.getOrientationConfidence());
        dstSkeleton.setJoint(_staticMap[j], joint);
    }
}

OpenNIDevice* OpenNIDevice::create(const QString devicePath)
{
    OpenNIDevice* device = _created_instances.value(devicePath);

    if (device == nullptr) {
        // Create device
        device = new OpenNIDevice(devicePath);
        _created_instances.insert(devicePath, device);
    }

    return device;
}

void OpenNIDevice::initOpenNI()
{
    // Init OpenNI and NiTE (
    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 2;

        _initialised = true;
    }
    catch (int ex) {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        throw ex;
    }
}

void OpenNIDevice::shutdownOpenNI()
{
    // Shutdown library
    nite::NiTE::shutdown();
    openni::OpenNI::shutdown();

    _initialised = false;
}

} // End Namespace
