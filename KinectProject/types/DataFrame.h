#ifndef DATAFRAME_H
#define DATAFRAME_H

namespace dai {

class DataFrame
{
public:
    DataFrame();
    DataFrame(const DataFrame& other);
    void setIndex(int index);
    int getIndex() const {return m_index;}

    // Overriden operators
    DataFrame& operator=(const DataFrame& other);

protected:
    int m_index;
};

} // End Namespace

#endif // DATAFRAME_H
