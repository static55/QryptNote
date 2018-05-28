#include <QApplication>

#include "mainwindow.h"
#include "newkeyswindow.h"
#include "trayicon.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    Keys keys;
    MainWindow mainWindow;
    mainWindow.setKeys(&keys);
    NewKeysWindow newKeysWindow(&keys);
    mainWindow.mNewKeysWindow = &newKeysWindow;


    TrayIcon trayIcon(&mainWindow);

    return a.exec();
}
