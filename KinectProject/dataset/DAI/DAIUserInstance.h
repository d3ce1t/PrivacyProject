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
    bool is_open() const Q_DECL_OVERRIDE;
    UserFrame& frame() Q_DECL_OVERRIDE;

protected:
    void openInstance() Q_DECL_OVERRIDE;
    void closeInstance() Q_DECL_OVERRIDE;
    void restartInstance() Q_DECL_OVERRIDE;
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;

private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    UserFrame   m_frameBuffer[2];
    BinaryUserFrame m_readBuffer[480];
};

} // End Namespace

#endif // DAIUSERINSTANCE_H
