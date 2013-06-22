#ifndef DATAFRAME_H
#define DATAFRAME_H

#include <QList>

namespace dai {

class DataFrame
{
public:
    DataFrame();
    DataFrame(const DataFrame& other);
    void setIndex(int index);
    int getIndex() const;

    // Overriden operators
    DataFrame& operator=(const DataFrame& other);

protected:
    int m_index;
};

typedef QList<DataFrame*> DataFrameList;

} // End Namespace

#endif // DATAFRAME_H
