#include "OpenNIColorInstance.h"
#include <exception>
#include <iostream>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
    : StreamInstance(DataFrame::Color)
{
    m_openni = nullptr;
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    closeInstance();
    m_openni = nullptr;
}

bool OpenNIColorInstance::is_open() const
{
    return m_openni != nullptr;
}

bool OpenNIColorInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        m_openni = OpenNIRuntime::getInstance();
        result = true;
    }

    return result;
}

void OpenNIColorInstance::closeInstance()
{
    if (is_open())
    {
        m_openni->releaseInstance();
        m_openni = nullptr;
    }
}

void OpenNIColorInstance::restartInstance()
{
}

QList<shared_ptr<DataFrame>> OpenNIColorInstance::nextFrames()
{
    QList<shared_ptr<DataFrame>> result;
    shared_ptr<ColorFrame> colorFrame = m_openni->readColorFrame();
    result.append(colorFrame);
    return result;
}

} // End namespace
