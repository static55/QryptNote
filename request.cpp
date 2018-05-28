#include "request.h"
#include <unistd.h>

Request::Request(QUrl url, QObject *parent) : QObject(parent), mUrl(url) {}
void Request::addPair(string key, string val) { mMap.insert(make_pair(key, val)); }

void Request::send() {

    mManager = new QNetworkAccessManager(this);
    connect(mManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onReply(QNetworkReply*)));

    QNetworkRequest request(mUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // turn map pairs into HTTP POST variables (var1=val1&var2=val2&, etc, etc..)
    string data;
    map<string, string>::iterator i;
    for(i = mMap.begin(); i != mMap.end(); i++) {
        if (i != mMap.begin()) {
            data += "&";
        }
        data += i->first;
        data += "=";
        data += i->second;
    }



    mManager->post(request, QByteArray(data.c_str()));
}

void Request::onReply(QNetworkReply *reply) {

    if (reply->error())
        qDebug() << reply->errorString();

    QList<QByteArray> lines = reply->readAll().split('\n');
    for(int i = 0; i < lines.count(); i++) {
        if (lines.at(i).startsWith("salt:")) {
            QByteArray salt = lines.at(i).right(lines.at(i).size()-5);
            emit newSalt(salt.toStdString());
        } else if (lines.at(i).startsWith("data:")) {
            QByteArray data = lines.at(i).right(lines.at(i).size()-5);
            emit newData(data.toStdString());
        } else if (lines.at(i).startsWith("saved")) {
            emit dataSaved();
        } else if (lines.at(i).startsWith("nokey")) {
            emit noKeyOnServer();
        } else if (lines.at(i).startsWith("getfail")) {
            emit getFailed();
        } else if (lines.at(i).startsWith("keyreceived")) {
            emit keyReceived();
        } else if (lines.at(i).startsWith("nodata")) {
            emit noData();
        }else {
            qInfo() << lines.at(i);
        }
    }
    reply->deleteLater();
}
