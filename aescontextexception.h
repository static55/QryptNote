#ifndef AESCONTEXTEXCEPTION_H
#define AESCONTEXTEXCEPTION_H

class AesContextException: public std::exception {

public:
    AesContextException() {}

    virtual const char* what() const throw() {
        return "Error getting AES context.";
    }
};


#endif // AESCONTEXTEXCEPTION_H
