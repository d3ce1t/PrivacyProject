#ifndef STREAMINSTANCE_H
#define STREAMINSTANCE_H

#include <QString>
#include "types/DataFrame.h"
#include "exceptions/NotOpenedInstanceException.h"
#include "exceptions/CannotOpenInstanceException.h"

namespace dai {

class StreamInstance
{
    unsigned int                  m_frame_counter;
    DataFrame::SupportedFrames    m_supportedFrames;
    QList< shared_ptr<DataFrame>> m_readFrames;

public:
    StreamInstance(DataFrame::SupportedFrames supportedFrames);
    StreamInstance(const StreamInstance& other) = delete;

    virtual void open();
    virtual void close();
    virtual void restart();
    virtual bool is_open() const = 0;
    virtual bool hasNext() const;

    inline void readNextFrame() {
        m_readFrames = nextFrame();
        m_frame_counter++;
    }

    inline QList< shared_ptr<DataFrame> > frames() {return m_readFrames;}

    unsigned int getFrameCounter() const;
    DataFrame::SupportedFrames getSupportedFrames() const;

protected:
    virtual bool openInstance() = 0;
    virtual void closeInstance() = 0;
    virtual void restartInstance() = 0;
    virtual QList< shared_ptr<DataFrame>> nextFrame() = 0;    
};

} // End namespace

#endif // STREAMINSTANCE_H
