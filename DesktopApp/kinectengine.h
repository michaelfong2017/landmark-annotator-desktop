#ifndef KINECTENGINE_H
#define KINECTENGINE_H

#include <QtWidgets/QWidget>
#include <k4a/k4a.hpp>
#include "stdafx.h"
#include <utility>

#define VIDEOWRITER_FPS 30
#define COLOR_IMAGE_WIDTH 1920
#define COLOR_IMAGE_HEIGHT 1080 
#define DEPTH_IMAGE_WIDTH 640
#define DEPTH_IMAGE_HEIGHT 576

#define MAX_GYROSCOPE_QUEUE_SIZE 30
#define MAX_ACCELEROMETER_QUEUE_SIZE 30

class KinectEngine : public QWidget
{
    Q_OBJECT

public:
    // COLOR_IMAGE_CROP_WIDTH_PER_SIDE can be set to 0 to disable such crop
    int COLOR_IMAGE_CROP_WIDTH_PER_SIDE = (COLOR_IMAGE_WIDTH - COLOR_IMAGE_HEIGHT) / 2;

    static KinectEngine& getInstance() {
        static KinectEngine instance;
        return instance;
    }

    KinectEngine(KinectEngine const&) = delete;

    void operator=(KinectEngine const&) = delete;

    void clear();

    bool isDeviceConnected();
    bool isDeviceOpened();
    bool openDevice();
    void closeDevice();
    void configDevice();

    void captureImages();
    bool queueIMUSample();
    void readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage);
    void readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage);
    void readColorImage(cv::Mat& colorImage, k4a_image_t k4aColorImage = NULL);
    void readDepthImage(cv::Mat& depthImage, k4a_image_t k4aDepthImage = NULL);
    void readColorToDepthImage(cv::Mat& colorToDepthImage, k4a_image_t k4aColorImage = NULL, k4a_image_t k4aDepthImage = NULL);
    void readDepthToColorImage(cv::Mat& depthToColorImage, k4a_image_t k4aColorImage = NULL, k4a_image_t k4aDepthImage = NULL);
    std::deque<k4a_float3_t> getGyroSampleQueue();
    std::deque<k4a_float3_t> getAccSampleQueue();
    QVector3D query3DPoint(int x, int y, cv::Mat depthToColorImage);

    /** New plane calculation */
    float* findPlaneEquationCoefficients(cv::Mat depthToColorImage);
    /** New plane calculation END */

    /** Calculate plane equation and distance between plane and 3D point */
    /** You are given a points (x1, y1, z1) and a plane a * x + b * y + c * z + d = 0.
    The task is to find the perpendicular(shortest) distance between that point and the given Plane. */
    
    // Find the equation of plane passing through 3 points.
    // Return [a, b, c, d].
    float* findPlaneEquationCoefficients(float x1, float y1,
        float z1, float x2,
        float y2, float z2,
		float x3, float y3, float z3);

    float findDistanceBetween3DPointAndPlane(float x1, float y1,
        float z1, float a,
        float b, float c,
        float d);

    /** Calculate plane equation and distance between plane and 3D point END */

private:
    KinectEngine();

    k4a_device_t device = NULL;
    bool deviceOpenedBefore = false;
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    k4a_calibration_t calibration;
    k4a_capture_t capture;
    QReadWriteLock k4aImageLock;
    k4a_image_t k4aColorImage;
    k4a_image_t k4aDepthImage;
    std::deque<k4a_float3_t> gyroSampleQueue;
    std::deque<k4a_float3_t> accSampleQueue;
};

QImage convertColorCVToQImage(cv::Mat);
QImage convertDepthCVToQImage(cv::Mat);
QImage convertDepthCVToColorizedQImage(cv::Mat);
QImage convertColorToDepthCVToQImage(cv::Mat);
QImage convertDepthToColorCVToQImage(cv::Mat);
QImage convertDepthToColorCVToColorizedQImage(cv::Mat);
void colorizeDepth(const cv::Mat& gray, cv::Mat& rgb);

#endif
