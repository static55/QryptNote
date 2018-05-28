#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sys/time.h>

#include <QDialog>
#include <QIcon>
#include <QShortcut>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QByteArray>

#include "ui_mainwindow.h"
#include "request.h"
#include "keys.h"
#include <newkeyswindow.h>


using namespace std;

namespace Ui { class MainWindow; }

class MainWindow : public QDialog {

    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent* event);
    void setKeys(Keys *keys);
    Keys *getKeys();
    NewKeysWindow *mNewKeysWindow;
    void dumpState();



protected:
    virtual void keyPressEvent(QKeyEvent* pe);

private slots:
    void on_connectButton_clicked();
    void onNewSalt(string salt);
    void onNewData(string data);
    void onNoData();
    void onTextChanged();
    void onDataSaved();
    void onNoKeyOnServer();
    void onGetFailed();
    void onKeyReceived();






private:

    Ui::MainWindow mUi;
    Keys *mKeys;
    vector<Request *> mRequests;
    string mSalt;
    string mData;
    bool eventFilter(QObject *target, QEvent *event);
    void clickConnectButton();
    unsigned long int timeInMillis();
    void cryptTest();

    enum State { STATE_SAVING, STATE_LOADING, STATE_IDLE, STATE_SENDING_KEY } mState;


    unsigned long int mLastCtrlXTime;

};

#endif // MAINWINDOW_H
