#include "OpenNIDepthInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
    : StreamInstance(DataFrame::Depth)
{
    m_device = nullptr;
}

OpenNIDepthInstance::OpenNIDepthInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Depth)
{
    m_device = device;
}

OpenNIDepthInstance::~OpenNIDepthInstance()
{
    closeInstance();
}

bool OpenNIDepthInstance::is_open() const
{
    return m_device->is_open();
}

bool OpenNIDepthInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        if (m_device == nullptr)
            m_device = OpenNIDevice::create("ANY_DEVICE");
        m_device->open();
        result = true;
    }

    return result;
}

void OpenNIDepthInstance::closeInstance()
{
    if (is_open())
    {
        m_device->close();
    }
}

void OpenNIDepthInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIDepthInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;
    shared_ptr<DepthFrame> depthFrame = m_device->readDepthFrame();
    result.append(depthFrame);
    return result;
}

} // End namespace
