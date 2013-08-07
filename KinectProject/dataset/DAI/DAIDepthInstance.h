#ifndef DAIDEPTHINSTANCE_H
#define DAIDEPTHINSTANCE_H

#include <fstream>
#include "types/DataInstance.h"
#include "types/DepthFrame.h"

using namespace std;

namespace dai {

class DAIDepthInstance : public DataInstance<DepthFrame>
{
public:
    struct BinaryDepthFrame {
        float depthRow[640];
    };

    explicit DAIDepthInstance(const InstanceInfo& info);
    virtual ~DAIDepthInstance();
    bool is_open() const override;

protected:
    void openInstance() override;
    void closeInstance() override;
    void restartInstance() override;
    void nextFrame(DepthFrame& frame) override;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    shared_ptr<DepthFrame> m_frameBuffer[2];
    BinaryDepthFrame m_readDepthBuffer[480];
};

} // End Namespace

#endif // DAIDEPTHINSTANCE_H
