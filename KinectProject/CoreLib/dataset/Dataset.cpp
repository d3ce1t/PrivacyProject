#include "Dataset.h"

namespace dai {

Dataset::Dataset(const QString& xmlDescriptor)
{
    open(xmlDescriptor);
}

void Dataset::open(const QString& xmlDescriptor)
{
    m_metadata = DatasetMetadata::load(xmlDescriptor);
    m_metadata->setDataset(this);
}

void Dataset::setPath(const QString& path)
{
    m_metadata->m_path = path;
}

const DatasetMetadata& Dataset::getMetadata() const
{
    return *m_metadata;
}

shared_ptr<BaseInstance> Dataset::getInstance(int activity, int actor, int sample, DataFrame::FrameType type) const
{
    return instance(activity, actor, sample, type);
}

shared_ptr<BaseInstance> Dataset::getInstance(const InstanceInfo& info) const
{
    return instance(info.getActivity(), info.getActor(), info.getSample(), info.getType());
}

} // End Namespace
