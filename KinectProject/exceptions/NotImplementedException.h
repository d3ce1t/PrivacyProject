#ifndef NOTIMPLEMENTEDEXCEPTION_H
#define NOTIMPLEMENTEDEXCEPTION_H

#include <exception>
using namespace std;

/*!
 * \brief The NotSupportedDataSetException class
 *
 * This exception is thrown when trying to load a DataSet that isn't supported
 */
class NotImplementedException : public exception
{
public:
    virtual const char* what() const throw() {
        return "This method isn't implemented yet";
    }
};

#endif // NOTIMPLEMENTEDEXCEPTION_H
