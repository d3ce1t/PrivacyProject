#include "Dataset.h"

namespace dai {

Dataset::Dataset(QString path)
{
    m_metadata = DatasetMetadata::load(path);
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

DataInstance* Dataset::getSkeletonInstance(InstanceInfo info)
{
    return getSkeletonInstance(info.getActivity(), info.getActor(), info.getSample());
}

DataInstance* Dataset::getDepthInstance(InstanceInfo info)
{
    return getDepthInstance(info.getActivity(), info.getActor(), info.getSample());
}

DataInstance &Dataset::getColorInstance(InstanceInfo info)
{
    return getColorInstance(info.getActivity(), info.getActor(), info.getSample());
}

} // End Namespace