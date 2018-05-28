#ifndef SIGNINGEXCEPTION_H
#define SIGNINGEXCEPTION_H

class SigningException: public std::exception {

public:
    SigningException() {}

    virtual const char* what() const throw() {
        return "Error somewhere in Keys::sign()";
    }
};

#endif // SIGNINGEXCEPTION_H
