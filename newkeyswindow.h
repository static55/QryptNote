#ifndef NEWKEYSWINDOW_H
#define NEWKEYSWINDOW_H

#include <QDialog>
#include <QThread>
#include <QPixmap>
#include <QDebug>

#include "keys.h"
#include "keygenerator.h"
#include "ui_newkeyswindow.h"


class NewKeysWindow : public QDialog {

    Q_OBJECT

public:
    NewKeysWindow(Keys *keys);
    ~NewKeysWindow();
    Keys *mKeys;

public slots:
    void onTextEdited(const QString &text);
    void onGenerateKeysClicked();
    void onCloseClicked();
    void onKeyGenerationComplete();
    void errorString(QString string);

signals:
    void finished();

private:
    Ui::NewKeysWindow mUi;
    QPixmap *mPixMapX = new QPixmap("x.png");
    QPixmap *mPixMapCheck = new QPixmap("check.png");
    QThread mKeyGeneratorThread;
    KeyGenerator *mKeyGenerator;
    QString mPassword;

public slots:
};

#endif // NEWKEYSWINDOW_H
