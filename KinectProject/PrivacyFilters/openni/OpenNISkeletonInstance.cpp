#include "OpenNISkeletonInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNISkeletonInstance::OpenNISkeletonInstance()
{
    this->m_type = INSTANCE_SKELETON;
    this->m_title = "Skeleton Live Stream";
    m_frameBuffer[0].reset(new SkeletonFrame);
    m_frameBuffer[1].reset(new SkeletonFrame);
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNISkeletonInstance::~OpenNISkeletonInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNISkeletonInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNISkeletonInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNISkeletonInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNISkeletonInstance::restartInstance()
{

}

void OpenNISkeletonInstance::nextFrame(SkeletonFrame &frame)
{
    // Read Data from OpenNI
    frame = m_openni->readSkeletonFrame(); // copy (block until there are a new frame)
}

} // End Namespace
