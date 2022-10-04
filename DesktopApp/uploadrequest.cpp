#include "uploadrequest.h"
#include <qnetworkaccessmanager.h>

uploadrequest::uploadrequest(QString userToken, QString signature, int patientId, int type, QString imageName, cv::Mat imageToSend, int captureNumber, QString patientName, UploadProgressDialog* uploadProgressDialog) {
	
    qDebug() << "uploadrequest";

    this->userToken = userToken;
    this->signature = signature;
    this->patientId = patientId;
    this->imageType = type;
    this->imageName = imageName;
    this->patientName = patientName;
    this->imageToSend = imageToSend;
    this->captureNumber = captureNumber;
    this->uploadProgressDialog = uploadProgressDialog;

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

    uploadProgressDialog->show();
    this->uploadNumber = ++uploadProgressDialog->latestUploadNumber;
    uploadProgressDialog->onUploading(patientName, captureNumber);

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

    receivedURL = reply->readAll();

    /** TIMEOUT */
    if (receivedURL == nullptr) {
        uploadProgressDialog->onFailed(this->uploadNumber);

        TwoLinesDialog dialog;
        dialog.setLine1("Analysis Step 1 Timeout!");
        dialog.exec();
        return;
    }
    /** TIMEOUT END */

    reply->deleteLater();

    if (receivedURL.contains("error")) {
        qCritical() << "onUploadImageNormally() received error reply!";
        TwoLinesDialog dialog;
        dialog.setLine1("Uploading Error 1");
        dialog.exec();
        return;
    }

    stageProgress = 1;
    bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void uploadrequest::bindImageUrl(const QObject* receiver, const char* member) {

    qDebug() << "bindImageUrl";

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/images"));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(10000);

    QJsonObject obj;
    obj["patientId"] = patientId;
    obj["url"] = receivedURL;
    obj["imageType"] = imageType;
    obj["imageName"] = imageName;

    qDebug() << "patientID: " << patientId << " imageType: " << imageType << " url: " << receivedURL;

    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);
}

void uploadrequest::onBindImageUrl(QNetworkReply* reply) {

    qDebug() << "onBindImageUrl";

    QByteArray response_data = reply->readAll();
    reply->deleteLater();

    qDebug() << response_data;

    /** TIMEOUT */
    if (response_data == nullptr) {
        TwoLinesDialog dialog;
        dialog.setLine1("Uploading Error 2");
        dialog.exec();
        return;
    }
    /** TIMEOUT END */

    stageProgress = 2;
    uploadProgressDialog->onCompleted(this->uploadNumber);
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