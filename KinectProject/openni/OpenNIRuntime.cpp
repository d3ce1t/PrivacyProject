#include "OpenNIRuntime.h"
#include <iostream>
#include <cstdio>
#include "dataset/DataInstance.h"

namespace dai {

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
    m_activeUser = -1;
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
   //m_recorder.stop();
   //m_recorder.destroy();
   shutdownOpenNI();
}

DepthFrame OpenNIRuntime::readDepthFrame()
{
    QReadLocker locker(&m_lockDepth);
    return m_depthFrame;
}

Skeleton OpenNIRuntime::readSkeleton()
{
    QReadLocker locker(&m_lockDepth);
    return m_skeleton;
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

        /*if (m_recorder.create("/files/capture/kinect.oni") != openni::STATUS_OK) {
            throw 8;
        }

        if (m_recorder.attach(m_oniColorStream) != openni::STATUS_OK) {
            throw 9;
        }

        if (m_recorder.start() != openni::STATUS_OK) {
            throw 10;
        }*/

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
            // FIX: I assume depth value is between 0 a 10000.
            m_depthFrame.setItem(y, x, DataInstance::normalise(*pDepth, 0, 10000, 0, 1));
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

    for (int i=0; i<users.getSize(); ++i)
    {
        const nite::UserData& user = users[i];

        if (user.isNew()) {
            qDebug() << "New User!";
            m_oniUserTracker.startSkeletonTracking(user.getId());
        }
        else if (!user.isLost())
        {
            const nite::Skeleton& oniSkeleton = user.getSkeleton();

            if (oniSkeleton.getState() == nite::SKELETON_TRACKED) {
                qDebug() << "Skeleton Tracked" << i;
                m_activeUser = i;
            } else {
                m_activeUser = -1;
            }
        }

    }

    if (m_activeUser != -1)
    {
        const nite::Skeleton& oniSkeleton = users[m_activeUser].getSkeleton();
        m_skeleton.clear();

        for (int i=0; i<15; ++i)
        {
            SkeletonJoint& joint = m_skeleton.getJoint(mapNiteToOwn(i));
            const nite::SkeletonJoint& niteJoint = oniSkeleton.getJoint((nite::JointType) i);
            nite::Point3f nitePos = niteJoint.getPosition();
            joint.setPosition(Point3f(nitePos.x, nitePos.y, nitePos.z));
            joint.setType(mapNiteToOwn(i));
            joint.enableJoint(true);
        }

        m_skeleton.normaliseDepth(0, 10000, 0, 1);
        m_skeleton.computeQuaternions();
    }
}

SkeletonJoint::JointType OpenNIRuntime::mapNiteToOwn(int value)
{
    SkeletonJoint::JointType result = SkeletonJoint::JOINT_HEAD;

    switch (value) {
    // NiTE JOINT_HEAD
    case 0:
        result = SkeletonJoint::JOINT_HEAD;
        break;
    // NiTE JOINT_NECK
    case 1:
        result = SkeletonJoint::JOINT_CENTER_SHOULDER;
        break;

    // NiTE JOINT_LEFT_SHOULDER,
    case 2:
        result = SkeletonJoint::JOINT_LEFT_SHOULDER;
        break;

    // NiTE JOINT_RIGHT_SHOULDER,
    case 3:
        result = SkeletonJoint::JOINT_RIGHT_SHOULDER;
        break;

    // NiTE JOINT_LEFT_ELBOW,
    case 4:
        result = SkeletonJoint::JOINT_LEFT_ELBOW;
        break;

    // NiTE JOINT_RIGHT_ELBOW,
    case 5:
        result = SkeletonJoint::JOINT_RIGHT_ELBOW;
        break;

    // NiTE JOINT_LEFT_HAND,
    case 6:
        result = SkeletonJoint::JOINT_LEFT_HAND;
        break;

    // NiTE JOINT_RIGHT_HAND,
    case 7:
        result = SkeletonJoint::JOINT_RIGHT_HAND;
        break;

    // NiTE JOINT_TORSO,
    case 8:
        result = SkeletonJoint::JOINT_SPINE;
        break;

    // NiTE JOINT_LEFT_HIP,
    case 9:
        result = SkeletonJoint::JOINT_LEFT_HIP;
        break;

    // NiTE JOINT_RIGHT_HIP,
    case 10:
        result = SkeletonJoint::JOINT_RIGHT_HIP;
        break;

    // NiTE JOINT_LEFT_KNEE,
    case 11:
        result = SkeletonJoint::JOINT_LEFT_KNEE;
        break;

    // NiTE JOINT_RIGHT_KNEE,
    case 12:
        result = SkeletonJoint::JOINT_RIGHT_KNEE;
        break;

    // NiTE JOINT_LEFT_FOOT,
    case 13:
        result = SkeletonJoint::JOINT_LEFT_FOOT;
        break;

    // NiTE JOINT_RIGHT_FOOT,
    case 14:
        result = SkeletonJoint::JOINT_RIGHT_FOOT;
        break;
    default:
        qDebug() << "No se deberia entrar aqui";
    }

    return result;
}


} // End namespace
