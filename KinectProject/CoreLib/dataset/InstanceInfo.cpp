#include "InstanceInfo.h"
#include "exceptions/NotImplementedException.h"
#include <dataset/DatasetMetadata.h>

using namespace dai;

InstanceInfo::InstanceInfo(shared_ptr<DatasetMetadata> parent)
{
    m_type = INSTANCE_UNINITIALISED;
    m_parent = parent;
}

InstanceInfo::InstanceInfo(InstanceType type, shared_ptr<DatasetMetadata> parent)
{
    if (type == INSTANCE_UNINITIALISED)
        throw NotImplementedException();

    m_type = type;
    m_parent = parent;
}

InstanceInfo::InstanceInfo(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_file = other.m_file;
    m_parent = other.m_parent;
}

InstanceInfo& InstanceInfo::operator=(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_file = other.m_file;
    m_parent = other.m_parent;
    return *this;
}

InstanceType InstanceInfo::getType() const
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

QString InstanceInfo::getFileName() const
{
    return m_file;
}

const DatasetMetadata &InstanceInfo::parent() const
{
    return *m_parent;
}

void InstanceInfo::setType(InstanceType type)
{
    m_type = type;
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

void InstanceInfo::setFileName(QString file)
{
    m_file = file;
}
