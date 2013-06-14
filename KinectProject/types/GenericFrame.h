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
    GenericFrame();
    GenericFrame(int width, int height);
    virtual ~GenericFrame();
    GenericFrame(const GenericFrame<T>& other);

    // Member Methods
    int getWidth() const;
    int getHeight() const;
    T getItem(int row, int column) const;
    short int getLabel(int row, int column) const;
    const T* getDataPtr() const;
    void setItem(int row, int column, T &value);
    void setItem(int row, int column, T &value, short int label);

    // Overriden Operators
    GenericFrame<T>& operator=(const GenericFrame<T>& other);

    /**
     * Write the depth frame into the output stream
     * @brief write
     * @param of
     */
    void write(QFile &of, bool writeLabels) const;


private:
    T* m_data;
    short int* m_label;
    int m_width;
    int m_height;
};

template <class T>
GenericFrame<T>::GenericFrame()
{
    this->m_data = 0;
    this->m_label = 0;
    this->m_width = 0;
    this->m_height = 0;
}

template <class T>
GenericFrame<T>::GenericFrame(int width, int height)
{
    this->m_width = width;
    this->m_height = height;
    this->m_data = new T[width * height];
    this->m_label = new short int[width * height];
    memset(this->m_data, 0, width*height*sizeof(T));
    memset(this->m_label, 0, width*height*sizeof(short int));
}

template <class T>
GenericFrame<T>::GenericFrame(const GenericFrame<T> &other)
    : DataFrame(other)
{
    this->m_width = other.m_width;
    this->m_height = other.m_height;
    this->m_data = new T[this->m_width * this->m_height];
    this->m_label = new short int[this->m_width * this->m_height];
    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
    memcpy(this->m_label, other.m_label, this->m_width * this->m_height * sizeof(short int));
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
        this->m_label = new short int[this->m_width * this->m_height];
    }

    memcpy(this->m_data, other.m_data, this->m_width * this->m_height * sizeof(T));
    memcpy(this->m_label, other.m_label, this->m_width * this->m_height * sizeof(short int));
    return *this;
}

template <class T>
GenericFrame<T>::~GenericFrame()
{
    if (this->m_data != 0) {
        delete[] this->m_data;
    }

    if (this->m_label != 0) {
        delete[] this->m_label;
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
short int GenericFrame<T>::getLabel(int row, int column) const
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    return this->m_label[row * this->m_width + column];
}

template <class T>
void GenericFrame<T>::setItem(int row, int column, T &value)
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    this->m_data[row * this->m_width + column] = value;
}

template <class T>
void GenericFrame<T>::setItem(int row, int column, T &value, short int label)
{
    if (row < 0 || row >= this->m_height || column < 0 || column >= this->m_width )
        throw 1;

    int index = row * this->m_width + column;
    this->m_data[index] = value;
    this->m_label[index] = label;
}

template <class T>
const T* GenericFrame<T>::getDataPtr() const
{
    return this->m_data;
}

template <class T>
void GenericFrame<T>::write(QFile& of, bool writeLabels = true) const
{
    char* buffer = (char *) this->m_data;
    of.write(buffer, this->m_width * this->m_height * sizeof(T));

    if (writeLabels) {
        buffer = (char *) this->m_label;
        of.write(buffer, this->m_width * this->m_height * sizeof(short int));
    }

    of.flush();
}

} // End Namespace

#endif // GENERICFRAME_H
