#include "uploadrequest.h"
#include <qnetworkaccessmanager.h>
#include <twolinesdialog.h>

uploadrequest::uploadrequest(QString token, QString sign, int id, cv::Mat image) {
	qDebug() << "uploadrequest";

    userToken = token;
	signature = sign;
	patientId = id;
	imageToSend = image;

    uploadImageNormally(this, SLOT(onUploadImageNormally(QNetworkReply*)));

   /* if (signature == "") {
        getSignature();
    }*/
}
void uploadrequest::getSignature() {

    qDebug() << "getSignature()";
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QEventLoop eventLoop;
    QNetworkRequest request(QUrl("https://api.conovamed.com/api/v1/file/aliyunOssSignature"));
    request.setRawHeader("Authorization",    this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSignatureReceived(QNetworkReply*)));

    manager->get(request);
}

void uploadrequest::onSignatureReceived(QNetworkReply* reply) {
    
    QByteArray response_data = reply->readAll();
    reply->deleteLater();

    qDebug() << response_data;

    /** TIMEOUT */
    if (response_data == nullptr) {
        TwoLinesDialog dialog;
        dialog.setLine1("Networking Error: Get Signature Timeout!");
        dialog.exec();
        return;
    }
    /** TIMEOUT END */

    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
    QJsonArray jsonArray = jsonResponse.array();

    //qDebug() << jsonResponse;
    qDebug() << jsonResponse["accessId"];
    qDebug() << jsonResponse["host"];
    qDebug() << jsonResponse["policy"];
    qDebug() << jsonResponse["signature"];
    qDebug() << jsonResponse["dir"];

    uploadImageToAliyun(this, SLOT(onUploadImageToAliyunCompleted(QNetworkReply*)), jsonResponse);
}


void uploadrequest::uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument json) {

    qDebug() << "uploadImageToAliyun: " << imageToSend.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString(json["host"].toString())));
    //request.setRawHeader("Authorization", this->userToken.toUtf8());
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"policy\""));
    textPart.setBody(json["policy"].toString().toUtf8());

    QHttpPart textPart2;
    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"OSSAccessKeyId\""));
    textPart2.setBody(json["accessId"].toString().toUtf8());

    QHttpPart textPart3;
    textPart3.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"success_action_status\""));
    textPart3.setBody(QString("200").toUtf8());

    QHttpPart textPart4;
    textPart4.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"signature\""));
    textPart4.setBody(json["signature"].toString().toUtf8());

    QHttpPart textPart5;
    textPart5.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
    textPart5.setBody(QString("/wukong/test.png").toUtf8());

    multipart->append(textPart5);
    multipart->append(textPart);
    //multipart->append(textPart3);
    multipart->append(textPart2);
    multipart->append(textPart4);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"test.png\""));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QImage qImage((const uchar*)imageToSend.data, imageToSend.cols, imageToSend.rows, imageToSend.step, QImage::Format_RGBA64);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);

    //multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, multipart);
}

void uploadrequest::onUploadImageToAliyunCompleted(QNetworkReply* reply) {

    qDebug() << "onUploadImageToAliyunCompleted";

    QByteArray response_data = reply->readAll();
    reply->deleteLater();

    qDebug() << response_data;
}

void uploadrequest::uploadImageNormally(const QObject* receiver, const char* member) {
    qDebug() << "uploadImage: " << imageToSend.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://qa.mosainet.com/sm-api/api/v1/upload")));
    request.setRawHeader("Authorization", this->userToken.toUtf8());
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"test.png\""));
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QImage qImage((const uchar*)imageToSend.data, imageToSend.cols, imageToSend.rows, imageToSend.step, QImage::Format_RGBA64);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);
    multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, multipart);
}

void uploadrequest::onUploadImageNormally(QNetworkReply* reply) {

    qDebug() << "onUploadImageNormally";

    QString url = reply->readAll();

    /** TIMEOUT */
    if (url == nullptr) {
        TwoLinesDialog dialog;
        dialog.setLine1("Analysis Step 1 Timeout!");
        dialog.exec();
        return;
    }
    /** TIMEOUT END */

    qDebug() << url;

    reply->deleteLater();

    if (url.contains("error")) {
        qCritical() << "onUploadImage received error reply!";
        TwoLinesDialog dialog;
        dialog.setLine1("onUploadImage received error reply!");
        dialog.exec();
        return;
    }

    //QNetworkClient::getInstance().bindImageUrl(this->parent->patientTab->getCurrentPatientId(), url, this->imageTypeBeingAnalyzed, this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void uploadrequest::debugRequest(QNetworkRequest request, QByteArray data = QByteArray())
{
    qDebug() << request.url().toString();
    const QList<QByteArray>& rawHeaderList(request.rawHeaderList());
    foreach(QByteArray rawHeader, rawHeaderList) {
        qDebug() << request.rawHeader(rawHeader);
    }
    qDebug() << data;
}