#ifndef OPENNIBASEINSTANCE_H
#define OPENNIBASEINSTANCE_H

namespace dai {

class OpenNIBaseInstance
{
public:
    OpenNIBaseInstance();

protected:
    void computeStats(unsigned int frameIndex);

private:
    unsigned int    m_lastFrameId;
};

} // End namespace

#endif // OPENNIBASEINSTANCE_H
