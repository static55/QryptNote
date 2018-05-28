#include "newkeyswindow.h"

NewKeysWindow::NewKeysWindow(Keys *keys) : mKeys(keys) {

    mUi.setupUi(this);
    setWindowIcon(QIcon(":lock.svg"));

    QPixmap pixMapX("x.png");
    QPixmap pixMapCheck("check.png");
    mUi.lineEditPassword->setEchoMode(QLineEdit::Password);
    mUi.lineEditPasswordAgain->setEchoMode(QLineEdit::Password);
    mUi.labelBottomIcon->setPixmap(pixMapX);

    QObject::connect(mUi.lineEditPasswordAgain, &QLineEdit::textEdited, this, &NewKeysWindow::onTextEdited);
    QObject::connect(mUi.buttonGenerateKeys, &QPushButton::clicked, this, &NewKeysWindow::onGenerateKeysClicked);
    QObject::connect(mUi.buttonClose, &QPushButton::clicked, this, &NewKeysWindow::onCloseClicked);
}

NewKeysWindow::~NewKeysWindow() {
    delete mKeyGenerator;
}

void NewKeysWindow::onTextEdited(const QString &text) {

    if (text == mUi.lineEditPassword->text())
        mUi.labelBottomIcon->setPixmap(*mPixMapCheck);
    else
        mUi.labelBottomIcon->setPixmap(*mPixMapX);
}

void NewKeysWindow::onGenerateKeysClicked() {

    if (mUi.lineEditPassword->text() != mUi.lineEditPasswordAgain->text()) {
        mUi.labelMessage->setText(QString("Passwords don't match."));
        return;
    }

    if (mUi.lineEditPassword->text() == "") {
        mUi.labelMessage->setText(QString("Enter a password."));
        return;
    }

    mUi.labelMessage->setText(QString("Be patient. This can take a minute."));
    mUi.lineEditPassword->setEnabled(false);
    mUi.lineEditPasswordAgain->setEnabled(false);
    mUi.buttonGenerateKeys->setEnabled(false);
    mPassword = mUi.lineEditPassword->text();

    mKeyGenerator = new KeyGenerator(mKeys, &mPassword);
    mKeyGenerator->moveToThread(&mKeyGeneratorThread);
    connect(&mKeyGeneratorThread, SIGNAL(started()), mKeyGenerator, SLOT(process()));
    connect(mKeyGenerator, SIGNAL(finished()), this, SLOT(onKeyGenerationComplete()));
    mKeyGeneratorThread.start();
}

void NewKeysWindow::onKeyGenerationComplete() {

    mUi.labelMessage->setText(QString("Finished!"));
}

void NewKeysWindow::onCloseClicked() {
    close();
}

void NewKeysWindow::errorString(QString string) {
    qInfo() << string;
}
