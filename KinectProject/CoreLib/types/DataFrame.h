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
        Depth    = 0x01,
        Color    = 0x02,
        Skeleton = 0x04,
        User     = 0x08
    };
    Q_DECLARE_FLAGS(SupportedFrames, FrameType)

    bool static isColorSupported(const FrameType& type);

    // Constructor
    DataFrame(FrameType type);
    DataFrame(const DataFrame& other);

    // Destructor
#if (!defined _MSC_VER)
    ~DataFrame() = default;
#endif

    virtual shared_ptr<DataFrame> clone() const = 0;
    void setIndex(unsigned int index);
    unsigned int getIndex() const;
    FrameType getType() const;

    // Overriden operators
    DataFrame& operator=(const DataFrame& other);

protected:
    unsigned int m_index;
    FrameType m_type;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(DataFrame::SupportedFrames)

typedef QHash<DataFrame::FrameType, shared_ptr<DataFrame>> QHashDataFrames;

} // End Namespace

#endif // DATAFRAME_H
