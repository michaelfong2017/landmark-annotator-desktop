#pragma once

#include <QtNetwork>
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <QtWidgets/QWidget>
#include <twolinesdialog.h>
#include "uploadprogressdialog.h"

class uploadrequest : public QWidget
{

	Q_OBJECT

	public:
		uploadrequest(QString userToken, QString signature, int patientId, int imageType, QString imageName, cv::Mat imageToSend, int captureNumber, QString patientName, UploadProgressDialog* uploadProgressDialog);
	private:
		QString userToken;
		QString signature;
		int patientId = -1;
		QString patientName;
		int imageType = 7;
		cv::Mat imageToSend;
		QString imageName;
		int captureNumber = -1;
		QString receivedURL;
		UploadProgressDialog* uploadProgressDialog;
		int uploadNumber;
		int stageProgress = 0;
		void getSignature();
		void uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageNormally(const QObject* receiver, const char* member);
		void bindImageUrl(const QObject* receiver, const char* member);
		void debugRequest(QNetworkRequest, QByteArray);
private slots:
		void onSignatureReceived(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted(QNetworkReply* reply);
		void onUploadImageNormally(QNetworkReply* reply);
		void onBindImageUrl(QNetworkReply* reply);

};

