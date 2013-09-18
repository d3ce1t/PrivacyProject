#ifndef NOTIMPLEMENTEDEXCEPTION_H
#define NOTIMPLEMENTEDEXCEPTION_H

#include <exception>
using namespace std;

/*!
 * \brief The NotImplementedException class
 *
 * This exception is thrown when invoking a not implemented method
 */
class NotImplementedException : public exception
{
public:
    virtual const char* what() const throw() {
        return "This method isn't implemented yet";
    }
};

#endif // NOTIMPLEMENTEDEXCEPTION_H
