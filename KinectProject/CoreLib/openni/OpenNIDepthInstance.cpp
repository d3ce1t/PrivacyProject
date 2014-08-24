#include "OpenNIDepthInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIDepthInstance::OpenNIDepthInstance()
    : StreamInstance(DataFrame::Depth, 640, 480)
{
    m_device = nullptr;
}

OpenNIDepthInstance::OpenNIDepthInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Depth, 640, 480)
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

void OpenNIDepthInstance::nextFrame(QHashDataFrames &output)
{
    Q_ASSERT(output.size() > 0);
    shared_ptr<DepthFrame> depthFrame = static_pointer_cast<DepthFrame>(output.value(DataFrame::Depth));
    m_device->readDepthFrame(depthFrame);
}

} // End namespace
