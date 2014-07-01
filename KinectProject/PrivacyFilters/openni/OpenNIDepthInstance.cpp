#include "OpenNIDepthInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
    : StreamInstance(DataFrame::Depth)
{
    m_openni = nullptr;
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIDepthInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIDepthInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIDepthInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIDepthInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIDepthInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;
    shared_ptr<DepthFrame> depthFrame = m_openni->readDepthFrame();
    result.append(depthFrame);
    return result;
}

} // End namespace
