#ifndef DAIUSERINSTANCE_H
#define DAIUSERINSTANCE_H

#include <fstream>
#include "dataset/DataInstance.h"
#include "types/UserFrame.h"

using namespace std;

namespace dai {

class DAIUserInstance : public DataInstance
{
public:
    struct BinaryUserFrame {
        u_int8_t userRow[640];
    };

    explicit DAIUserInstance(const InstanceInfo& info);
    virtual ~DAIUserInstance();
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
    shared_ptr<UserFrame> m_frameBuffer[2];
    BinaryUserFrame m_readBuffer[480];
};

} // End Namespace

#endif // DAIUSERINSTANCE_H
