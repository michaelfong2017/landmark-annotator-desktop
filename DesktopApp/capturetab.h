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

class CaptureTab: public QWidget
{
    Q_OBJECT

public:
    CaptureTab(DesktopApp* parent);
    DesktopApp* getParent();
    QTimer* timer;
    QImage getQCapturedColorImage();
    QImage getQCapturedDepthToColorImage();
    k4a_image_t* getK4aPointCloud();
    k4a_image_t* getK4aDepthToColor();
    QVector3D query3DPoint(int x, int y);
    QImage getColorImage();
    QImage getDepthImage();
    QImage getColorToDepthImage();
    QImage getDepthToColorImage();
    int getCaptureCount();
    void setCaptureCount(int newCaptureCount);
    Recorder* getRecorder();
    QString getCaptureFilepath();
    void setCaptureFilepath(QString captureFilepath);

private:
    DesktopApp* parent;
    k4a_image_t k4aPointCloud;
    k4a_image_t k4aDepthToColor;
    QImage colorImage;
    QImage depthImage;
    QImage colorToDepthImage;
    QImage depthToColorImage;
    int captureCount;
    Recorder* recorder;
    QString captureFilepath;
    QElapsedTimer recordingElapsedTimer;
    void setDefaultCaptureMode();
    void registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image);
    void drawGyroscopeData();
    void drawAccelerometerData();
};

#endif
