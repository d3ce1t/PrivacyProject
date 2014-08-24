#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <QList>
#include <memory>
#include <QHash>
#include <QFlags>

using namespace std;

namespace dai {

class DataFrame
{
public:
    enum FrameType {
        Unknown  = 0x00,
        Color    = 0x01,
        Depth    = 0x02,
        Skeleton = 0x04,
        Mask     = 0x08,
        Metadata = 0x16
    };
    Q_DECLARE_FLAGS(SupportedFrames, FrameType)

    static shared_ptr<DataFrame> create(FrameType type, int width = 0, int height = 0);

    DataFrame(FrameType type);
    DataFrame(const DataFrame& other);
    virtual shared_ptr<DataFrame> clone() const = 0;
    DataFrame& operator=(const DataFrame& other);
    void setIndex(unsigned int index);
    unsigned int getIndex() const;
    FrameType getType() const;

protected:
    unsigned int m_index;
    FrameType m_type;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DataFrame::SupportedFrames)

typedef QHash<DataFrame::FrameType, shared_ptr<DataFrame>> QHashDataFrames;

} // End Namespace

#endif // DATAFRAME_H
