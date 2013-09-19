#ifndef NOTOPENEDINSTANCEEXCEPTION_H
#define NOTOPENEDINSTANCEEXCEPTION_H

#include <exception>
using namespace std;


/*!
 * \brief The NotOpenedInstanceException class
 *
 * This exception is thrown when trying to read from a not opened instance
 */
class NotOpenedInstanceException : public exception
{
public:
    virtual const char* what() const throw() {
        return "Trying to perform a read into a not opened instance";
    }
};

#endif // NOTOPENEDINSTANCEEXCEPTION_H
