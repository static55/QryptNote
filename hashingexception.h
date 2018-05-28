#ifndef HASHINGEXCEPTION_H
#define HASHINGEXCEPTION_H

class HashingException: public std::exception {

public:
    HashingException() {}

    virtual const char* what() const throw() {
        return "Couldn't create hash.";
    }
};


#endif // HASHINGEXCEPTION_H
