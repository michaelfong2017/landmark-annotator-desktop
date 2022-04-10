#ifndef KINECTENGINE_H
#define KINECTENGINE_H

#include <QtWidgets/QWidget>
#include <k4a/k4a.hpp>
#include "stdafx.h"

class KinectEngine : public QWidget
{
    Q_OBJECT

public:
    static KinectEngine& getInstance() {
        static KinectEngine instance;
        return instance;
    }

    KinectEngine(KinectEngine const&) = delete;

    void operator=(KinectEngine const&) = delete;

    void clear();

    bool openDevice();
    void closeDevice();
    void configDevice();

    void captureImages();
    void readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage);
    void readColorImage(cv::Mat& colorImage, k4a_image_t k4aColorImage);
    void readDepthImage(cv::Mat& depthImage, k4a_image_t k4aDepthImage);
    void readColorToDepthImage(cv::Mat& colorToDepthImage, k4a_image_t k4aColorImage, k4a_image_t k4aDepthImage);
    void readDepthToColorImage(cv::Mat& depthToColorImage, k4a_image_t k4aColorImage, k4a_image_t k4aDepthImage);

private:
    KinectEngine();

    k4a_device_t device = NULL;
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    k4a_calibration_t calibration;
    k4a_capture_t capture;
    QReadWriteLock k4aImageLock;
    k4a_image_t k4aColorImage;
    k4a_image_t k4aDepthImage;
};

#endif
