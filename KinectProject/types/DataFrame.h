#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <QList>
#include <memory>
#include <QHash>

using namespace std;

namespace dai {

class DataFrame
{
public:

    enum FrameType {
        Depth,
        Color,
        Skeleton,
        User
    };

    DataFrame(FrameType type);
    DataFrame(const DataFrame& other);
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

typedef QHash<DataFrame::FrameType,shared_ptr<DataFrame>> QHashDataFrames;

} // End Namespace

#endif // DATAFRAME_H
