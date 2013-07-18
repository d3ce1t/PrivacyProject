#ifndef DAI_COLOR_INSTANCE_H
#define DAI_COLOR_INSTANCE_H

#include <fstream>
#include "dataset/DataInstance.h"
#include "types/ColorFrame.h"

using namespace std;

namespace dai {

class DAIColorInstance : public DataInstance
{
public:
    explicit DAIColorInstance(const InstanceInfo& info);
    virtual ~DAIColorInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DataFrame& frame) override;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    shared_ptr<ColorFrame> m_frameBuffer[2];
};

} // End Namespace

#endif // DAI_COLOR_INSTANCE_H
