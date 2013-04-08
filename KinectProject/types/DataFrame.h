#ifndef DATAFRAME_H
#define DATAFRAME_H

namespace dai {

class DataFrame
{
public:
    DataFrame();
    void setIndex(int index);
    int getIndex() {return m_index;}

protected:
    int m_index;
};

} // End Namespace

#endif // DATAFRAME_H
