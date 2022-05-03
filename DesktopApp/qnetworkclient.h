#ifndef QNETWORKCLIENT_H
#define QNETWORKCLIENT_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include <QtNetwork>
#include <QtWidgets/QWidget>


class QNetworkClient : public QWidget
{
    Q_OBJECT

public:
    static QNetworkClient& getInstance() {
        static QNetworkClient instance;
        return instance;
    }

    QNetworkClient(QNetworkClient const&) = delete;

    void operator=(QNetworkClient const&) = delete;

    QNetworkAccessManager* getManager();

    void login();

private:
    QNetworkClient();
    QNetworkAccessManager* manager;

private slots:
    void onLogin(QNetworkReply* reply);
};

#endif
