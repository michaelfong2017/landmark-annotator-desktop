#include "kinectengine.h"
#include <QtWidgets/QWidget>
#include <k4a/k4a.hpp>
#include "stdafx.h"

KinectEngine::KinectEngine() : QWidget() {
    this->configDevice();
    bool openDeviceSuccess = openDevice();
}

#pragma region cvMatRawDataMethods
void KinectEngine::clear()
{
    this->device = NULL;
    this->config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
}
bool KinectEngine::openDevice()
{
    // Shallow copy
    k4a_device_t& device = this->device;
    k4a_device_configuration_t& config = this->config;
    k4a_calibration_t& calibration = this->calibration;

    uint32_t device_count = k4a_device_get_installed_count();

    if (device_count == 0)
    {
        qCritical() << "No K4A devices found.\n";
        return false;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        qCritical() << "Failed to open device.\n";
        k4a_device_close(device);
        return false;
    }

    // Retrive calibration
    if (K4A_RESULT_SUCCEEDED !=
        k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &calibration))
    {
        qCritical() << "Failed to get calibration.\n";
        k4a_device_close(device);
        return false;
    }

    // Start cameras
    if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
    {
        qCritical() << "Failed to start device.\n";
        k4a_device_close(device);
        return false;
    }

    return true;
}
void KinectEngine::closeDevice()
{
    k4a_device_close(this->device);
    this->device = NULL;
}
void KinectEngine::configDevice()
{
    // Shallow copy
    k4a_device_configuration_t& config = this->config;
    config.camera_fps = K4A_FRAMES_PER_SECOND_30;
    config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_720P;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    config.depth_delay_off_color_usec = 0;
}

void KinectEngine::captureImages()
{
    // Shallow copy
    k4a_capture_t& capture = this->capture;

    this->k4aImageLock.lockForWrite();

    if (k4a_device_get_capture(this->device, &capture, K4A_WAIT_INFINITE) != K4A_RESULT_SUCCEEDED) {
    }
    else {
        k4a_image_release(this->k4aColorImage);
        k4a_image_release(this->k4aDepthImage);
        this->k4aColorImage = k4a_capture_get_color_image(this->capture);
        this->k4aDepthImage = k4a_capture_get_depth_image(this->capture);
        k4a_capture_release(capture);
    }

    this->k4aImageLock.unlock();
}

/*
* It is ensured that all read images are from the same capture because once one of the image reads
* starts, the QReadWriteLock blocks the writer from updating any of the images.
*/
void KinectEngine::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage) {
    this->k4aImageLock.lockForRead();
    // Shallow copy
    k4a_image_t k4aColorImage = this->k4aColorImage;
    k4a_image_t k4aDepthImage = this->k4aDepthImage;
    this->k4aImageLock.unlock();

    readColorImage(colorImage, k4aColorImage);
    readDepthImage(depthImage, k4aDepthImage);
    readColorToDepthImage(colorToDepthImage, k4aColorImage, k4aDepthImage);
    readDepthToColorImage(depthToColorImage, k4aColorImage, k4aDepthImage);
}

/*
* Retrieves image from colorImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(1080, 1920, 8UC4), empty cv::Mat if error
*/
void KinectEngine::readColorImage(cv::Mat& colorImage, k4a_image_t k4aColorImage = NULL) {
    // Shallow copy
    k4a_image_t _k4aColorImage = this->k4aColorImage;

    if (k4aColorImage != NULL) {
        _k4aColorImage = k4aColorImage;
    }
    else {
        if (_k4aColorImage == NULL) {
            colorImage = cv::Mat{};
            return;
        }
    }

    uint8_t* buffer = k4a_image_get_buffer(_k4aColorImage);
    int rows = k4a_image_get_height_pixels(_k4aColorImage);
    int cols = k4a_image_get_width_pixels(_k4aColorImage);

    colorImage = cv::Mat(k4a_image_get_height_pixels(_k4aColorImage), k4a_image_get_width_pixels(_k4aColorImage), CV_8UC4, k4a_image_get_buffer(_k4aColorImage));

    return;
}

/*
* Retrieves image from depthImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(576, 640, 16UC1), empty cv::Mat if error
*/
void KinectEngine::readDepthImage(cv::Mat& depthImage, k4a_image_t k4aDepthImage = NULL) {
    // Shallow copy
    k4a_image_t _k4aDepthImage = this->k4aDepthImage;

    if (k4aDepthImage != NULL) {
        _k4aDepthImage = k4aDepthImage;
    }
    else {
        if (_k4aDepthImage == NULL) {
            depthImage = cv::Mat{};
            return;
        }
    }

    int rows = k4a_image_get_height_pixels(_k4aDepthImage);
    int cols = k4a_image_get_width_pixels(_k4aDepthImage);
    uint8_t* buffer = k4a_image_get_buffer(_k4aDepthImage);

    depthImage = cv::Mat(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);
    return;
}

/*
* Takes a new capture and aligned the Color onto Depth
* @return cv::Mat(576, 640, 8UC4), empty cv::Mat if error
*/
void KinectEngine::readColorToDepthImage(cv::Mat& colorToDepthImage, k4a_image_t k4aColorImage = NULL, k4a_image_t k4aDepthImage = NULL) {
    this->k4aImageLock.lockForRead();
    // Shallow copy
    k4a_image_t _k4aColorImage = this->k4aColorImage;
    k4a_image_t _k4aDepthImage = this->k4aDepthImage;
    this->k4aImageLock.unlock();

    if ((k4aColorImage != NULL && k4aDepthImage == NULL) || (k4aColorImage == NULL && k4aDepthImage != NULL)) {
        qWarning() << "Returning empty image. Please either set both or unset both of k4aColorImage and k4aDepthImage.\n";
        colorToDepthImage = cv::Mat{};
        return;
    }
    else if (k4aColorImage != NULL && k4aDepthImage != NULL) {
        _k4aColorImage = k4aColorImage;
        _k4aDepthImage = k4aDepthImage;
    }

    k4a_transformation_t transformationHandle = k4a_transformation_create(&this->calibration);

    k4a_image_t alignmentImage;

    if (k4a_image_create(K4A_IMAGE_FORMAT_COLOR_BGRA32,
        k4a_image_get_width_pixels(_k4aDepthImage),
        k4a_image_get_height_pixels(_k4aDepthImage),
        k4a_image_get_width_pixels(_k4aDepthImage) * 4 * (int)sizeof(uint8_t),
        &alignmentImage) != K4A_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        colorToDepthImage = cv::Mat{};
        return;
    }

    if (k4a_transformation_color_image_to_depth_camera(transformationHandle, _k4aDepthImage, _k4aColorImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        colorToDepthImage = cv::Mat{};
        return;
    }

    // .clone() is necessary
    colorToDepthImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_8UC4, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP).clone();

    k4a_transformation_destroy(transformationHandle);
    k4a_image_release(alignmentImage);

    return;
}

/*
* Takes a new capture and aligned the Depth onto Color
* @return cv::Mat(1080, 1920, 16UC1), empty cv::Mat if error
*/
void KinectEngine::readDepthToColorImage(cv::Mat& depthToColorImage, k4a_image_t k4aColorImage = NULL, k4a_image_t k4aDepthImage = NULL) {
    this->k4aImageLock.lockForRead();
    // Shallow copy
    k4a_image_t _k4aColorImage = this->k4aColorImage;
    k4a_image_t _k4aDepthImage = this->k4aDepthImage;
    this->k4aImageLock.unlock();

    if ((k4aColorImage != NULL && k4aDepthImage == NULL) || (k4aColorImage == NULL && k4aDepthImage != NULL)) {
        qWarning() << "Returning empty image. Please either set both or unset both of k4aColorImage and k4aDepthImage.\n";
        depthToColorImage = cv::Mat{};
        return;
    }
    else if (k4aColorImage != NULL && k4aDepthImage != NULL) {
        _k4aColorImage = k4aColorImage;
        _k4aDepthImage = k4aDepthImage;
    }

    k4a_transformation_t transformationHandle = k4a_transformation_create(&this->calibration);

    k4a_image_t alignmentImage;
    if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, k4a_image_get_width_pixels(_k4aColorImage), k4a_image_get_height_pixels(_k4aColorImage), k4a_image_get_width_pixels(_k4aColorImage) * (int)sizeof(uint16_t),
        &alignmentImage) != K4A_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        depthToColorImage = cv::Mat{};
        return;
    }

    if (k4a_transformation_depth_image_to_color_camera(transformationHandle, _k4aDepthImage, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
        k4a_transformation_destroy(transformationHandle);
        k4a_image_release(alignmentImage);
        depthToColorImage = cv::Mat{};
        return;
    }

    // .clone() is necessary
    depthToColorImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16U, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP).clone();

    k4a_transformation_destroy(transformationHandle);
    k4a_image_release(alignmentImage);

    return;
}

#pragma endregion

