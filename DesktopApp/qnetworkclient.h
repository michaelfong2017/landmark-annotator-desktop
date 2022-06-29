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

    void login(QTabWidget* qTabWidget, QString username , QString password);
    void fetchPatientList(const QObject* receiver, const char* member);
    void checkNewPatient(Patient patient, const QObject* receiver, const char* member);
    void uploadNewPatient(Patient patient, const QObject* receiver, const char* member);
    void fetchExistingImagesOfPatient(int patientId, const QObject* receiver, const char* member);
    void uploadImage(cv::Mat image, const QObject* receiver, const char* member);
    void bindImageUrl(int patientId, QString url, const QObject* receiver, const char* member);
    void findLandmarkPredictions(int imageId, const QObject* receiver, const char* member);
    void downloadImage(QString imageUrl, const QObject* receiver, const char* member);
    void confirmLandmarks(int imageId, QString aiOriginResult, const QObject* receiver, const char* member);

private:
    QNetworkClient();
    QString userToken;
    QTabWidget* qTabWidget;

private slots:
    void onLogin(QNetworkReply* reply);
};

#endif
