#ifndef CANNOTOPENINSTANCEEXCEPTION_H
#define CANNOTOPENINSTANCEEXCEPTION_H

#include <exception>
using namespace std;

/*!
 * \brief The NotOpenedInstanceException class
 *
 * This exception is thrown when trying to read from a not opened instance
 */
class CannotOpenInstanceException : public exception
{
public:
    virtual const char* what() const throw() {
        return "Cannot open instance exception";
    }
};

#endif // CANNOTOPENINSTANCEEXCEPTION_H
