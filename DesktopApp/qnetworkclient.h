#ifndef QNETWORKCLIENT_H
#define QNETWORKCLIENT_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include <QtNetwork>
#include <QtWidgets/QWidget>
#include "patient.h"
#include <opencv2/opencv.hpp>


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

    void login();
    void fetchPatientList(const QObject* receiver, const char* member);
    void checkNewPatient(Patient patient, const QObject* receiver, const char* member);
    void uploadNewPatient(Patient patient, const QObject* receiver, const char* member);
    void fetchExistingImagesOfPatient(int patientId, const QObject* receiver, const char* member);
    void uploadImage(cv::Mat image, const QObject* receiver, const char* member);

private:
    QNetworkClient();
    QString userToken;

private slots:
    void onLogin(QNetworkReply* reply);
};

#endif
