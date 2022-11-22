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
		uploadrequest(QString userToken, QString signature, int patientId, int imageType, QString imageName, cv::Mat imageToSend, cv::Mat imageToSend2, cv::Mat imageToSend3, int uploadNumber, int captureNumber, QString patientName, UploadProgressDialog* uploadProgressDialog);
		void retry(int);
		void readHostAddress();
		void start();
	private:
		//QString hostAddress = "https://api.conovamed.com/";
		//QString hostAddress = "https://qa.mosainet.com/sm-api/";
		QString hostAddress = "";
		QString userToken;
		QString signature;
		int patientId = -1;
		QString patientName;
		int imageType = 7;
		cv::Mat imageToSend;
		cv::Mat imageToSend2;
		cv::Mat imageToSend3;
		QString imageName;
		int captureNumber = -1;
		QString receivedURL;
		QString receivedURL2;
		QString receivedURL3;
		bool Completed1 = false;
		bool Completed2 = false;
		bool Completed3 = false;
		UploadProgressDialog* uploadProgressDialog;
		int uploadNumber;
		int stageProgress = 0;
		void getSignature();
		void uploadImageToAliyun(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageToAliyun2(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageToAliyun3(const QObject* receiver, const char* member, QJsonDocument);
		void uploadImageNormally(const QObject* receiver, const char* member);
		void reuploadNormally(int, const QObject* receiver, const char* member);
		void bindImageUrl(const QObject* receiver, const char* member);
		void debugRequest(QNetworkRequest, QByteArray);
private slots:
		void onSignatureReceived(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted2(QNetworkReply* reply);
		void onUploadImageToAliyunCompleted3(QNetworkReply* reply);
		void onUploadImageNormally(QNetworkReply* reply);
		void onBindImageUrl(QNetworkReply* reply);

};

