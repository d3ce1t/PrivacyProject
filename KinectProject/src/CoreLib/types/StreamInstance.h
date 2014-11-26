#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <QString>
#include "types/DataFrame.h"
#include "exceptions/NotOpenedInstanceException.h"
#include "exceptions/CannotOpenInstanceException.h"

namespace dai {

class StreamInfo
{
public:
    int width;  // -1) undetermined, 0) do not have width
    int height; // -1) undetermined, 0) do not have height
};

class StreamInstance
{
    unsigned int                m_frame_counter;
    DataFrame::SupportedFrames  m_supportedFrames;
    StreamInfo                  m_info;

public:

    static QList<DataFrame::FrameType> getTypes(DataFrame::SupportedFrames type);

    StreamInstance(DataFrame::SupportedFrames supportedFrames);
    StreamInstance(DataFrame::SupportedFrames supportedFrames, int width, int height);
    StreamInstance(const StreamInstance& other) = delete;

    virtual void open();
    virtual void close();
    virtual void restart();
    virtual bool is_open() const = 0;
    virtual bool hasNext() const;

    inline void readNextFrame(QHashDataFrames& frames) {
        nextFrame(frames);
        m_frame_counter++;
    }

    const StreamInfo& getStreamInfo() const;
    unsigned int getFrameCounter() const;
    DataFrame::SupportedFrames getSupportedFrames() const;

protected:
    virtual bool openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual void nextFrame(QHashDataFrames& output) = 0;
};

} // End namespace

#endif // STREAMINSTANCE_H
