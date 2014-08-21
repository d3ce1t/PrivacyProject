#include "InstanceInfo.h"
#include "exceptions/NotImplementedException.h"
#include <dataset/DatasetMetadata.h>
#include <QDebug>

using namespace dai;

InstanceInfo::InstanceInfo(shared_ptr<DatasetMetadata> parent)
{
    m_type = DataFrame::Unknown;
    m_parent = parent;
}

InstanceInfo::InstanceInfo(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_camera = other.m_camera;
    m_labels = other.m_labels;
    m_files = other.m_files; // Implicit sharing
    m_parent = other.m_parent;
}

InstanceInfo& InstanceInfo::operator=(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_actor = other.m_actor;
    m_camera = other.m_camera;
    m_sample = other.m_sample;
    m_labels = other.m_labels;
    m_files = other.m_files;
    m_parent = other.m_parent;
    return *this;
}

DataFrame::SupportedFrames InstanceInfo::getType() const
{
    return m_type;
}

QList<QString> InstanceInfo::getLabels() const
{
    return m_labels.values();
}

bool InstanceInfo::hasLabels(const QList<QString>& labels) const
{
    bool hasAll = true;

    auto it = labels.constBegin();

    while (it != labels.constEnd() && hasAll == true)
    {
        hasAll = m_labels.contains(*it);
        ++it;
    }

    return hasAll;
}

int InstanceInfo::getActor() const
{
    return m_actor;
}

int InstanceInfo::getCamera() const
{
    return m_camera;
}

int InstanceInfo::getSample() const
{
    return m_sample;
}

QString InstanceInfo::getFileName(DataFrame::FrameType frameType) const
{
    return m_files.value(frameType);
}

const DatasetMetadata &InstanceInfo::parent() const
{
    return *m_parent;
}

void InstanceInfo::addType(DataFrame::FrameType type)
{
    m_type |= type;
}

void InstanceInfo::addLabel(const QString &label)
{
    m_labels.insert(label);
}

void InstanceInfo::setActor(int actor)
{
    m_actor = actor;
}

void InstanceInfo::setCamera(int camera)
{
    m_camera = camera;
}

void InstanceInfo::setSample(int sample)
{
    m_sample = sample;
}

void InstanceInfo::addFileName(DataFrame::FrameType frameType, QString file)
{
    m_files.insert(frameType, file);
}
