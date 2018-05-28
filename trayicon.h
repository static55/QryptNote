#ifndef TRAYICON_H
#define TRAYICON_H

#include <QMenu>
#include <QSystemTrayIcon>
#include <QApplication>
#include <QScreen>

#include <mainwindow.h>

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    TrayIcon(MainWindow *wdo);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void showWindow();


private:

    MainWindow *mMainWindow;
    QAction *mShowAction;
    QAction *mQuitAction;
    QSystemTrayIcon *mTrayIcon;
    QMenu *mTrayIconMenu;
};

#endif // TRAYICON_H
