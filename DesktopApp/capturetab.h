#ifndef CAPTURETAB_H
#define CAPTURETAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "helper.h"
#include "annotatetab.h"
#include "patienttab.h"
#include "desktopapp.h"
#include "recorder.h"
#include <k4a/k4a.hpp>
#include <opencv2/opencv.hpp>
#include "util/networkutil.h"
#include "qnetworkclient.h"
#include "twolinesdialog.h"

class CaptureTab: public QWidget
{
    Q_OBJECT

public:
    CaptureTab(DesktopApp* parent);
    DesktopApp* getParent();
    QTimer* timer;
    int getCaptureCount();
    void setCaptureCount(int newCaptureCount);
    Recorder* getRecorder();
    QString getCaptureFilepath();
    void setCaptureFilepath(QString captureFilepath);

    bool isUploading;
    cv::Mat getCapturedColorImage();
    cv::Mat getCapturedDepthImage();
    cv::Mat getCapturedColorToDepthImage();
    cv::Mat getCapturedDepthToColorImage();

    cv::Mat computeNormalizedDepthImage(cv::Mat);

    QImage getQColorImage();
    QImage getQDepthImage();
    QImage getQColorToDepthImage();
    QImage getQDepthToColorImage();
    QImage getQDepthToColorColorizedImage();

private:
    DesktopApp* parent;

    // Only captured images have to be stored
    // Live preview images are not stored
    cv::Mat capturedColorImage;
    cv::Mat capturedDepthImage;
    cv::Mat capturedColorToDepthImage;
    cv::Mat capturedDepthToColorImage;
    // stored captured images END

    QImage qColorImage;
    QImage qDepthImage;
    QImage qColorToDepthImage;
    QImage qDepthToColorImage;
    QImage qDepthToColorColorizedImage;

    cv::Mat RANSACImage;

    int captureCount;
    Recorder* recorder;
    QString captureFilepath;
    QElapsedTimer recordingElapsedTimer;
    QNetworkAccessManager manager;
    bool noImageCaptured;
    void setDefaultCaptureMode();
    void registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image);
    void alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ);
    void onManagerFinished(QNetworkReply* reply);

private slots:
    void onUploadImage(QNetworkReply* reply);
    void onBindImageUrl(QNetworkReply* reply);
    void onFindLandmarkPredictions(QNetworkReply* reply);
};

#endif
