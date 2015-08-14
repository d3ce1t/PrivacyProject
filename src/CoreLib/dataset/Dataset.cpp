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

shared_ptr<StreamInstance> Dataset::getInstance(int actor, int camera, int sample, const QList<QString>& labels, DataFrame::FrameType type) const
{
    return instance(actor, camera, sample, labels, type);
}

shared_ptr<StreamInstance> Dataset::getInstance(const InstanceInfo& info, DataFrame::FrameType type) const
{
    return instance(info.getActor(), info.getCamera(), info.getSample(), info.getLabels(), type);
}

} // End Namespace
