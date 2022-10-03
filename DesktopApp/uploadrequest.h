#pragma once

#include <QtNetwork>
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <QtWidgets/QWidget>
#include <QtNetwork>
#include <twolinesdialog.h>
#include "uploadprogressdialog.h"

class uploadrequest : public QWidget
{

	Q_OBJECT

	public:
		uploadrequest(QString userToken, QString signature, int patientId, int captureNumber, cv::Mat imageToSend, UploadProgressDialog* uploadProgressDialog);
	private:
		QString userToken;
		QString signature;
		int patientId = -1;
		int captureNumber = -1;
		cv::Mat imageToSend;
		UploadProgressDialog* uploadProgressDialog;
		int uploadNumber;

		void debugRequest(QNetworkRequest, QByteArray);
		void getSignature();
		void uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageNormally(const QObject* receiver, const char* member);
	private slots:
		void onSignatureReceived(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted(QNetworkReply* reply);
		void onUploadImageNormally(QNetworkReply* reply);

};

