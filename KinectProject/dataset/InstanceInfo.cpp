#include "InstanceInfo.h"

using namespace dai;

InstanceInfo::InstanceInfo()
{
    m_type = Uninitialised;
}

InstanceInfo::InstanceInfo(InstanceType type)
{
    m_type = type;
}

InstanceInfo::InstanceInfo(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_file = other.m_file;
    m_path = other.m_path;
}

InstanceInfo& InstanceInfo::operator=(const InstanceInfo& other)
{
    m_type = other.m_type;
    m_activity = other.m_activity;
    m_actor = other.m_actor;
    m_sample = other.m_sample;
    m_file = other.m_file;
    m_path = other.m_path;
}

InstanceInfo::InstanceType InstanceInfo::getType() const
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

const QString& InstanceInfo::getFileName() const
{
    return m_file;
}

const QString& InstanceInfo::getDatasetPath() const
{
    return m_path;
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

void InstanceInfo::setDatasetPath(QString path)
{
    m_path = path;
}
