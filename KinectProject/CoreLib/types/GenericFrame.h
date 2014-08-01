#ifndef GENERICFRAME_H
#define GENERICFRAME_H

#include "DataFrame.h"
#include <stdint.h>

namespace dai {

template <class T, DataFrame::FrameType frameType>
class GenericFrame : public DataFrame
{
public:
    // Constructor, Destructors and Copy Constructor
    GenericFrame();
    GenericFrame(int width, int height);
    /**
     * Creates a GenericFrame using pData as storage data. This constructor does
     * not create memory, so the caller is responsible of destroying pData memory when
     * is finished.
     *
     * @brief GenericFrame
     * @param width
     * @param height
     * @param pData
     * @param type
     */
    GenericFrame(int width, int height, T* pData, int stride = 0);
    GenericFrame(const GenericFrame& other);
    virtual ~GenericFrame();
    shared_ptr<DataFrame> clone() const override;
    void setDataPtr(int width, int height, T* pData, int stride = 0);
    void setDataPtr(T* pData);
    shared_ptr<GenericFrame> subFrame(int row, int column, int width, int height) const;

    // Member Methods
    int getWidth() const;
    int getHeight() const;

    /**
     * Return the number of bytes that should be skipped to go to the first element of the next row
     * from the first element of the previous row.
     * @brief getStride
     * @return
     */
    int getStep() const;
    T getItem(int row, int column) const;
    T* getRowPtr(int row) const;
    const T* getDataPtr() const;
    void setItem(int row, int column, T value);

    // Overriden Operators
    GenericFrame& operator=(const GenericFrame& other);

    /**
     * Write the depth frame into the output stream
     * @brief write
     * @param of
     */
    //void write(QFile &of) const;


private:
    T*  m_data;
    int m_padding;
    int m_width;
    int m_height;
    bool m_managedData;
};

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame()
    : DataFrame(frameType)
    , m_managedData(false)
{
    this->m_data = nullptr;
    this->m_padding = 0;
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
    this->m_data = new T[width * height];
    memset(this->m_data, 0, width*height*sizeof(T));
    this->m_padding = 0;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame(int width, int height, T* pData, int stride)
    : DataFrame(frameType)
    , m_managedData(false)
{
    this->m_width = width;
    this->m_height = height;
    this->m_data = pData;
    this->m_padding = stride;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>::GenericFrame(const GenericFrame &other)
    : DataFrame(other)
{
    Q_ASSERT(other.m_padding == 0); // By now I do not allow copy with stride
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    this->m_data = new T[this->m_width * this->m_height];
    this->m_padding = other.m_padding;
    this->m_managedData = true;
    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
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
shared_ptr<DataFrame> GenericFrame<T, frameType>::clone() const
{
    return shared_ptr<GenericFrame<T, frameType>>(new GenericFrame<T, frameType>(*this));
}

template <class T, DataFrame::FrameType frameType>
void GenericFrame<T, frameType>::setDataPtr(int width, int height, T *pData, int stride)
{
    this->m_width = width;
    this->m_height = height;
    this->m_padding = stride;
    setDataPtr(pData);
}

template <class T, DataFrame::FrameType frameType>
inline void GenericFrame<T, frameType>::setDataPtr(T* pData)
{
    if (m_managedData && m_data != nullptr) {
        delete[] this->m_data;
        this->m_data = nullptr;
    }

    this->m_data = pData;
    this->m_managedData = false;
}

template <class T, DataFrame::FrameType frameType>
GenericFrame<T, frameType>& GenericFrame<T, frameType>::operator=(const GenericFrame<T,frameType>& other)
{
    Q_ASSERT(other.m_padding == 0); // By now I do not allow copy with stride

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
        }
    }

    this->m_managedData = true;
    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
    this->m_padding = other.m_padding;
    return *this;
}

template <class T, DataFrame::FrameType frameType>
shared_ptr<GenericFrame<T,frameType>> GenericFrame<T,frameType>::subFrame(int row, int column, int width, int height) const
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width ||
            row+height > this->m_height || column+width > this->m_width)
        throw 1;

    T* dataPtr = this->m_data + (row * this->m_width + column);
    int stride = this->m_width - width;
    return make_shared<GenericFrame<T,frameType>>(width, height, dataPtr, stride);
}

template <class T,DataFrame::FrameType frameType>
int GenericFrame<T,frameType>::getWidth() const
{
    return this->m_width;
}

template <class T,DataFrame::FrameType frameType>
int GenericFrame<T, frameType>::getHeight() const
{
    return this->m_height;
}

template <class T,DataFrame::FrameType frameType>
int GenericFrame<T, frameType>::getStep() const
{
    return (m_padding + m_width)*sizeof(T);
}

template <class T,DataFrame::FrameType frameType>
T GenericFrame<T,frameType>::getItem(int row, int column) const
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    return this->m_data[row * (this->m_width + this->m_padding) + column];
}

template <class T,DataFrame::FrameType frameType>
void GenericFrame<T,frameType>::setItem(int row, int column, T value)
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    this->m_data[row * (this->m_width + this->m_padding) + column] = value;
}

template <class T,DataFrame::FrameType frameType>
const T* GenericFrame<T,frameType>::getDataPtr() const
{
    return this->m_data;
}

template <class T,DataFrame::FrameType frameType>
T *GenericFrame<T, frameType>::getRowPtr(int row) const
{
     return m_data + row * (this->m_width + this->m_padding);
}

/*template <class T,DataFrame::FrameType frameType>
void GenericFrame<T,frameType>::write(QFile& of) const
{
    char* buffer = (char *) this->m_data;
    of.write(buffer, this->m_width * this->m_height * sizeof(T));
    of.flush();
}*/

} // End Namespace

#endif // GENERICFRAME_H
