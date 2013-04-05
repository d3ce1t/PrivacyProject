#include "DepthFrame.h"

using namespace dai;

DepthFrame::DepthFrame(int width, int height)
{
    m_width = width;
    m_height = height;
    m_index = -1;
    m_data = new int[width * height];
}

DepthFrame::DepthFrame(const DepthFrame& other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_index = other.m_index;
    m_data = new int[m_width * m_height];

    int r,c;

    for(r=0; r<other.m_height; r++) {
        for(c=0; c<other.m_width; c++)
        {
            this->setItem(r,c, other.getItem(r, c));
        }
    }
}

DepthFrame& DepthFrame::operator=(const DepthFrame& other)
{
    if (m_data != 0) {
        delete[] m_data;
        m_data = 0;
    }

    m_width = other.m_width;
    m_height = other.m_height;
    m_index = other.m_index;
    m_data = new int[m_width * m_height];

    int r,c;

    for(r=0; r<other.m_height; r++) {
        for(c=0; c<other.m_width; c++)
        {
            this->setItem(r,c, other.getItem(r, c));
        }
    }

    return *this;
}

DepthFrame::~DepthFrame()
{
    if (m_data != 0) {
        delete[] m_data;
    }
}

int DepthFrame::getWidth() const
{
    return m_width;
}

int DepthFrame::getHeight() const
{
    return m_height;
}

int DepthFrame::getItem(int row, int column) const
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    return m_data[row * m_width + column] ;
}

void DepthFrame::setIndex(int index)
{
    m_index = index;
}

void DepthFrame::setItem(int row, int column, int value)
{
    if (row < 0 || row >= m_height || column < 0 || column >= m_width )
        throw 1;

    m_data[row * m_width + column] = value;
}


int DepthFrame::maxValue() const
{
    int r,c;
    int bestValue = 0;

    for(r=0; r<m_height; r++)
    {
        for(c=0; c<m_width; c++)
        {
            int temp = getItem(r,c);

            if(temp > bestValue)
            {
                bestValue = temp;
            }
        }
    }
    return bestValue;
}

int DepthFrame::minValue() const
{
    int r,c;
    int bestValue = 4096;

    for(r=0; r<m_height; r++)
    {
        for(c=0; c<m_width; c++)
        {
            int temp = getItem(r,c);

            if(temp != 0 && temp < bestValue)
            {
                bestValue = temp;
            }
        }
    }

    return bestValue;
}

/*
int Sample::NumberOfNonZeroPoints() const
{
    int count=0;
    int r,c;
    for(r=0; r<GetNRows(); r++)
    {
        for(c=0; c<GetNCols(); c++)
        {
            int temp = GetItem(r,c);
            if(temp != 0)
            {
                count++;
            }
        }
    }
    return count;
}

float Sample::AvgNonZeroDepth() const
{
    int count=0;
    float sum = 0;
    int r,c;
    for(r=0; r<GetNRows(); r++)
    {
        for(c=0; c<GetNCols(); c++)
        {
            int temp = GetItem(r,c);
            if(temp != 0)
            {
                count++;
                sum += temp;
            }
        }
    }
    if(count == 0)
        return 0;
    else
        return sum/count;
}
*/
