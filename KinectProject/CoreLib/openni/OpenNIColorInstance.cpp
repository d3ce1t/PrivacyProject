#include "OpenNIColorInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
    : StreamInstance(DataFrame::Color)
{
    m_device = nullptr;
}

OpenNIColorInstance::OpenNIColorInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Color)
{
    m_device = device;
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    closeInstance();
}

bool OpenNIColorInstance::is_open() const
{
    return m_device != nullptr && m_device->is_open();
}

bool OpenNIColorInstance::openInstance()
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

void OpenNIColorInstance::closeInstance()
{
    if (is_open())
    {
        m_device->close();
    }
}

void OpenNIColorInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIColorInstance::nextFrame()
{
    QList<shared_ptr<DataFrame>> result;
    shared_ptr<ColorFrame> colorFrame = m_device->readColorFrame();
    result.append(colorFrame);
    return result;
}

} // End namespace
