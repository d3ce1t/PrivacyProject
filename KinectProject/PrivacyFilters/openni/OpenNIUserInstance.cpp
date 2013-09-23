#include "OpenNIUserInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIUserInstance::OpenNIUserInstance()
{
    this->m_type = INSTANCE_USER;
    this->m_title = "User Live Stream";
    m_frameBuffer[0].reset(new UserFrame(640, 480));
    m_frameBuffer[1].reset(new UserFrame(640, 480));
    StreamInstance::initFrameBuffer(m_frameBuffer[0], m_frameBuffer[1]);
    m_openni = nullptr;
}

OpenNIUserInstance::~OpenNIUserInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIUserInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIUserInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIUserInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIUserInstance::restartInstance()
{

}

void OpenNIUserInstance::nextFrame(UserFrame &frame)
{
    frame = m_openni->readUserFrame(); // copy (block until there are a new frame)
}

} // End namespace
