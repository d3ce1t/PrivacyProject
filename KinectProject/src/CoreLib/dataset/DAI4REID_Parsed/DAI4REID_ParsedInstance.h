#ifndef DAI4REID_PARSED_INSTANCE_H
#define DAI4REID_PARSED_INSTANCE_H

#include "dataset/DataInstance.h"

namespace dai {

class DAI4REID_ParsedInstance : public DataInstance
{
    int  m_width;
    int  m_height;
    bool m_open;

public:
    explicit DAI4REID_ParsedInstance(const InstanceInfo& info);
    virtual ~DAI4REID_ParsedInstance();
    bool is_open() const override;
    bool hasNext() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;
};

} // End namespace

#endif // DAI4REID_PARSED_INSTANCE_H
