#include "keys.h"

#include <QDebug>

Keys::Keys() {}

void Keys::dumpState() {
    qInfo() << "pubKey64 : " << string(getPublicKeyBase64()).substr(0, 20).c_str();
    qInfo() << "privKey64: " << string(getPrivateKeyBase64()).substr(0, 20).c_str();
}

int Keys::init(QString password) {

        if (!loadFromDisk(password)) // if incorrect password
            return 0;

    mPublicKey = gcry_sexp_find_token(mKeys, "public-key", 0);
    mPrivateKey = gcry_sexp_find_token(mKeys, "private-key", 0);

    if (mKeys == NULL || mPublicKey == NULL || mPrivateKey == NULL)
        throw NoKeysLoadedException();
    return 1;
}

Keys::~Keys() {

    for (uint i = 0; i < mAllocatedStrings.size(); i++)
        delete mAllocatedStrings[i];

    gcry_sexp_release(mKeys);
    gcry_sexp_release(mPublicKey);
    gcry_sexp_release(mPrivateKey);
}

string Keys::getPublicKeyBase64() { return sexpToBase64(mPublicKey); }
string Keys::getPrivateKeyBase64() { return sexpToBase64(mPrivateKey); }

string Keys::calcHashBinary(char *data, int dataLen) {

    gcry_md_hd_t md;
    if (gcry_md_open(&md, GCRY_MD_TIGER, 0))
        throw HashingException();

    gcry_md_write(md, data, dataLen);
    int mdSize = gcry_md_get_algo_dlen(GCRY_MD_TIGER);
    if (mdSize == 0)
        throw HashingException();

    char *mdBuf;
    mdBuf = (char *)gcry_md_read(md, GCRY_MD_TIGER);
    char *mdBuf64 = (char *)calloc(1, base64_enc_len(mdSize+1));
    base64_encode(mdBuf64, mdBuf, mdSize);

    string hash = mdBuf64;
    free(mdBuf64);
    gcry_md_close(md);
    return hash;
}

string Keys::calcHash(string str) {

    if (str.size() == 0)
        throw HashingException();

    return calcHashBinary((char *)str.c_str(), str.size());
}

string Keys::sign(string str) {

    if (str.length() == 0)
        throw SigningException();

    string hash = calcHash(str);
    gcry_mpi_t hashMsg;
    if (gcry_mpi_scan(&hashMsg, GCRYMPI_FMT_USG, hash.c_str(), hash.size(), NULL))
        throw SigningException();

    // mpi message -> sexp message
    gcry_sexp_t hashSexp;
    if (gcry_sexp_build(&hashSexp, NULL, "(data (flags raw) (value %m))", hashMsg))
        throw SigningException();

    gcry_sexp_t sig;
    gcry_pk_sign(&sig, hashSexp, mPrivateKey);
    string buffer64 = sexpToBase64(sig);

    gcry_mpi_release(hashMsg);
    gcry_sexp_release(hashSexp);
    gcry_sexp_release(sig);
    return buffer64;
}

//gcry_sexp_t Keys::getPublicKey() { return mPublicKey; }
//gcry_sexp_t Keys::getPrivateKey() { return mPrivateKey; }

// return value needs to be free()d
char *Keys::sexpToBinary(gcry_sexp_t sexp, int *length) {

    *length = gcry_sexp_sprint(sexp, GCRYSEXP_FMT_CANON, NULL, 0);
    char *sexpBuf = (char *)calloc(1, *length);
    gcry_sexp_sprint(sexp, GCRYSEXP_FMT_CANON, sexpBuf, *length);
    return sexpBuf;
}

// return value needs to be free()d
char *Keys::getPublicKeyBinary(int *length) { return sexpToBinary(mPublicKey, length); }

string Keys::sexpToBase64(gcry_sexp_t sexp) {

    int sexpLen = gcry_sexp_sprint(sexp, GCRYSEXP_FMT_CANON, NULL, 0);
    char *sexpBuf = (char *)calloc(1, sexpLen);//new char[sexpLen];
    gcry_sexp_sprint(sexp, GCRYSEXP_FMT_CANON, sexpBuf, sexpLen);
    int sexp64Len = base64_enc_len(sexpLen);

    char sexp64[sexp64Len+1];
    base64_encode(sexp64, sexpBuf, sexpLen);

    string *retString = new string(sexp64);
    mAllocatedStrings.push_back(retString);
    free(sexpBuf);
    return *retString;
}

bool Keys::loadFromDisk(QString password) {

    gcry_cipher_hd_t aesHd;
    createAesContext(&aesHd, &password);
    std::ifstream keys(KEYS_FILE_NAME, std::ifstream::binary | std::ifstream::in);

    if (!keys.is_open())
        throw KeyLoadingException();

    char keyBuf[KEY_BITS];
    keys.read(keyBuf, KEY_BITS);
    keys.close();

    if (gcry_cipher_decrypt(aesHd, keyBuf, KEY_BITS, NULL, 0))
        throw KeyLoadingException();
    if (strncmp("(8:key-data", keyBuf, 11) != 0)
        return false;

    if (gcry_sexp_new(&mKeys, keyBuf, KEY_BITS, 0))
    throw KeyLoadingException();
    return true;
}

string Keys::encrypt(string plainText) {

    gcry_mpi_t plainTextMpi;
    gcry_mpi_scan(&plainTextMpi, GCRYMPI_FMT_USG, plainText.c_str(), plainText.size(), NULL);
    gcry_sexp_t plainTextSexp;
    gcry_sexp_build(&plainTextSexp, NULL, "(data (flags raw) (value %m))", plainTextMpi);

    gcry_sexp_t cipherTextSexp;
    gcry_pk_encrypt(&cipherTextSexp, plainTextSexp, mPublicKey);

    int bufLen = gcry_sexp_sprint(cipherTextSexp, GCRYSEXP_FMT_CANON, NULL, 0);
    char *buf = (char *)calloc(1, bufLen);
    gcry_sexp_sprint(cipherTextSexp, GCRYSEXP_FMT_CANON, buf, bufLen);

    int buf64Len = base64_enc_len(bufLen);
    char buf64[buf64Len+1];
    base64_encode(buf64, buf, bufLen);
    string retString = string(buf64);

    free(buf64);
    free(buf);
    gcry_mpi_release(plainTextMpi);
    gcry_sexp_release(plainTextSexp);
    gcry_sexp_release(cipherTextSexp);

    return retString;
}

string Keys::decrypt(string cipherText) {

    int bufLen = base64_dec_len((char *)cipherText.c_str(), cipherText.size());
    char buf[bufLen];
    base64_decode(buf, (char *)cipherText.c_str(), cipherText.size());
    gcry_sexp_t cipherTextSexp;
    gcry_sexp_new(&cipherTextSexp, buf, bufLen, 0);
    free(buf);
    gcry_sexp_t plainTextSexp;
    gcry_pk_decrypt(&plainTextSexp, cipherTextSexp, mPrivateKey);
    int length = gcry_sexp_sprint(plainTextSexp, GCRYSEXP_FMT_CANON, NULL, 0);
    gcry_mpi_t outMpi = gcry_sexp_nth_mpi(plainTextSexp, 0, GCRYMPI_FMT_USG);

    char *plainBuf = (char *)calloc(1, length);

    gcry_mpi_print(GCRYMPI_FMT_USG, (unsigned char *)plainBuf, length, NULL, outMpi);

    if (strcmp(plainBuf, "hello world!") == 0)
        qInfo("decrypted result was the same as plaintext input.");

    string retString(plainBuf);
    gcry_mpi_release(outMpi);
    gcry_sexp_release(cipherTextSexp);
    gcry_sexp_release(plainTextSexp);
    free(plainBuf);

    return retString;
}



bool Keys::existOnDisk() {
    return (access(KEYS_FILE_NAME, F_OK) != -1);
}

void Keys::createAesContext(gcry_cipher_hd_t *aesHd, QString *qStringPassword) {

    if (qStringPassword == nullptr)
        qStringPassword = new QString("abc123derpherp2949"); // for testing? forgot...

    const char *HASH_SALT = "7b79158d01f032h0ff5a2238ca7d03e4891c5f93c21bfcd09cc7cbafd1c6ff98";
    const int AES_KEY_SIZE_BYTES = 16;
    char passwordHash[AES_KEY_SIZE_BYTES];
    int saltedPasswordLen = strlen(HASH_SALT) + qStringPassword->size();
    char *saltedPassword = new char[saltedPasswordLen+1];
    strcpy(saltedPassword, qStringPassword->toStdString().c_str());
    strcat(saltedPassword, HASH_SALT);

    if (gcry_cipher_open(aesHd, GCRY_CIPHER_AES128  , GCRY_CIPHER_MODE_CFB, 0))
        throw AesContextException();

    gcry_md_hash_buffer(GCRY_MD_MD5, &passwordHash, saltedPassword, saltedPasswordLen);
    if (gcry_cipher_setkey(*aesHd, &passwordHash, AES_KEY_SIZE_BYTES))
        throw AesContextException();
    if (gcry_cipher_setiv(*aesHd, &passwordHash, AES_KEY_SIZE_BYTES))
        throw AesContextException();

    if (qStringPassword == QString("abc123derpherp2949"))
        delete qStringPassword;

    delete saltedPassword;
}

void Keys::create(QString *password) {

    gcry_sexp_t params;

    if (gcry_sexp_build(&params, NULL, "(genkey (rsa (nbits 4:4096)))"))
        throw KeyCreationException();
    if (gcry_pk_genkey(&mKeys, params))
        throw KeyCreationException();

    char *rsaBuf = (char *)calloc(1, KEY_BITS);
    gcry_sexp_sprint(mKeys, GCRYSEXP_FMT_CANON, rsaBuf, KEY_BITS);

    gcry_cipher_hd_t aesHd;
    createAesContext(&aesHd, password);
    if (gcry_cipher_encrypt(aesHd, rsaBuf, KEY_BITS, NULL, 0))
        throw KeyCreationException();

    std::ofstream keys(KEYS_FILE_NAME, std::ifstream::binary);
    if (!keys.is_open())
        throw KeyCreationException();

    keys.write(rsaBuf, KEY_BITS);

    keys.close();
    gcry_sexp_release(params);
    free(rsaBuf);
}

void Keys::initGcrypt() {

    if (!gcry_check_version(GCRYPT_VERSION))
        throw GcryptInitException();
    if (gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN))
        throw GcryptInitException();
    if (gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0))
        throw GcryptInitException();
    if (gcry_control(GCRYCTL_RESUME_SECMEM_WARN))
        throw GcryptInitException();
    if (gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0))
        throw GcryptInitException();
}


