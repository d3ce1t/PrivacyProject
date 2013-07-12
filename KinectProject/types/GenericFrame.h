#ifndef GENERICFRAME_H
#define GENERICFRAME_H

#include "DataFrame.h"
#include <stdint.h>
#include <QMap>
#include <QFile>

namespace dai {

template <class T>
class GenericFrame : public DataFrame
{
public:
    // Constructor, Destructors and Copy Constructor
    GenericFrame(FrameType type);
    GenericFrame(int width, int height, FrameType type);
    virtual ~GenericFrame();
    GenericFrame(const GenericFrame<T>& other);
    GenericFrame<T>* clone() const;

    // Member Methods
    int getWidth() const;
    int getHeight() const;
    T getItem(int row, int column) const;
    const T* getDataPtr() const;
    void setItem(int row, int column, T &value);

    // Overriden Operators
    GenericFrame<T>& operator=(const GenericFrame<T>& other);

    /**
     * Write the depth frame into the output stream
     * @brief write
     * @param of
     */
    void write(QFile &of) const;


private:
    T* m_data;
    int m_width;
    int m_height;
};

template <class T>
GenericFrame<T>::GenericFrame(FrameType type)
    : DataFrame(type)
{
    this->m_data = 0;
    this->m_width = 0;
    this->m_height = 0;
}

template <class T>
GenericFrame<T>::GenericFrame(int width, int height, FrameType type)
    : DataFrame(type)
{
    this->m_width = width;
    this->m_height = height;
    this->m_data = new T[width * height];
    memset(this->m_data, 0, width*height*sizeof(T));
}

template <class T>
GenericFrame<T>::GenericFrame(const GenericFrame<T> &other)
    : DataFrame(other)
{
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    this->m_data = new T[this->m_width * this->m_height];
    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
}

template <class T>
GenericFrame<T>* GenericFrame<T>::clone() const
{
    GenericFrame<T>* clonedObject = new GenericFrame<T>(*this);
    return clonedObject;
}

template <class T>
GenericFrame<T>& GenericFrame<T>::operator=(const GenericFrame<T>& other)
{
    DataFrame::operator=(other);

    // If want to reuse m_data memory. So, if size isn't correct to store new frame
    // I need to create another one.
    if (this->m_data == 0 || this->m_width != other.m_width || this->m_height != other.m_height)
    {
        if (this->m_data != 0) {
            delete[] this->m_data;
        }

        this->m_width = other.m_width;
        this->m_height = other.m_height;
        this->m_data = new T[this->m_width * this->m_height];
    }

    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
    return *this;
}

template <class T>
GenericFrame<T>::~GenericFrame()
{
    if (this->m_data != 0) {
        delete[] this->m_data;
    }
}

template <class T>
int GenericFrame<T>::getWidth() const
{
    return this->m_width;
}

template <class T>
int GenericFrame<T>::getHeight() const
{
    return this->m_height;
}

template <class T>
T GenericFrame<T>::getItem(int row, int column) const
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    return this->m_data[row * this->m_width + column];
}

template <class T>
void GenericFrame<T>::setItem(int row, int column, T &value)
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    this->m_data[row * this->m_width + column] = value;
}

template <class T>
const T* GenericFrame<T>::getDataPtr() const
{
    return this->m_data;
}

template <class T>
void GenericFrame<T>::write(QFile& of) const
{
    char* buffer = (char *) this->m_data;
    of.write(buffer, this->m_width * this->m_height * sizeof(T));
    of.flush();
}

} // End Namespace

#endif // GENERICFRAME_H
