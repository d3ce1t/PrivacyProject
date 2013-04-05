#ifndef NOTSUPPORTEDDATASETEXCEPTION_H
#define NOTSUPPORTEDDATASETEXCEPTION_H

#include <exception>
using namespace std;

/*!
 * \brief The NotSupportedDataSetException class
 *
 * This exception is thrown when trying to load a DataSet that isn't supported
 */
class NotSupportedDatasetException : public exception
{
public:
    virtual const char* what() const throw() {
        return "Trying to load a not supported dataset";
    }
};

#endif // NOTSUPPORTEDDATASETEXCEPTION_H
