#include "InstanceInfo.h"
#include "exceptions/NotImplementedException.h"
#include <dataset/DatasetMetadata.h>

using namespace dai;

InstanceInfo::InstanceInfo(shared_ptr<DatasetMetadata> parent)
{
    m_type = DataFrame::Unknown;
    m_parent = parent;
}

InstanceInfo::InstanceInfo(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_files = other.m_files; // Implicit sharing
    m_parent = other.m_parent;
}

InstanceInfo& InstanceInfo::operator=(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_files = other.m_files;
    m_parent = other.m_parent;
    return *this;
}

DataFrame::SupportedFrames InstanceInfo::getType() const
{
    return m_type;
}

int InstanceInfo::getActivity() const
{
    return m_activity;
}

int InstanceInfo::getActor() const
{
    return m_actor;
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

void InstanceInfo::setActivity(int activity)
{
    m_activity = activity;
}

void InstanceInfo::setActor(int actor)
{
    m_actor = actor;
}

void InstanceInfo::setSample(int sample)
{
    m_sample = sample;
}

void InstanceInfo::addFileName(DataFrame::FrameType frameType, QString file)
{
    m_files.insert(frameType, file);
}
