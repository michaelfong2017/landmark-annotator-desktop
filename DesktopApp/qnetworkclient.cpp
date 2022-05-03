#include "qnetworkclient.h"
#include "stdafx.h"

QNetworkClient::QNetworkClient() : QWidget() {
}

QNetworkAccessManager* QNetworkClient::getManager()
{
	return manager;
}

void QNetworkClient::login() {
    // Login

    manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/account/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["account"] = "isl512gp@gmail.com";
    obj["password"] = "123456";
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onLogin(QNetworkReply*)));

    manager->post(request, data);

    // returns userToken, needs to be stored
}
void QNetworkClient::onLogin(QNetworkReply* reply) {
    qDebug() << reply->readAll();

    reply->deleteLater();
}
