#ifndef ASTRAENGINE_H
#define ASTRAENGINE_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include <libobsensor/ObSensor.hpp>
#include "libobsensor/hpp/Pipeline.hpp"
#include "libobsensor/hpp/Error.hpp"
#include "types.h"
#include "librealsense2/rs.hpp"

#define VIDEOWRITER_FPS 30
#define COLOR_IMAGE_WIDTH_ASTRA 1920
#define COLOR_IMAGE_HEIGHT_ASTRA 1080 
#define DEPTH_IMAGE_WIDTH_ASTRA 640
#define DEPTH_IMAGE_HEIGHT_ASTRA 480

#define MAX_GYROSCOPE_QUEUE_SIZE 30
#define MAX_ACCELEROMETER_QUEUE_SIZE 30

class AstraEngine : public QWidget
{
    Q_OBJECT

public:
    // COLOR_IMAGE_CROP_WIDTH_PER_SIDE can be set to 0 to disable such crop
    //int COLOR_IMAGE_CROP_WIDTH_PER_SIDE = (COLOR_IMAGE_WIDTH_REALSENSE - COLOR_IMAGE_HEIGHT_REALSENSE) / 2;
    int COLOR_IMAGE_CROP_WIDTH_PER_SIDE = 0;

    static AstraEngine& getInstance() {
        static AstraEngine instance;
        return instance;
    }

    AstraEngine(AstraEngine const&) = delete;

    void operator=(AstraEngine const&) = delete;

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
    void readColorImage(cv::Mat& colorImage, std::shared_ptr<ob::ColorFrame> colorFrame = NULL);
    void readDepthImage(cv::Mat& depthImage, std::shared_ptr<ob::DepthFrame> depthFrame = NULL);
    void readColorToDepthImage(cv::Mat& colorToDepthImage, std::shared_ptr<ob::ColorFrame> colorFrame = NULL);
    void readDepthToColorImage(cv::Mat& depthToColorImage, std::shared_ptr<ob::DepthFrame> depthFrame = NULL);
    void readPointCloudImage(cv::Mat& xyzImage);
    std::deque<point3D> getGyroSampleQueue();
    std::deque<point3D> getAccSampleQueue();

    void computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out);
    QVector3D query3DPoint(int x, int y, cv::Mat depthToColorImage);

    /** New plane calculation */
    float* findPlaneEquationCoefficients(cv::Mat depthToColorImage);
    /** New plane calculation END */

    /** Calculate plane equation and distance between plane and 3D point */
    /** You are given a points (x1, y1, z1) and a plane a * x + b * y + c * z + d = 0.
    The task is to find the perpendicular(shortest) distance between that point and the given Plane. */

    // Find the equation of plane passing through 3 points.
    // Return [a, b, c, d].
    float* findPlaneEquationCoefficients(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3);

    float findDistanceBetween3DPointAndPlane(float x1, float y1, float z1, float a, float b, float c, float d);
    /** Calculate plane equation and distance between plane and 3D point END */

    cv::Mat readCVImageFromFile(std::wstring filename);
    void writeIntrinsicsToFile(rs2_intrinsics& intrin);
    void readIntrinsicsFromFile(std::string path);

private:
    AstraEngine();

    bool deviceOpenedBefore = false;

    std::shared_ptr<ob::FrameSet> frames;
    QReadWriteLock framesLock;

    std::deque<point3D> gyroSampleQueue;
    std::deque<point3D> accSampleQueue;

    //k4a_device_t device = NULL;
    //bool deviceOpenedBefore = false;
    //k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    //k4a_calibration_t calibration;
    //k4a_capture_t capture;
    //QReadWriteLock k4aImageLock;
    //k4a_image_t k4aColorImage;
    //k4a_image_t k4aDepthImage;
    //std::deque<k4a_float3_t> gyroSampleQueue;
    //std::deque<k4a_float3_t> accSampleQueue;

    //rs2_intrinsics intrin;
};

//QImage convertColorCVToQImage(cv::Mat);
//QImage convertDepthCVToQImage(cv::Mat);
//QImage convertDepthCVToColorizedQImage(cv::Mat);
//QImage convertColorToDepthCVToQImage(cv::Mat);
//QImage convertDepthToColorCVToQImage(cv::Mat);
//QImage convertDepthToColorCVToColorizedQImage(cv::Mat);
//QImage convertDepthToColorCVToColorizedQImageDetailed(cv::Mat);
//void colorizeDepth(const cv::Mat& gray, cv::Mat& rgb);

#endif
