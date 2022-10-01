#pragma once

#include <QtNetwork>
#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <QtWidgets/QWidget>
#include <QtNetwork>

class uploadrequest : public QWidget
{

	Q_OBJECT

	public:
		uploadrequest(QString, QString, int, cv::Mat);
		QString userToken;
		QString signature;
		cv::Mat imageToSend;
		int patientId = 0;
	private:
		void debugRequest(QNetworkRequest, QByteArray);
		void getSignature();
		void uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageNormally(const QObject* receiver, const char* member);
	private slots:
		void onSignatureReceived(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted(QNetworkReply* reply);
		void onUploadImageNormally(QNetworkReply* reply);

};

