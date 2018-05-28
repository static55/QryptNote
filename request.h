#ifndef REQUEST_H
#define REQUEST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

#include <map>


using namespace std;

class Request : public QObject {
    Q_OBJECT

public:
    explicit Request(QUrl url = QUrl("http://m3.sdf.org/cgi-bin/qrypt.cgi"), QObject *parent = 0);
    void send();
    void addPair(string key, string val);

public slots:
    void onReply(QNetworkReply *reply);

signals:
    void newSalt(string salt);
    void newData(string data);
    void dataSaved();
    void noKeyOnServer();
    void keyReceived();
    void getFailed();
    void noData();

private:
    QNetworkAccessManager *mManager;
    map<string, string> mMap;
    QUrl mUrl;
};

#endif // REQUEST_H
