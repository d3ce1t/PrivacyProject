#include "Dataset.h"

namespace dai {

Dataset::Dataset(QString path)
{
    m_metadata = DatasetMetadata::load(path);
}

const DatasetMetadata& Dataset::getMetadata() const
{
    return *m_metadata;
}

} // End Namespace
