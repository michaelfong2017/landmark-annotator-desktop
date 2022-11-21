#include "uploadrequest.h"
#include <qnetworkaccessmanager.h>
#include <objbase.h>  

#define GUID_LEN 64 

unsigned char random_char() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return static_cast<unsigned char>(dis(gen));
}

std::string generate_hex(const unsigned int len) {
    std::stringstream ss;
    for (auto i = 0; i < len; i++) {
        auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << int(rc);
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

uploadrequest::uploadrequest(QString userToken, QString signature, int patientId, int type, QString imageName, cv::Mat imageToSend, cv::Mat imageToSend2, cv::Mat imageToSend3, int captureNumber, QString patientName, UploadProgressDialog* uploadProgressDialog) {
	
    qDebug() << "uploadrequest";

    // Read host address
    readHostAddress();
    // Read host address END

    std::string s = generate_hex(32);
    qDebug() << QString::fromUtf8(s.c_str());

    this->userToken = userToken;
    this->signature = signature;
    this->patientId = patientId;
    this->imageType = type;
    this->imageName = QString::fromUtf8(s.c_str());
    this->patientName = patientName;
    this->imageToSend = imageToSend;
    this->imageToSend2 = imageToSend2;
    this->imageToSend3 = imageToSend3;
    this->captureNumber = captureNumber;
    this->uploadProgressDialog = uploadProgressDialog;
    this->uploadNumber = uploadProgressDialog->latestUploadNumber;

}

void uploadrequest::retry(int uploadNumber) {
    reuploadNormally(uploadNumber, this, SLOT(onUploadImageNormally(QNetworkReply*)));
}

void uploadrequest::reuploadNormally(int uploadNumber, const QObject* receiver, const char* member) {
    qDebug() << "re-uploadImage: " << imageToSend.channels();
    uploadProgressDialog->show();
    uploadProgressDialog->onUploading(uploadNumber);

    /*QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://api.conovamed.com/api/v1/upload")));
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

    manager->post(request, multipart);*/

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QEventLoop eventLoop;
    QNetworkRequest request(QUrl(hostAddress + "api/v1/file/aliyunOssSignature?dir=wukong&expireln=600"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/api/v1/file/aliyunOssSignature?dir=wukong&expireln=600"));
    request.setRawHeader("Authorization", this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSignatureReceived(QNetworkReply*)));

    manager->get(request);
}

void uploadrequest::getSignature() {

    qDebug() << "getSignature()";

    uploadProgressDialog->show();
    uploadProgressDialog->addRow(this->uploadNumber, patientName, captureNumber);

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    QEventLoop eventLoop;
    QNetworkRequest request(QUrl(hostAddress + "api/v1/file/aliyunOssSignature?dir=wukong&expireln=600"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/api/v1/file/aliyunOssSignature?dir=wukong&expireln=600"));
    request.setRawHeader("Authorization",    this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSignatureReceived(QNetworkReply*)));

    manager->get(request);
}

void uploadrequest::onSignatureReceived(QNetworkReply* reply) {

    qDebug() << "onSignatureReceived()";

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
    uploadImageToAliyun2(this, SLOT(onUploadImageToAliyunCompleted2(QNetworkReply*)), jsonResponse);
    uploadImageToAliyun3(this, SLOT(onUploadImageToAliyunCompleted3(QNetworkReply*)), jsonResponse);
}

void uploadrequest::uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument json) {

    qDebug() << "uploadImageToAliyun: " << imageToSend.cols << ", " << imageToSend.rows << ", " << imageToSend.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString(json["host"].toString())));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Origin", "https://qa.mosainet.com/");
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Referer", "https://qa.mosainet.com/");
    request.setRawHeader("Sec-Fetch-Dest", "empty");
    request.setRawHeader("Sec-Fetch-Mode", "cors");
    request.setRawHeader("Sec-Fetch-Site", "cross-site");
    request.setRawHeader("sec-ch-ua-mobile", "?0");
    request.setRawHeader("sec-ch-ua-platform", "\"AI Server\"");
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data;boundary="+ multipart->boundary());

    //加密串
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"policy\""));
    textPart.setBody(json["policy"].toString().toUtf8());

    //身份
    QHttpPart textPart2;
    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"OSSAccessKeyId\""));
    textPart2.setBody(json["accessId"].toString().toUtf8());

    QHttpPart textPart3;
    textPart3.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"success_action_status\""));
    textPart3.setBody(QString("200").toUtf8());

    //签名
    QHttpPart textPart4;
    textPart4.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"signature\""));
    textPart4.setBody(json["signature"].toString().toUtf8());

    //文件Key
    QHttpPart textPart5;
    textPart5.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
    //textPart5.setBody(QString("wukong/testImage1123").toUtf8());
    textPart5.setBody((QString("wukong/") + imageName).toUtf8());

    multipart->append(textPart);
    multipart->append(textPart2);
    multipart->append(textPart3);
    multipart->append(textPart4);
    multipart->append(textPart5);

    //QHttpPart imagePart;
    ////imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    //imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"abc.jpg\""));
    //QFile* file = new QFile("C:/Users/User/Desktop/abc.jpg");
    //file->open(QIODevice::ReadOnly);
    //imagePart.setBodyDevice(file);
    //file->setParent(multipart);
    //multipart->append(imagePart);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\""+ imageName +".png\""));
    //imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"testImage1123.png\""));
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QImage qImage((const uchar*)imageToSend.data, imageToSend.cols, imageToSend.rows, imageToSend.step, QImage::Format_RGBA64);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);

    multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    receivedURL = QString(json["host"].toString()) + QString("/wukong/") + imageName;

    manager->post(request, multipart);
}

void uploadrequest::uploadImageToAliyun2(const QObject* receiver, const char* member, QJsonDocument json) {

    qDebug() << "uploadImageToAliyun2: " << imageToSend2.cols << ", " << imageToSend2.rows << ", " << imageToSend2.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString(json["host"].toString())));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Origin", "https://qa.mosainet.com/");
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Referer", "https://qa.mosainet.com/");
    request.setRawHeader("Sec-Fetch-Dest", "empty");
    request.setRawHeader("Sec-Fetch-Mode", "cors");
    request.setRawHeader("Sec-Fetch-Site", "cross-site");
    request.setRawHeader("sec-ch-ua-mobile", "?0");
    request.setRawHeader("sec-ch-ua-platform", "\"AI Server\"");
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data;boundary=" + multipart->boundary());

    //加密串
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"policy\""));
    textPart.setBody(json["policy"].toString().toUtf8());

    //身份
    QHttpPart textPart2;
    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"OSSAccessKeyId\""));
    textPart2.setBody(json["accessId"].toString().toUtf8());

    QHttpPart textPart3;
    textPart3.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"success_action_status\""));
    textPart3.setBody(QString("200").toUtf8());

    //签名
    QHttpPart textPart4;
    textPart4.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"signature\""));
    textPart4.setBody(json["signature"].toString().toUtf8());

    //文件Key
    QHttpPart textPart5;
    textPart5.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
    //textPart5.setBody(QString("wukong/testImage1123").toUtf8());
    textPart5.setBody((QString("wukong/") + imageName + QString("RGB")).toUtf8());

    multipart->append(textPart);
    multipart->append(textPart2);
    multipart->append(textPart3);
    multipart->append(textPart4);
    multipart->append(textPart5);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + imageName + "RGB.png\""));
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));

    cvtColor(imageToSend2, imageToSend2, cv::COLOR_BGRA2RGB);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QImage qImage((const uchar*)imageToSend2.data, imageToSend2.cols, imageToSend2.rows, imageToSend2.step, QImage::Format_RGB888);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);

    multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    receivedURL2 = QString(json["host"].toString()) + QString("/wukong/") + imageName + QString("RGB");

    manager->post(request, multipart);
}

void uploadrequest::uploadImageToAliyun3(const QObject* receiver, const char* member, QJsonDocument json) {

    qDebug() << "uploadImageToAliyun3: " << imageToSend3.cols << ", " << imageToSend3.rows << ", " << imageToSend3.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString(json["host"].toString())));
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Connection", "keep-alive");
    request.setRawHeader("Origin", "https://qa.mosainet.com/");
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Referer", "https://qa.mosainet.com/");
    request.setRawHeader("Sec-Fetch-Dest", "empty");
    request.setRawHeader("Sec-Fetch-Mode", "cors");
    request.setRawHeader("Sec-Fetch-Site", "cross-site");
    request.setRawHeader("sec-ch-ua-mobile", "?0");
    request.setRawHeader("sec-ch-ua-platform", "\"AI Server\"");
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data;boundary=" + multipart->boundary());

    //加密串
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"policy\""));
    textPart.setBody(json["policy"].toString().toUtf8());

    //身份
    QHttpPart textPart2;
    textPart2.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"OSSAccessKeyId\""));
    textPart2.setBody(json["accessId"].toString().toUtf8());

    QHttpPart textPart3;
    textPart3.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"success_action_status\""));
    textPart3.setBody(QString("200").toUtf8());

    //签名
    QHttpPart textPart4;
    textPart4.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"signature\""));
    textPart4.setBody(json["signature"].toString().toUtf8());

    //文件Key
    QHttpPart textPart5;
    textPart5.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"key\""));
    textPart5.setBody((QString("wukong/") + imageName + QString("PCD")).toUtf8());

    multipart->append(textPart);
    multipart->append(textPart2);
    multipart->append(textPart3);
    multipart->append(textPart4);
    multipart->append(textPart5);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"" + imageName + "PCD.png\""));
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    QImage qImage((const uchar*)imageToSend3.data, imageToSend3.cols, imageToSend3.rows, imageToSend3.step, QImage::Format_RGBA64);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);

    multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    receivedURL3 = QString(json["host"].toString()) + QString("/wukong/") + imageName + QString("PCD");

    manager->post(request, multipart);
}


void uploadrequest::onUploadImageToAliyunCompleted(QNetworkReply* reply) {

    qDebug() << "onUploadImageToAliyunCompleted";

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QByteArray response_data = reply->readAll();

    reply->deleteLater();

    /** ERROR */
    if (statusCode.toInt() != 200) {
        uploadProgressDialog->onFailed(this->uploadNumber);
        qDebug() << statusCode.toInt();
        return;
    }


    stageProgress = 1;
    Completed1 = TRUE;

    if (Completed2 && Completed3) {
        // both completed
        bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
    }
    //bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void uploadrequest::onUploadImageToAliyunCompleted2(QNetworkReply* reply) {

    qDebug() << "onUploadImageToAliyunCompleted2";

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    QByteArray response_data = reply->readAll();
    //qDebug() << response_data;

    reply->deleteLater();
    /** ERROR */
    if (statusCode.toInt() != 200) {
        uploadProgressDialog->onFailed(this->uploadNumber);
        qDebug() << statusCode.toInt();
        return;
    }

    stageProgress = 1;
    Completed2 = TRUE;

    if (Completed1 && Completed3) {
        // both completed
        bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
    }

    //bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void uploadrequest::onUploadImageToAliyunCompleted3(QNetworkReply* reply) {

    qDebug() << "onUploadImageToAliyunCompleted3";

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QByteArray response_data = reply->readAll();

    reply->deleteLater();

    /** ERROR */
    if (statusCode.toInt() != 200) {
        uploadProgressDialog->onFailed(this->uploadNumber);
        qDebug() << statusCode.toInt();
        return;
    }

    stageProgress = 1;
    Completed3 = TRUE;

    if (Completed1 && Completed2) {
        // both completed
        bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
    }
    //bindImageUrl(this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void uploadrequest::uploadImageNormally(const QObject* receiver, const char* member) {
    qDebug() << "uploadImage: " << imageToSend.cols << ", " << imageToSend.rows;

    uploadProgressDialog->show();
    uploadProgressDialog->addRow(this->uploadNumber, patientName, captureNumber);

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://api.conovamed.com/api/v1/upload")));
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

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/images"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/doctor-api/v1/images"));

    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(10000);

    QJsonObject obj;
    obj["patientId"] = patientId;
    obj["url"] = receivedURL;
    obj["originImageUrl"] = receivedURL2;
    obj["pointCloudUrl"] = receivedURL3;
    obj["imageType"] = imageType;
    obj["imageName"] = imageName;

    qDebug() << "patientID: " << patientId << " imageType: " << imageType << " url: " << receivedURL << " url2: " << receivedURL2 << " url3: " << receivedURL3;

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

    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
    QJsonObject obj = jsonResponse.object();
    if (obj.contains("error")) {
        uploadProgressDialog->onFailed(this->uploadNumber);
        qDebug() << "ERROR";
        return;
    }

    stageProgress = 2;

    uploadProgressDialog->onCompleted(this->uploadNumber);
    //uploadProgressDialog = NULL;
    //delete this;
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

void uploadrequest::readHostAddress() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    QString configDir = dir.absolutePath() + "/configs";
    QSettings settings(QString(configDir + "/config.ini"), QSettings::IniFormat);
    hostAddress = settings.value("hostAddress", "hostAddress").toString(); // settings.value() returns QVariant
    qDebug() << "uploadrequest hostAddress: " << hostAddress;
}

void uploadrequest::start()
{
    //uploadImageNormally(this, SLOT(onUploadImageNormally(QNetworkReply*)));

    if (signature == "") {
        getSignature();
    }
}

