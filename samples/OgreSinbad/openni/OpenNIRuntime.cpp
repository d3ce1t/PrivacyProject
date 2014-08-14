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
    qDebug() << "OpenNIRuntime::getInstance()";
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
    qDebug() << "OpenNIRuntime::OpenNIRuntime()";
    initOpenNI();
}

OpenNIRuntime::~OpenNIRuntime()
{
   shutdownOpenNI();
}

void OpenNIRuntime::addNewDepthListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniDepthStream.addNewFrameListener(listener);
}

void OpenNIRuntime::removeDepthListener(openni::VideoStream::NewFrameListener* listener)
{
    m_oniDepthStream.removeNewFrameListener(listener);
}

void OpenNIRuntime::addNewUserTrackerListener(nite::UserTracker::NewFrameListener* listener)
{
    m_oniUserTracker.addNewFrameListener(listener);
}

void OpenNIRuntime::removeUserTrackerListener(nite::UserTracker::NewFrameListener* listener)
{
    m_oniUserTracker.removeNewFrameListener(listener);
}

void OpenNIRuntime::initOpenNI()
{
    qDebug() << "OpenNIRuntime::initOpenNI()";
    //const char* deviceURI = openni::ANY_DEVICE;
    const char* deviceURI = "/files/capture/PSSR - Ogre.oni";

    try {
        if (openni::OpenNI::initialize() != openni::STATUS_OK)
            throw 1;

        if (m_device.open(deviceURI) != openni::STATUS_OK)
            throw 3;

        if (nite::NiTE::initialize() != nite::STATUS_OK)
            throw 2;

        // Create Depth Stream and Setup Mode
        if (m_oniDepthStream.create(m_device, openni::SENSOR_DEPTH) != openni::STATUS_OK)
            throw 5;

        openni::VideoMode videoMode = m_oniDepthStream.getVideoMode();
        videoMode.setResolution(640, 480);
        m_oniDepthStream.setVideoMode(videoMode);

        if (m_oniUserTracker.create(&m_device) != nite::STATUS_OK)
            throw 8;

        if (!m_oniUserTracker.isValid() || !m_oniDepthStream.isValid() )
            throw 10;

        qDebug() << "All Streams are Valid";
    }
    catch (int ex)
    {
        printf("OpenNI init error:\n%s\n", openni::OpenNI::getExtendedError());
        throw ex;
    }
}

void OpenNIRuntime::shutdownOpenNI()
{
    //m_oniUserTracker.removeNewFrameListener(this);

    // Release frame refs
    //m_oniColorFrame.release();
    /*m_oniDepthStream.stop();
    m_oniColorStream.stop();*/

    // Destroy streams and close device
    m_oniUserTracker.destroy();
    m_oniDepthStream.destroy();
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

void OpenNIRuntime::convertDepthToRealWorld(int x, int y, float distance, float &outX, float &outY)
{
    m_oniUserTracker.convertDepthCoordinatesToJoint(x, y, distance * 1000, &outX, &outY);
}


} // End namespace
