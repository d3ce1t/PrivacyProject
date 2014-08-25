#include "OpenNIDevice.h"
#include <QDebug>
#include <iostream>
#include <glm/glm.hpp>

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
const QString OpenNIDevice::ANY_DEVICE = "ANY_DEVICE";

OpenNIDevice::OpenNIDevice(const QString devicePath)
    : m_devicePath(devicePath)
    , m_opened(false)
    , m_manual_registration(false)
{
    // Init OpenNI
    _mutex_counter.lock();
    _instance_counter++;
    if (!_initialised) {
        initOpenNI();
    }
    _mutex_counter.unlock();

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

        if (m_devicePath != OpenNIDevice::ANY_DEVICE)
            deviceURI = deviceURIStr.c_str();

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 2;

        // Enable Depth to Color image registration
        if (m_device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
            if (m_device.getImageRegistrationMode() == openni::IMAGE_REGISTRATION_OFF) {
                if (m_device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR) != openni::STATUS_OK)
                    qDebug() << "Depth to Color registration cannot be set";
            }
        } else {
            qDebug() << "Depth to Color registration isn't supported";
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

void OpenNIDevice::setRegistration(bool flag)
{
    m_manual_registration = flag;
}

void OpenNIDevice::readColorFrame(shared_ptr<ColorFrame> colorFrame)
{
    if (m_oniColorStream.readFrame(&m_oniColorFrame) != openni::STATUS_OK)
        throw 1;

    if (!m_oniColorFrame.isValid())
        throw 2;

    int stride = m_oniColorFrame.getStrideInBytes() / sizeof(openni::RGB888Pixel) - m_oniColorFrame.getWidth();

    if (stride > 0) {
        qWarning() << "WARNING: OpenNIDevice - Not managed color stride!!!";
        throw 3;
    }

    colorFrame->setDataPtr(640, 480, (RGBColor*) m_oniColorFrame.getData());
    colorFrame->setIndex(m_oniColorFrame.getFrameIndex());
}

void OpenNIDevice::readDepthFrame(shared_ptr<DepthFrame> depthFrame)
{
    if (m_oniDepthStream.readFrame(&m_oniDepthFrame) != openni::STATUS_OK) {
        throw 1;
    }

    if (!m_oniDepthFrame.isValid()) {
        throw 2;
    }

    int strideDepth = m_oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - m_oniDepthFrame.getWidth();

    if (strideDepth > 0) {
        qWarning() << "WARNING: OpenNIDevice - Not managed depth stride!!!";
        throw 3;
    }

    depthFrame->setDataPtr(640, 480, (uint16_t*) m_oniDepthFrame.getData());
    depthFrame->setIndex(m_oniDepthFrame.getFrameIndex());
    depthFrame->setDistanceUnits(DepthFrame::MILIMETERS);

    if (m_manual_registration) {
        depth2color(depthFrame);
    }
}

/* Old Version
 * shared_ptr<DepthFrame> OpenNIDevice::readDepthFrame()
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
                qWarning() << "WARNING: OpenNIDevice - Not managed depth stride!!!";
                throw 3;
            }

            m_depthFrame->setDataPtr(640, 480, (uint16_t*) m_oniDepthFrame.getData());
            m_depthFrame->setIndex(m_oniDepthFrame.getFrameIndex());
            return m_depthFrame;
        }
    }

    return m_depthFrame;
}*/

void OpenNIDevice::readUserTrackerFrame(shared_ptr<DepthFrame> depthFrame,
                                                             shared_ptr<MaskFrame> maskFrame,
                                                             shared_ptr<SkeletonFrame> skeletonFrame,
                                                             shared_ptr<MetadataFrame> metadataFrame)
{
    nite::UserTrackerFrameRef oniUserTrackerFrame;

    if (m_oniUserTracker.readFrame(&oniUserTrackerFrame) != nite::STATUS_OK) {
        throw 1;
    }

    if (!oniUserTrackerFrame.isValid()) {
        throw 2;
    }

    // Depth Frame
    m_oniDepthFrame = oniUserTrackerFrame.getDepthFrame();

    int strideDepth = m_oniDepthFrame.getStrideInBytes() / sizeof(openni::DepthPixel) - m_oniDepthFrame.getWidth();

    if (strideDepth > 0) {
        qWarning() << "WARNING: OpenNIDevice - Not managed depth stride!!!";
        throw 3;
    }

    depthFrame->setDataPtr(640, 480, (uint16_t*) m_oniDepthFrame.getData());
    depthFrame->setIndex(m_oniDepthFrame.getFrameIndex());
    depthFrame->setDistanceUnits(DepthFrame::MILIMETERS);

    // Load User Labels (copy)
    const nite::UserMap& userMap = oniUserTrackerFrame.getUserMap();

    int strideUser = userMap.getStride() / sizeof(nite::UserId) - userMap.getWidth();

    if (strideUser > 0) {
        qWarning() << "WARNING: OpenNIRuntime - Not managed user stride!!!";
        throw 1;
    }

    const nite::UserId* pLabel = userMap.getPixels();

    for (int i=0; i < userMap.getHeight(); ++i) {
        uint8_t* pMask = maskFrame->getRowPtr(i);
        for (int j=0; j < userMap.getWidth(); ++j) {
            pMask[j] = *pLabel;
            pLabel++;
        }
    }

    maskFrame->setIndex(oniUserTrackerFrame.getFrameIndex());

    // Registration
    if (m_manual_registration) {
        depth2color(depthFrame, maskFrame);
    }

    // Process Users
    skeletonFrame->clear();
    skeletonFrame->setIndex(oniUserTrackerFrame.getFrameIndex());
    metadataFrame->boundingBoxes().clear();
    metadataFrame->setIndex(oniUserTrackerFrame.getFrameIndex());

    const nite::Array<nite::UserData>& users = oniUserTrackerFrame.getUsers();

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            m_oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (user.isVisible())
        {
            // Get Boundingbox
            const nite::BoundingBox niteBoundingBox = user.getBoundingBox();
            const NitePoint3f bbMin = niteBoundingBox.min;
            const NitePoint3f bbMax = niteBoundingBox.max;
            dai::BoundingBox boundingBox(dai::Point3f(bbMin.x, bbMin.y, bbMin.z),
                                         dai::Point3f(bbMax.x, bbMax.y, bbMax.z));
            metadataFrame->boundingBoxes().append(boundingBox);

            // Get Skeleton
            const nite::Skeleton& oniSkeleton = user.getSkeleton();
            const nite::SkeletonJoint& head = user.getSkeleton().getJoint(nite::JOINT_HEAD);

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED && head.getPositionConfidence() > 0.5)
            {
                auto daiSkeleton = skeletonFrame->getSkeleton(user.getId());

                if (daiSkeleton == nullptr) {
                    daiSkeleton = make_shared<dai::Skeleton>(dai::Skeleton::SKELETON_OPENNI);
                    skeletonFrame->setSkeleton(user.getId(), daiSkeleton);
                }

                OpenNIDevice::copySkeleton(oniSkeleton, *(daiSkeleton.get()));
                daiSkeleton->computeQuaternions();
            }
        }
        else if (user.isLost()) {
            m_oniUserTracker.stopSkeletonTracking(user.getId());
        }
    } // End for
}

void OpenNIDevice::depth2color(shared_ptr<DepthFrame> depthFrame, shared_ptr<MaskFrame> mask) const
{
    // RGB Intrinsics
    const double fx_rgb = 529.21508098293293;
    const double fy_rgb = 525.56393630057437;
    const double cx_rgb = 320.0f; // 328.94272028759258;
    const double cy_rgb = 240.0f; // 267.48068171871557;

    // Depth Intrinsics
    const double fx_d = 594.21434211923247;
    const double fy_d = 591.04053696870778;
    const double cx_d = 320.0f; // 339.30780975300314;
    const double cy_d = 240.0f; // 242.73913761751615;

    const glm::mat3 r_matrix = {
        999.979, 6.497, -0.801,
        -6.498, 999.978, -1.054,
        0.794, 1.059, 999.999
    };

    const glm::vec3 t_vector = {
        -25.165, 0.047, -4.077
    };

    // Rotation Transform (millimeterS)
    /*glm::mat3 r_matrix = {
         999.84628826577793, 1.2635359098409581, -17.487233004436643,
        -1.4779096108364480, 999.92385683542895, -12.251380107679535,
         17.470421412464927, 12.275341476520762,  999.77202419716948
    };

    // Translation Transform (millimeters)
    glm::vec3 t_vector = {
        19.985242312092553, -7442.3738761617583e-04, -10.916736334336222
    };*/

    // Do Registration
    shared_ptr<DepthFrame> outputDepth = make_shared<DepthFrame>(640, 480);
    shared_ptr<MaskFrame> outputMask = mask ? make_shared<MaskFrame>(640, 480) : nullptr;

    for (int i=0; i<depthFrame->getHeight(); ++i)
    {
        uint16_t* pDepth = depthFrame->getRowPtr(i);
        uint8_t* pMask = mask ? mask->getRowPtr(i) : nullptr;

        for (int j=0; j<depthFrame->getWidth(); ++j)
        {
            // Convert each point of the depth frame into a real world coordinate in millimeters
            // FIX: I think next formula assumes depth is given as a distance from a point to the
            // sensor, whereas OpenNI gives it as a distance from the point to the sensor plane.

            // Start hack in order to get distance to sensor, rather than to the plane
            float out_x, out_y, out_z;
            m_oniUserTracker.convertDepthCoordinatesToJoint(j, i, pDepth[j], &out_x, &out_y);
            out_z = Point3f::euclideanDistance(Point3f(0, 0, 0), Point3f(out_x, out_y, pDepth[j]));
            // End hack

            glm::vec3 p3d;
            p3d.x = (j - cx_d) * out_z / fx_d;
            p3d.y = (i - cy_d) * out_z / fy_d;
            p3d.z = out_z;

            // Rotate and Translate 3D point to change origin to color sensor
            p3d = r_matrix * p3d;
            p3d = p3d + t_vector;

            // Reproject to 2D
            glm::vec2 p2d;
            p2d.x = (p3d.x * fx_rgb / p3d.z) + cx_rgb;
            p2d.y = (p3d.y * fy_rgb / p3d.z) + cy_rgb;

            if (p2d.x >= 0 && p2d.y >= 0 && p2d.x < 640 && p2d.y < 480) {
                outputDepth->setItem(p2d.y, p2d.x, pDepth[j]);
                if (outputMask) {
                    outputMask->setItem(p2d.y, p2d.x, pMask[j]);
                }
            }
        }
    }

    *depthFrame = *outputDepth; // Copy
    if (mask) {
        *mask = *outputMask; // Copy
    }
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
        device = new OpenNIDevice(devicePath); // TODO: Memory leak here
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

bool OpenNIDevice::isFile() const
{
    return m_device.isFile();
}

} // End Namespace
