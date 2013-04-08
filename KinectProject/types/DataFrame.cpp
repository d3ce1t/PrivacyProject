#include "DataFrame.h"

namespace dai {

DataFrame::DataFrame()
{
    m_index = -1;
}

void DataFrame::setIndex(int index)
{
    m_index = index;
}



} // End Namespace
