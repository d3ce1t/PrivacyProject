#include "Dataset.h"
#include "exceptions/NotImplementedException.h"

namespace dai {

Dataset::Dataset(QString path)
{
    m_metadata = DatasetMetadata::load(path);
    m_metadata->setDataset(this);
}

Dataset::~Dataset()
{
    if (m_metadata != nullptr) {
        delete m_metadata;
        m_metadata = nullptr;
    }
}

const DatasetMetadata& Dataset::getMetadata() const
{
    return *m_metadata;
}

shared_ptr<DataInstance> Dataset::getSkeletonInstance(InstanceInfo info) const
{
    return getSkeletonInstance(info.getActivity(), info.getActor(), info.getSample());
}

shared_ptr<DataInstance> Dataset::getDepthInstance(InstanceInfo info) const
{
    return getDepthInstance(info.getActivity(), info.getActor(), info.getSample());
}

shared_ptr<DataInstance> Dataset::getColorInstance(InstanceInfo info) const
{
    return getColorInstance(info.getActivity(), info.getActor(), info.getSample());
}

shared_ptr<DataInstance> Dataset::getUserInstance(InstanceInfo info) const
{
    return getUserInstance(info.getActivity(), info.getActor(), info.getSample());
}

} // End Namespace
