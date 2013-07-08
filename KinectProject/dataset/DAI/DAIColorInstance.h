#ifndef DAI_COLOR_INSTANCE_H
#define DAI_COLOR_INSTANCE_H

#include <iostream>
#include <fstream>
#include "dataset/DataInstance.h"
#include "types/ColorFrame.h"
#include <stdint.h>

using namespace std;

namespace dai {

class DAIColorInstance : public DataInstance
{
public:

    struct BinaryColorFrame {
        RGBAColor colorRow[640];
    };

    explicit DAIColorInstance(const InstanceInfo& info);
    virtual ~DAIColorInstance();
    bool is_open() const Q_DECL_OVERRIDE;
    ColorFrame& frame() Q_DECL_OVERRIDE;

protected:
    void openInstance() Q_DECL_OVERRIDE;
    void closeInstance() Q_DECL_OVERRIDE;
    void restartInstance() Q_DECL_OVERRIDE;
    void nextFrame(DataFrame& frame) Q_DECL_OVERRIDE;


private:
    ifstream    m_file;
    int         m_width;
    int         m_height;
    ColorFrame  m_frameBuffer[2];
    BinaryColorFrame m_readBuffer[480];
};

} // End Namespace

#endif // DAI_COLOR_INSTANCE_H