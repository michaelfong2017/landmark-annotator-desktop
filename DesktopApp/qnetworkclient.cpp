#include "qnetworkclient.h"
#include "stdafx.h"
#include <twolinesdialog.h>
#include "uploadrequest.h"

QNetworkClient::QNetworkClient() : QWidget() {
    
}

void QNetworkClient::login(QTabWidget* qTabWidget, QString username, QString password, bool isEmailLogin) {
    this->qTabWidget = qTabWidget;

    // Login
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/account/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(10000);

    qDebug() << "Login with username:" << username;

    QJsonObject obj;
    obj["account"] = username;
    obj["password"] = password;
    if (isEmailLogin) {
        obj["platformType"] = 0;
    }
    else {
        obj["platformType"] = 4;
    }
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onLogin(QNetworkReply*)));

    manager->post(request, data);
    // returns userToken, needs to be stored
}
void QNetworkClient::onLogin(QNetworkReply* reply) {

    QByteArray response_data = reply->readAll();
    //qDebug() << response_data;

    /** TIMEOUT */
    if (response_data == nullptr) {
        TwoLinesDialog dialog;
        dialog.setLine1("Login Timeout!");
        dialog.exec();
        return;
    }
    /** TIMEOUT END */

    QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
    //qDebug() << jsonResponse;
    QJsonObject obj = jsonResponse.object();

    if (obj.contains("error")) {
        QJsonObject child = obj["error"].toObject();
        TwoLinesDialog dialog;
        dialog.setLine1("Error: " + child["message"].toString());
        dialog.exec();
    }
    else {
        this->userToken = QString::fromStdString("Bearer " + response_data.toStdString());
        qDebug() << this->userToken;
        qTabWidget->setCurrentIndex(TabIndex::PATIENTLISTTAB);
        qTabWidget->setTabEnabled(TabIndex::LOGINTAB, false);
    }

    reply->deleteLater();

}

void QNetworkClient::fetchPatientList(const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/patients?MaxResultCount=500&Category=Passed"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/doctor-api/v1/patients?MaxResultCount=500&Category=Passed"));
    request.setRawHeader("Authorization", this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::checkNewPatient(Patient patient, const QObject* receiver, const char* member) {
    // Check New Patient Exist?

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/patients/check"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/doctor-api/v1/patients/check"));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    /** Parse patient */
    int sex;
    switch (patient.getSex()) {
    case Sex::Male:
        sex = 1;
        break;
    case Sex::Female:
        sex = 2;
        break;
    default:
        sex = 0;
        break;
    }

    /** Parse patient END */

    QJsonObject obj;
    obj["name"] = QString::fromStdString(patient.getName());
    obj["sex"] = sex;
    obj["birthday"] = patient.getDOB().toString("yyyy-MM-dd");
    obj["phoneNumber"] = QString::fromStdString(patient.getPhoneNumber());
    obj["idCard"] = QString::fromStdString(patient.getHKID());
    obj["sin"] = QString::fromStdString(patient.getSocialSecurityNumber());
    obj["subjectNumber"] = QString::fromStdString(patient.getSubjectNumber());
    obj["email"] = QString::fromStdString(patient.getEmail());
    obj["address"] = QString::fromStdString(patient.getAddress());
    /**
    * Not sure whether height and weight can be uploaded. Currently not uploaded.
    */
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);

    //returns [] if no problem, returns information of patient if already exist
}

void QNetworkClient::uploadNewPatient(Patient patient, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/patients"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/doctor-api/v1/patients"));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    /** Parse patient */
    int sex;
    switch (patient.getSex()) {
    case Sex::Male:
        sex = 1;
        break;
    case Sex::Female:
        sex = 2;
        break;
    default:
        sex = 0;
        break;
    }

    /** Parse patient END */

    QJsonObject obj;
    obj["name"] = QString::fromStdString(patient.getName());
    obj["sex"] = sex;
    obj["birthday"] = patient.getDOB().toString("yyyy-MM-dd");
    obj["phoneNumber"] = QString::fromStdString(patient.getPhoneNumber());
    obj["idCard"] = QString::fromStdString(patient.getHKID());
    obj["sin"] = QString::fromStdString(patient.getSocialSecurityNumber());
    obj["subjectNumber"] = QString::fromStdString(patient.getSubjectNumber());
    obj["email"] = QString::fromStdString(patient.getEmail());
    obj["address"] = QString::fromStdString(patient.getAddress());
    obj["country"] = QString::fromStdString(patient.getNationality());
    obj["remarks"] = QString::fromStdString(patient.getRemarks());
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);

    // will return patientId, needs to be stored
}

void QNetworkClient::fetchExistingImagesOfPatient(int patientId, const QObject* receiver, const char* member) {
    // Get Image List
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    qDebug() << "fetchExistingImagesOfPatient";

    qDebug() << hostAddress + QString("doctor-api/v1/patients/%1/images?ImageTypes=7&MaxResultCount=500").arg(patientId);

    QNetworkRequest request(QUrl(hostAddress + QString("doctor-api/v1/patients/%1/images?ImageTypes=7&MaxResultCount=500").arg(patientId)));
    //QNetworkRequest request(QUrl(QString("https://api.conovamed.com/doctor-api/v1/patients/%1/images?ImageTypes=7&MaxResultCount=500").arg(patientId)));
    request.setRawHeader("Authorization", userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::uploadImage(cv::Mat image, const QObject* receiver, const char* member) {

    qDebug() << "uploadImage: " << image.channels();

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString(hostAddress + "api/v1/upload")));
    //QNetworkRequest request(QUrl(QString("https://api.conovamed.com/api/v1/upload")));
    request.setRawHeader("Authorization", this->userToken.toUtf8());
    request.setTransferTimeout(25000);

    QHttpMultiPart* multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imageArray;
    imageArray.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/png"));
    imageArray.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"test.png\""));
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    QImage qImage((const uchar*)image.data, image.cols, image.rows, image.step, QImage::Format_RGBA64);
    qImage.bits();
    qImage.save(&buffer, "PNG");
    imageArray.setBody(byteArray);
    multipart->append(imageArray);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, multipart);
}

void QNetworkClient::bindImageUrl(int patientId, QString url, int imageType, const QObject* receiver, const char* member) {

    qDebug() << "bindImageUrl";

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + "doctor-api/v1/images"));
    //QNetworkRequest request(QUrl("https://api.conovamed.com/doctor-api/v1/images"));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(10000);

    QJsonObject obj;
    obj["patientId"] = patientId;
    obj["url"] = url;
    obj["imageType"] = imageType;
    obj["imageName"] = "test.png";

    qDebug() << "patientID: " << patientId << " imageType: " << imageType;

    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);
}

void QNetworkClient::findLandmarkPredictions(int imageId, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(hostAddress + QString("doctor-api/v1/images/%1").arg(imageId)));
    //QNetworkRequest request(QUrl(QString("https://api.conovamed.com/doctor-api/v1/images/%1").arg(imageId)));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setTransferTimeout(10000);

    //qDebug() << QUrl(QString("https://api.conovamed.com/doctor-api/v1/images/%1").arg(imageId));

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::downloadImage(QString imageUrl, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(imageUrl);
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setTransferTimeout(10000);

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::confirmLandmarks(int imageId, QString aiOriginResult, const QObject* receiver, const char* member) {

    qDebug() << "confirmLandmarks imageId:" << imageId;

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://api.conovamed.com/doctor-api/v1/images/%1/audit").arg(imageId)));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["result"] = aiOriginResult;
    obj["conditionStatus"] = 1;
    obj["describe"] = "null";
    obj["addResult"] = "null";
    obj["scoliosisType"] = 0;

    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->put(request, data);
}

void QNetworkClient::readHostAddress() {
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    QString configDir = dir.absolutePath() + "/configs";
    QSettings settings(QString(configDir + "/config.ini"), QSettings::IniFormat);
    hostAddress = settings.value("hostAddress", "hostAddress").toString(); // settings.value() returns QVariant
    qDebug() << "QNetworkClient hostAddress: " << hostAddress;
}
