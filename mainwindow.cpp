#include "mainwindow.h"

#include <QTextStream>

MainWindow::MainWindow() {

    mUi.setupUi(this);
    mUi.stackedWidget->setCurrentIndex(0);
    mUi.passwordLineEdit->setEchoMode(QLineEdit::Password);
    mUi.plainTextEdit->installEventFilter(this);
    setWindowIcon(QIcon(":lock.svg"));
}

MainWindow::~MainWindow() {

    for (uint i = 0; i < mRequests.size(); i++)
        delete mRequests[i];
}

void MainWindow::setKeys(Keys *keys) {
    mKeys = keys;
}




void MainWindow::cryptTest() {

    qInfo() << "begin test";
    mKeys->init(QString("ddd"));

    string crypted = mKeys->encrypt(string("hello world!"));
    string decrypted = mKeys->decrypt(crypted);
}

Keys *MainWindow::getKeys() {
    return mKeys;
}

unsigned long int MainWindow::timeInMillis() {

    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

bool MainWindow::eventFilter(QObject *target, QEvent *event) {

    if (target == mUi.plainTextEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = nullptr;
            keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_S || keyEvent->key() == Qt::Key_X)
                && (keyEvent->modifiers() & Qt::ControlModifier)) {
                keyPressEvent(keyEvent);
                return true;
            }
        }
    }
    return QDialog::eventFilter(target, event);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {

    if (event->key() == Qt::Key_S && (event->modifiers() & Qt::ControlModifier)) {
        setWindowTitle("QryptNote (Saving. )");
        Request *request = new Request();
        QObject::connect(request, &Request::newSalt, this, &MainWindow::onNewSalt);
        mState = STATE_SAVING;
        request->addPair("cmd", "salt");
        request->send();
        mRequests.push_back(request);
    }

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        mUi.connectButton->click();

    if (event->key() == Qt::Key_X && (event->modifiers() & Qt::ControlModifier)) {
        if (mState == STATE_IDLE) {
            unsigned long int timeNow = timeInMillis();
            if ((timeNow - mLastCtrlXTime) < 500) // ctrl-xx?
                close();
            mLastCtrlXTime = timeNow;
        }
    }
}

void MainWindow::dumpState() {

    if (mKeys == nullptr)
        qInfo() << "mKeys : null";
    else
        mKeys->dumpState();

    qInfo() << "mSalt :" << mSalt.substr(0, 20).c_str();
    qInfo() << "mData :" << mData.substr(0, 20).c_str();

    if (mState == STATE_LOADING)
        qInfo() << "mState: STATE_LOADING";
    else if (mState == STATE_SAVING)
        qInfo() << "mState: STATE_SAVING";
    else
        qInfo() << "mState: unset";
}

void MainWindow::on_connectButton_clicked() {

    if (!mKeys->init(mUi.passwordLineEdit->text())) {
        onGetFailed();
        return;
    }
    mUi.labelError->setText(QString("Connecting (Requesting salt)..."));

    Request *request = new Request();
    QObject::connect(request, &Request::newSalt, this, &MainWindow::onNewSalt);
    QObject::connect(request, &Request::noKeyOnServer, this, &MainWindow::onNoKeyOnServer);

    mState = STATE_LOADING;
    request->addPair("cmd", "salt");
    request->send();

    mRequests.push_back(request);
}

void MainWindow::onNoKeyOnServer() {

    mState = STATE_SENDING_KEY;
    mUi.labelError->setText(QString("No key on server. Sending key..."));
    Request *request = new Request();
    QObject::connect(request, &Request::keyReceived, this, &MainWindow::onKeyReceived);
    request->addPair("key", mKeys->getPublicKeyBase64());
    request->send();
    mRequests.push_back(request);
}

void MainWindow::onKeyReceived() {
    mUi.labelError->setText(QString("Server has key. Click \"Connect\" again."));
    mState = STATE_IDLE;
}


void MainWindow::onNewSalt(string salt) {

    mSalt = salt;
    Request *request = new Request();
    QObject::connect(request, &Request::newData, this, &MainWindow::onNewData);
    QObject::connect(request, &Request::noData, this, &MainWindow::onNoData);
    QObject::connect(request, &Request::dataSaved, this, &MainWindow::onDataSaved);
    QObject::connect(request, &Request::getFailed, this, &MainWindow::onGetFailed);

    if (mState == STATE_LOADING) {
            mUi.labelError->setText(QString("Connecting (Requesting data)..."));
            request->addPair("cmd", "get");
            request->addPair("sig", mKeys->sign(mSalt));
    } else if (mState == STATE_SAVING) {
            setWindowTitle("QryptNote (Saving..)");
            string data = mKeys->encrypt(mUi.plainTextEdit->document()->toRawText().toStdString());
            request->addPair("cmd", "put");
            request->addPair("data", data);
            request->addPair("dataHash", mKeys->calcHash(data + mSalt));
            request->addPair("sig", mKeys->sign(data + mSalt));
    }
    request->send();
    mRequests.push_back(request);

    //            // the following is used for the CGI script's unit tests
    //            request->addPair("cmd", "unitTests");
    //            request->addPair("test1", "hello world!");
    //            request->addPair("test1Hash", mKeys.calcHash("hello world!"));
    //            request->addPair("key", mKeys.getPublicKeyBase64());
    //            request->addPair("keyHash", mKeys.calcHash(mKeys.getPublicKeyBase64()));
    //            request->addPair("saltHash", mKeys.calcHash(mSalt));
    //            int binKeyLen;
    //            char *keyBinary = mKeys.getPublicKeyBinary(&binKeyLen);
    //            request->addPair("binaryKeyHash", mKeys.calcHashBinary(keyBinary, binKeyLen));
    //            free(keyBinary);
    //            request->addPair("dataHash", mKeys.calcHash("hello world!" + mSalt));
    //            request->addPair("data", "hello world!");
    //            request->addPair("sig", mKeys.sign("hello world!" + mSalt));
    //            // end CGI unit test stuff
}

void MainWindow::onTextChanged() {
    setWindowTitle("QryptNote (Modified: Ctrl-s to save)");
}

void MainWindow::onGetFailed() {
    mUi.labelError->setText(QString("Error: Wrong password or key mismatch."));
    mState = STATE_IDLE;
}

void MainWindow::onNewData(string data) {

    // some stuff here is repeated in below onNoData().. too lazy to fix
    mData = mKeys->decrypt(data);
    mUi.verticalLayout_2->setMargin(0);
    mUi.plainTextEdit->document()->setPlainText(mData.c_str());
    setWindowTitle("QryptNote (Saved: Ctrl-xx to close)");
    if (mUi.stackedWidget->currentIndex() == 0)
        QObject::connect(mUi.plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    mUi.horizontalLayout->setMargin(0);
    mUi.stackedWidget->setCurrentIndex(1);
    mState = STATE_IDLE;
}

void MainWindow::onNoData() {

    // some stuff here is repeated in above onNewData().. too lazy to fix
    mData = "";
    mUi.verticalLayout_2->setMargin(0);
    mUi.plainTextEdit->document()->setPlainText("No data on server. Ctrl-s to save.");
    setWindowTitle("QryptNote (Ctrl-s to save)");
    if (mUi.stackedWidget->currentIndex() == 0)
        QObject::connect(mUi.plainTextEdit, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    mUi.stackedWidget->setCurrentIndex(1);
    mState = STATE_IDLE;
}


void MainWindow::onDataSaved() {
    setWindowTitle("QryptNote (Saved: Ctrl-xx to close)");
    mState = STATE_IDLE;
}

void MainWindow::closeEvent (QCloseEvent *event) {

    mUi.plainTextEdit->document()->clear();
    mUi.plainTextEdit->document()->clearUndoRedoStacks();
    mUi.stackedWidget->setCurrentIndex(0);
    mUi.passwordLineEdit->clear();
    mUi.hostLineEdit->clear();
    event->accept();
}

void MainWindow::showEvent( QShowEvent* event ) {

    setWindowTitle("QryptNote");
    mUi.labelError->setText(QString(""));
    QWidget::showEvent( event );
    mUi.passwordLineEdit->setFocus();
    cryptTest();

}
