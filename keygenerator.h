#ifndef KEYGENERATOR_H
#define KEYGENERATOR_H

#include <QString>
#include <QThread>
#include <QDebug>

#include "keys.h"

class KeyGenerator : public QObject {
    Q_OBJECT

public:
    KeyGenerator(Keys *keys, QString *);
    ~KeyGenerator();

public slots:
   void process();

signals:
   void finished();
   void error(QString string);

private:
    Keys *mKeys;
    QString *mPassword;
};

#endif // KEYGENERATOR_H
