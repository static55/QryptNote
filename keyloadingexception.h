#ifndef KEYLOADINGEXCEPTION_H
#define KEYLOADINGEXCEPTION_H

class KeyLoadingException: public std::exception {

public:
    KeyLoadingException() {}

    virtual const char* what() const throw() {
        return "Couldn't load keys from disk.";
    }
};

#endif // KEYLOADINGEXCEPTION_H
