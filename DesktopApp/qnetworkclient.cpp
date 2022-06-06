#include "qnetworkclient.h"
#include "stdafx.h"

QNetworkClient::QNetworkClient() : QWidget() {
    
}

void QNetworkClient::login(QTabWidget* qTabWidget) {
    this->qTabWidget = qTabWidget;

    // Login
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

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
    this->userToken = QString::fromStdString("Bearer " + reply->readAll().toStdString());
    qDebug() << this->userToken;
    reply->deleteLater();

    qTabWidget->setCurrentIndex(1);
}

void QNetworkClient::fetchPatientList(const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/patients?MaxResultCount=100"));
    request.setRawHeader("Authorization", this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::checkNewPatient(Patient patient, const QObject* receiver, const char* member) {
    // Check New Patient Exist?

    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/patients/check"));
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
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);

    //returns [] if no problem, returns information of patient if already exist
}

void QNetworkClient::uploadNewPatient(Patient patient, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/patients"));
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
    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);

    // will return patientId, needs to be stored
}

void QNetworkClient::fetchExistingImagesOfPatient(int patientId, const QObject* receiver, const char* member) {
    // Get Image List
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    qDebug() << QString("https://qa.mosainet.com/sm-api/doctor-api/v1/patients/%1/images?ImageTypes=7").arg(patientId);

    QNetworkRequest request(QUrl(QString("https://qa.mosainet.com/sm-api/doctor-api/v1/patients/%1/images?ImageTypes=7").arg(patientId)));
    request.setRawHeader("Authorization", userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}

void QNetworkClient::uploadImage(cv::Mat image, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://qa.mosainet.com/sm-api/api/v1/upload")));
    request.setRawHeader("Authorization", this->userToken.toUtf8());

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

void QNetworkClient::bindImageUrl(int patientId, QString url, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/images"));
    request.setRawHeader("Authorization", userToken.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject obj;
    obj["patientId"] = patientId;
    obj["url"] = url;
    obj["imageType"] = 7;
    obj["imageName"] = "test.png";

    QByteArray data = QJsonDocument(obj).toJson();

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->post(request, data);
}

void QNetworkClient::findLandmarkPredictions(int imageId, const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl(QString("https://qa.mosainet.com/sm-api/doctor-api/v1/images/%1").arg(imageId)));
    request.setRawHeader("Authorization", userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
}
