#ifndef CAPTURETAB_H
#define CAPTURETAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "helper.h"
#include "annotatetab.h"
#include "desktopapp.h"
#include "recorder.h"
#include <k4a/k4a.hpp>
#include <opencv2/opencv.hpp>
#include "util/networkutil.h"
#include "qnetworkclient.h"

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

    cv::Mat getCapturedColorImage();
    cv::Mat getCapturedDepthImage();
    cv::Mat getCapturedColorToDepthImage();
    cv::Mat getCapturedDepthToColorImage();

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

    int captureCount;
    Recorder* recorder;
    QString captureFilepath;
    QElapsedTimer recordingElapsedTimer;
    QNetworkAccessManager manager;
    bool noImageCaptured;
    void setDefaultCaptureMode();
    void registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image);
    void drawGyroscopeData(std::deque<k4a_float3_t> gyroSampleQueue);
    void drawAccelerometerData(std::deque<k4a_float3_t> accSampleQueue);
    void alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ);
    void onManagerFinished(QNetworkReply* reply);

private slots:
    void onUploadImage(QNetworkReply* reply);
};

#endif
