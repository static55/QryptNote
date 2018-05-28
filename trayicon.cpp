#include "trayicon.h"

TrayIcon::TrayIcon(MainWindow *wdo) : mMainWindow(wdo)  {

    mShowAction = new QAction(tr("&Show"), this);
    connect(mShowAction, SIGNAL(triggered()), this, SLOT(showWindow()));
    mQuitAction = new QAction(tr("&Quit"), this);
    connect(mQuitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    mTrayIconMenu = new QMenu();
    mTrayIconMenu->addAction(mShowAction);
    mTrayIconMenu->addSeparator();
    mTrayIconMenu->addAction(mQuitAction);

    mTrayIcon = new QSystemTrayIcon(this);
    mTrayIcon->setIcon(QIcon(":lock.svg"));
    mTrayIcon->setContextMenu(mTrayIconMenu);

    connect(mTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    mTrayIcon->show();
}

void TrayIcon::iconActivated(QSystemTrayIcon::ActivationReason reason) {

    if (reason == QSystemTrayIcon::Trigger) {

        if (!mMainWindow->getKeys()->existOnDisk()) {
            mMainWindow->mNewKeysWindow->showNormal();
            mMainWindow->mNewKeysWindow->raise();
            mMainWindow->mNewKeysWindow->activateWindow();
        } else {

            if (mMainWindow->isVisible())
                mMainWindow->close();
            else
                showWindow();
        }
    }
}

void TrayIcon::showWindow() {

    QSize size = qApp->screens()[0]->size();
    mMainWindow->setGeometry(
                size.width()-mMainWindow->width(),
                mMainWindow->height(),
                mMainWindow->width(),
                mMainWindow->height());
    mMainWindow->showNormal();
    mMainWindow->raise();
    mMainWindow->activateWindow();
}
