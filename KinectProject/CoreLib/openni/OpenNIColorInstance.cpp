#include "OpenNIColorInstance.h"
#include <exception>
#include <QDebug>

using namespace std;

namespace dai {

OpenNIColorInstance::OpenNIColorInstance()
    : StreamInstance(DataFrame::Color, 640, 480)
{
    m_device = nullptr;
    eof = false;
}

OpenNIColorInstance::OpenNIColorInstance(OpenNIDevice* device)
    : StreamInstance(DataFrame::Color, 640, 480)
{
    m_device = device;
    eof = false;
}

OpenNIColorInstance::~OpenNIColorInstance()
{
    closeInstance();
}

OpenNIDevice& OpenNIColorInstance::device()
{
    return *m_device;
}

bool OpenNIColorInstance::is_open() const
{
    return m_device != nullptr && m_device->is_open();
}

bool OpenNIColorInstance::hasNext() const
{
    if (!m_device->isFile())
        return true;

    return true;//!eof;
}

bool OpenNIColorInstance::openInstance()
{
    bool result = false;

    if (!is_open())
    {
        if (m_device == nullptr)
            m_device = OpenNIDevice::create("ANY_DEVICE");

        m_device->open();

        if (m_device->isFile()) {
            m_device->playbackControl()->setRepeatEnabled(false);
        }

        result = true;
    }

    return result;
}

void OpenNIColorInstance::closeInstance()
{
    if (is_open()) {
        m_device->close();
    }
}

void OpenNIColorInstance::restartInstance()
{
}

void OpenNIColorInstance::nextFrame(QHashDataFrames &output)
{
    Q_ASSERT(output.size() > 0);
    shared_ptr<ColorFrame> colorFrame = static_pointer_cast<ColorFrame>(output.value(DataFrame::Color));
    m_device->readColorFrame(colorFrame);

    if (m_device->isFile() && m_device->getTotalFrames() == colorFrame->getIndex()) {
        eof = true;
    }
}

} // End namespace
