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

shared_ptr<BaseInstance> Dataset::getInstance(InstanceInfo info) const
{
    return getInstance(info.getActivity(), info.getActor(), info.getSample(), info.getType());
}

} // End Namespace
