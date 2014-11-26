#include "MetadataFrame.h"

namespace dai {

MetadataFrame::MetadataFrame()
    : DataFrame(DataFrame::Metadata)
{
}

MetadataFrame::MetadataFrame(const MetadataFrame& other)
    : DataFrame(other)
{
    m_boundingBoxList = other.m_boundingBoxList; // Implicit sharing
}

MetadataFrame& MetadataFrame::operator=(const MetadataFrame& other)
{
    DataFrame::operator=(other);
    m_boundingBoxList = other.m_boundingBoxList; // Implicit sharing
    return *this;
}

shared_ptr<DataFrame> MetadataFrame::clone() const
{
    return shared_ptr<DataFrame>(new MetadataFrame(*this));
}

void MetadataFrame::addBoundingBox(const BoundingBox& box)
{
    m_boundingBoxList.append(box);
}

QList<BoundingBox>& MetadataFrame::boundingBoxes()
{
    return m_boundingBoxList;
}

} // End Namespace
