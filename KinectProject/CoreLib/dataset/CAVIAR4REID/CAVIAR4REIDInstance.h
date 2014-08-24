#ifndef CAVIAR4REID_INSTANCE_H
#define CAVIAR4REID_INSTANCE_H

#include "dataset/DataInstance.h"
#include "types/ColorFrame.h"
#include <opencv2/opencv.hpp>

namespace dai {

class CAVIAR4REIDInstance : public DataInstance
{
    int                     m_width;
    int                     m_height;
    cv::Mat                 m_frameBuffer;
    shared_ptr<ColorFrame>  m_colorFrame;

public:
    explicit CAVIAR4REIDInstance(const InstanceInfo& info);
    virtual ~CAVIAR4REIDInstance();
    bool is_open() const override;
    bool hasNext() const override;

protected:
    bool openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(QHashDataFrames& output) override;
};

} // End namespace

#endif // CAVIAR4REID_INSTANCE_H
