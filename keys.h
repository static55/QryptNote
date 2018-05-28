#ifndef KEYS_H
#define KEYS_H

#include "gcrypt.h"
#include "base64.h"

#include <QFile>

#include <fstream>
#include <sys/stat.h>
//#include <stdint.h>

#include "gcryptinitexception.h"
#include "keycreationexception.h"
#include "keyloadingexception.h"
#include "nokeysloadedexception.h"
#include "signingexception.h"
#include "hashingexception.h"
#include "aescontextexception.h"


#define KEYS_FILE_NAME "rsapair.key"
#define KEY_BITS 4096

using namespace std;

class Keys {

private:
    gcry_sexp_t mKeys = NULL;
    gcry_sexp_t mPublicKey = NULL;
    gcry_sexp_t mPrivateKey = NULL;
    vector<string *> mAllocatedStrings;

    void testKeyStuff();
    void initGcrypt();
    bool loadFromDisk(QString password);
    char *sexpToBinary(gcry_sexp_t sexp, int *length);
    string sexpToBase64(gcry_sexp_t sexp);
    void createAesContext(gcry_cipher_hd_t *aesHd, QString *qStringPassword = nullptr);


public:
    Keys();
    ~Keys();
    string getPublicKeyBase64();
    string getPrivateKeyBase64();
    char *getPublicKeyBinary(int *length);
    string sign(string str);
    string calcHash(string str);
    string calcHashBinary(char *data, int dataLen);
    bool existOnDisk();
    void create(QString *password);
    int init(QString password);
    bool initialized();
    void dumpState();
//    gcry_sexp_t getPublicKey();
//    gcry_sexp_t getPrivateKey();
    string encrypt(string plainText);
    string decrypt(string cipherText);


};

#endif // KEYS_H
