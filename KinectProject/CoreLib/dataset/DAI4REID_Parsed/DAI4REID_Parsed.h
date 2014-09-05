#ifndef DAI4REID_PARSED_H
#define DAI4REID_PARSED_H

#include "dataset/Dataset.h"

namespace dai {

class DAI4REID_Parsed : public Dataset
{
public:
    DAI4REID_Parsed();

protected:
    shared_ptr<StreamInstance> instance(int actor, int camera, int sample, const QList<QString>& label, DataFrame::FrameType type) const;
};

}

#endif // DAI4REID_PARSED_H
