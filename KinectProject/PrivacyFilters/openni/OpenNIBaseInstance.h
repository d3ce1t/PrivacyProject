#ifndef OPENNIBASEINSTANCE_H
#define OPENNIBASEINSTANCE_H

namespace dai {

class OpenNIBaseInstance
{
public:
    OpenNIBaseInstance();
    unsigned int getSkippedFramesNumber() const;

protected:
    void computeStats(unsigned int frameIndex);

private:
    unsigned int m_lastFrameId;
    unsigned int m_skippedFrames;
};

} // End namespace

#endif // OPENNIBASEINSTANCE_H
