#include "Dataset.h"

namespace dai {

Dataset::Dataset(QString path)
{
    m_metadata = DatasetMetadata::load(path);
    m_metadata->setDataset(this);
}

const DatasetMetadata& Dataset::getMetadata() const
{
    return *m_metadata;
}

shared_ptr<BaseInstance> Dataset::getInstance(int activity, int actor, int sample, InstanceType type) const
{
    return instance(activity, actor, sample, type);
}

shared_ptr<BaseInstance> Dataset::getInstance(const InstanceInfo& info) const
{
    return instance(info.getActivity(), info.getActor(), info.getSample(), info.getType());
}

} // End Namespace
