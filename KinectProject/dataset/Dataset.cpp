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

shared_ptr<BaseInstance> Dataset::getInstance(InstanceInfo info) const
{
    return getInstance(info.getActivity(), info.getActor(), info.getSample(), info.getType());
}

} // End Namespace
