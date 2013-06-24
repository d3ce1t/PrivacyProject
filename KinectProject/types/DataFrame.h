#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <QList>

namespace dai {

class DataFrame
{
public:

    enum FrameType {
        Depth,
        Color,
        Skeleton
    };

    DataFrame(FrameType type);
    DataFrame(const DataFrame& other);
    void setIndex(int index);
    int getIndex() const;
    FrameType getType() const;

    // Overriden operators
    DataFrame& operator=(const DataFrame& other);

protected:
    int m_index;
    FrameType m_type;
};

} // End Namespace

#endif // DATAFRAME_H
