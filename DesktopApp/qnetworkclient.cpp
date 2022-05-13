#include "qnetworkclient.h"
#include "stdafx.h"

QNetworkClient::QNetworkClient() : QWidget() {
    
}

void QNetworkClient::login() {
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
    reply->deleteLater();
}

void QNetworkClient::fetchPatientList(const QObject* receiver, const char* member) {
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);

    QNetworkRequest request(QUrl("https://qa.mosainet.com/sm-api/doctor-api/v1/patients"));
    request.setRawHeader("Authorization", this->userToken.toUtf8());

    connect(manager, SIGNAL(finished(QNetworkReply*)), receiver, member);

    manager->get(request);
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
        sex = 0;
        break;
    default:
        sex = -1;
        break;
    }

    /** Parse patient END */

    //QJsonObject obj;
    //obj["name"] = QString::fromStdString(patient.getName());
    //obj["sex"] = sex;
    //obj["birthday"] = patient.getDOB().toString("yyyy-MM-dd");
    //obj["phoneNumber"] = QString::fromStdString(patient.getPhoneNumber());
    //obj["idCard"] = QString::fromStdString(patient.getHKID());
    //obj["sin"] = "1232135";		// sin is social security number
    //obj["subjectNumber"] = "Subject A123";
    //obj["email"] = "1243test@gmail.com";
    //obj["address"] = "Mong Kok";
    //QByteArray data = QJsonDocument(obj).toJson();

    //out << "Full name: " << QString::fromStdString(this->parent->patient.getName()) << "\n";
    //out << "Study number: " << QString::fromStdString(this->parent->patient.getStudyNumber()) << "\n";
    //out << "Medical number: " << QString::fromStdString(this->parent->patient.getMedicalNumber()) << "\n";
    //out << "Phone number: " << QString::fromStdString(this->parent->patient.getPhoneNumber()) << "\n";
    //out << "HKID: " << QString::fromStdString(this->parent->patient.getHKID()) << "\n";
    //out << "Email: " << QString::fromStdString(this->parent->patient.getEmail()) << "\n";
    //out << "Date of birth: " << this->parent->patient.getDOB().toString(DATE_FORMAT) << "\n";
    //out << "Nationality: " << QString::fromStdString(this->parent->patient.getNationality()) << "\n";
    //out << "Address: " << QString::fromStdString(this->parent->patient.getAddress()) << "\n";


    //connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    //manager->post(request, data);

    // will return patientId, needs to be stored
}
