#include "Dataset.h"

namespace dai {

Dataset::Dataset(QString path)
{
    m_metadata = DatasetMetadata::load(path);
    m_metadata->setDataset(this);
}

Dataset::~Dataset()
{
    if (m_metadata != NULL) {
        delete m_metadata;
        m_metadata = NULL;
    }
}

const DatasetMetadata& Dataset::getMetadata() const
{
    return *m_metadata;
}

DataInstance* Dataset::getSkeletonInstance(InstanceInfo info) const
{
    return getSkeletonInstance(info.getActivity(), info.getActor(), info.getSample());
}

DataInstance* Dataset::getDepthInstance(InstanceInfo info) const
{
    return getDepthInstance(info.getActivity(), info.getActor(), info.getSample());
}

DataInstance* Dataset::getColorInstance(InstanceInfo info) const
{
    return getColorInstance(info.getActivity(), info.getActor(), info.getSample());
}

} // End Namespace
