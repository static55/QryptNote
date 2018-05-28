#ifndef KEYCREATIONEXCEPTION_H
#define KEYCREATIONEXCEPTION_H

class KeyCreationException: public std::exception {

public:
    KeyCreationException() {}

    virtual const char* what() const throw() {
        return "Couldn't create keys.";
    }
};

#endif // KEYCREATIONEXCEPTION_H
