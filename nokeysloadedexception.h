#ifndef NOKEYSLOADEDEXCEPTION_H
#define NOKEYSLOADEDEXCEPTION_H

class NoKeysLoadedException: public std::exception {

public:
    NoKeysLoadedException() {}

    virtual const char* what() const throw() {
        return "mRsaKeys is NULL.";
    }
};


#endif // NOKEYSLOADEDEXCEPTION_H
