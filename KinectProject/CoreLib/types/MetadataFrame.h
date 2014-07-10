#ifndef METADATAFRAME_H
#define METADATAFRAME_H

#include "DataFrame.h"
#include "BoundingBox.h"
#include <QList>

namespace dai {

class MetadataFrame : public DataFrame
{
public:
    MetadataFrame();
    MetadataFrame(const MetadataFrame& other);
    MetadataFrame& operator=(const MetadataFrame& other);
    shared_ptr<DataFrame> clone() const;

    // Methods
    void addBoundingBox(const BoundingBox& box);
    QList<BoundingBox>& boundingBoxes();

private:
    QList<BoundingBox> m_boundingBoxList;
};

} // End Namespace

#endif // METADATAFRAME_H
