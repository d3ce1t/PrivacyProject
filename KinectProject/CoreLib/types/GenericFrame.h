#ifndef GENERICFRAME_H
#define GENERICFRAME_H

#include "DataFrame.h"
#include "types/Point.h"
#include "types/BoundingBox.h"
#include <stdint.h>

namespace dai {

template <class T, DataFrame::FrameType frameType>
class GenericFrame : public DataFrame
{
    T*   m_data;
    uint m_stride; // Size of a row in bytes
    int m_width;
    int m_height;
    bool m_managedData;
    Point2i m_offset;

public:
    // Constructor, Destructors and Copy Constructor
    GenericFrame();
    GenericFrame(int width, int height);
    /**
     * Creates a GenericFrame using pData as storage data. This constructor does
     * not create memory, so the caller is responsible of destroying pData memory when
     * is finished.
     */
    GenericFrame(int width, int height, T* pData, uint stride = 0);
    virtual ~GenericFrame();
    GenericFrame(const GenericFrame& other);
    GenericFrame& operator=(const GenericFrame& other);
    virtual shared_ptr<DataFrame> clone() const override;

    void setDataPtr(int width, int height, const T* pData, uint stride);
    void setDataPtr(int width, int height, const T* pData);
    void setItem(int row, int column, T value);
    void setOffset(const Point2i& offset) {m_offset = offset;}

    shared_ptr<GenericFrame> subFrame(int row, int column, int width, int height) const;

    shared_ptr<GenericFrame> subFrame(const BoundingBox& bb) const {
        return subFrame(bb.getMin()[1], bb.getMin()[0], bb.size().width(), bb.size().height());
    }

    int width() const {return m_width;}
    int height() const {return m_height;}
    const Point2i& offset() const {return m_offset;}

    /**
     * Return the number of bytes that should be skipped to go to the first element of the next row
     * from the first element of the previous row.
     */
    uint getStride() const {return m_stride;}
    T getItem(int row, int column) const;
    T* getRowPtr(int row) const;
    const T* getDataPtr() const;

private:
    void setDataPtr(const T* pData);
};

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame()
    : DataFrame(frameType)
    , m_managedData(false)
{
    this->m_data = nullptr;
    this->m_stride = 0;
    this->m_width = 0;
    this->m_height = 0;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame(int width, int height)
    : DataFrame(frameType)
    , m_managedData(true)
{
    this->m_width = width;
    this->m_height = height;

    if (width > 0 && height > 0) {
        this->m_data = new T[width * height];
        this->m_stride = width * sizeof(T);
        memset(this->m_data, 0, width*height*sizeof(T));
    } else {
        this->m_data = nullptr;
        this->m_stride = 0;
    }
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame(int width, int height, T* pData, uint stride)
    : DataFrame(frameType)
    , m_managedData(false)
{
    this->m_width = width;
    this->m_height = height;
    this->m_data = pData;
    this->m_stride = stride;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame(const GenericFrame &other)
    : DataFrame(other)
{
    m_width = other.m_width;
    m_height = other.m_height;
    m_data = new T[m_width * m_height];

    // No padding
    if (other.m_stride == other.m_width * sizeof(T)) {
        m_stride = other.m_stride;
        memcpy(m_data, other.m_data, m_stride * m_height);
    }
    // Padding, so I have to read each row
    else {
        m_stride = m_width * sizeof(T);
        for (int i=0; i<other.height(); ++i) {
            T* src = other.getRowPtr(i);
            T* dst = this->getRowPtr(i);

            for (int j=0; j<other.width(); ++j)
                dst[j] = src[j]; // Copy    // FIX: I could copy row by row
        }
    }

    this->m_managedData = true;
    this->m_offset = other.m_offset;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>& GenericFrame<T, frameType>::operator=(const GenericFrame<T,frameType>& other)
{
    Q_ASSERT(other.m_stride == other.m_width * sizeof(T)); // By now I do not allow assign with stride

    DataFrame::operator=(other);

    // If want to reuse m_data memory. So, if size isn't correct to store new frame
    // I need to create another one.
    if (!this->m_data || this->m_width != other.m_width || this->m_height != other.m_height)
    {
        this->m_width = other.m_width;
        this->m_height = other.m_height;

        if (this->m_data) {
            delete [] this->m_data;
            this->m_data = new T[this->m_width * this->m_height];
            this->m_managedData = true;
        }
    }

    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
    this->m_stride = other.m_stride;
    this->m_offset = other.m_offset;
    return *this;
}

template <class T, DataFrame::FrameType frameType>
shared_ptr<DataFrame> GenericFrame<T, frameType>::clone() const
{
    return shared_ptr<GenericFrame<T, frameType>>(new GenericFrame<T, frameType>(*this));
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::~GenericFrame()
{
    if (m_managedData && m_data != nullptr) {
        delete[] this->m_data;
        this->m_data = nullptr;
    }
}

template <class T, DataFrame::FrameType frameType>
void GenericFrame<T, frameType>::setDataPtr(int width, int height, const T *pData, uint stride)
{
    this->m_width = width;
    this->m_height = height;
    this->m_stride = stride;
    setDataPtr(pData);
}

template <class T, DataFrame::FrameType frameType>
void GenericFrame<T, frameType>::setDataPtr(int width, int height, const T *pData)
{
    this->m_width = width;
    this->m_height = height;
    this->m_stride = width*sizeof(T);
    setDataPtr(pData);
}

template <class T, DataFrame::FrameType frameType>
inline void GenericFrame<T, frameType>::setDataPtr(const T* pData)
{
    if (m_managedData && m_data != nullptr) {
        delete[] this->m_data;
        this->m_data = nullptr;
    }

    this->m_data = const_cast<T*>(pData);
    this->m_managedData = false;
}

template <class T, DataFrame::FrameType frameType>
shared_ptr<GenericFrame<T,frameType>> GenericFrame<T,frameType>::subFrame(int row, int column, int width, int height) const
{
    Q_ASSERT(row >= 0 && row < this->m_height && column >= 0 && column < this->m_width &&
            row+height <= this->m_height && column+width <= this->m_width);

    uchar* dataPtr = (uchar*) this->m_data;
    dataPtr += row * this->m_stride + column * sizeof(T);

    shared_ptr<GenericFrame> result = make_shared<GenericFrame>(width, height, (T*) dataPtr, this->m_stride);
    result->m_offset[0] = column;
    result->m_offset[1] = row;
    return result;
}

template <class T,DataFrame::FrameType frameType>
T GenericFrame<T,frameType>::getItem(int row, int column) const
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    uchar* ptr = (uchar*) this->m_data;
    ptr += row * this->m_stride + column * sizeof(T);
    return *((T*) ptr);
}

template <class T,DataFrame::FrameType frameType>
void GenericFrame<T,frameType>::setItem(int row, int column, T value)
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    uchar* ptr = (uchar*) this->m_data;
    ptr += row * this->m_stride + column * sizeof(T);
    *((T*) ptr) = value;
}

template <class T,DataFrame::FrameType frameType>
const T* GenericFrame<T,frameType>::getDataPtr() const
{
    return this->m_data;
}

template <class T,DataFrame::FrameType frameType>
T *GenericFrame<T, frameType>::getRowPtr(int row) const
{
    uchar* ptr = (uchar*) this->m_data;
    ptr += row * this->m_stride;
    return (T*) ptr;
}

} // End Namespace

#endif // GENERICFRAME_H
