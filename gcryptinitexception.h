#ifndef GCRYPTINITEXCEPTION_H
#define GCRYPTINITEXCEPTION_H

class GcryptInitException: public std::exception {

public:
    GcryptInitException() {}

    virtual const char* what() const throw() {
        return "Couldn't initialize gcrypt.";
    }
};

#endif // GCRYPTINITEXCEPTION_H
