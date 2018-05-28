#include "keygenerator.h"

KeyGenerator::KeyGenerator(Keys *keys, QString *password)
    : mKeys(keys), mPassword(password) {}

KeyGenerator::~KeyGenerator() {}

void KeyGenerator::process() {

    mKeys->create(mPassword);
    emit finished();
}




