#include "kinectengine.h"
#include <QtWidgets/QWidget>
#include <k4a/k4a.hpp>
#include "stdafx.h"
#include <fstream>

KinectEngine::KinectEngine() : QWidget() {
}

#pragma region cvMatRawDataMethods
void KinectEngine::clear()
{
	this->device = NULL;
	this->config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
}
bool KinectEngine::isDeviceConnected() {
	uint32_t device_count = k4a_device_get_installed_count();
	return device_count != 0;
}
bool KinectEngine::isDeviceOpened() {
	if (isDeviceConnected()) {
		return this->deviceOpenedBefore;
	}
	else {
		this->deviceOpenedBefore = false;
		return false;
	}
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

	// Running once is enough
	//writeCalibrationToFile(calibration);

	// Debug calibration
	k4a_calibration_intrinsic_parameters_t intrin = calibration.depth_camera_calibration.intrinsics.parameters;

	// Start cameras
	if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
	{
		qCritical() << "Failed to start cameras.\n";
		k4a_device_close(device);
		return false;
	}

	// Start IMU
	if (K4A_RESULT_SUCCEEDED != k4a_device_start_imu(device))
	{
		qCritical() << "Failed to start imu.\n";
		k4a_device_close(device);
		return false;
	}

	this->deviceOpenedBefore = true;
	return true;
}
void KinectEngine::closeDevice()
{
	k4a_device_close(this->device);
	this->deviceOpenedBefore = false;
	this->device = NULL;
}
void KinectEngine::configDevice()
{
	// Shallow copy
	k4a_device_configuration_t& config = this->config;
	config.camera_fps = K4A_FRAMES_PER_SECOND_30;
	config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	switch (COLOR_IMAGE_WIDTH) {
	case 1920:
		config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
		break;
	case 1280:
		config.color_resolution = K4A_COLOR_RESOLUTION_720P;
		break;
	}
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

bool KinectEngine::queueIMUSample()
{
	k4a_imu_sample_t imuSample;

	switch (k4a_device_get_imu_sample(this->device, &imuSample, K4A_WAIT_INFINITE)) {
	case K4A_WAIT_RESULT_SUCCEEDED:
		this->gyroSampleQueue.push_back(point3D{ imuSample.gyro_sample.xyz.x, imuSample.gyro_sample.xyz.y, imuSample.gyro_sample.xyz.z });
		this->accSampleQueue.push_back(point3D{ imuSample.acc_sample.xyz.x, imuSample.acc_sample.xyz.y, imuSample.acc_sample.xyz.z });

		while (this->gyroSampleQueue.size() > MAX_GYROSCOPE_QUEUE_SIZE) this->gyroSampleQueue.pop_front();
		while (this->accSampleQueue.size() > MAX_ACCELEROMETER_QUEUE_SIZE) this->accSampleQueue.pop_front();

		return true;

	default:
		return false;
	}
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

void KinectEngine::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
{
	this->k4aImageLock.lockForRead();
	// Shallow copy
	k4a_image_t k4aColorImage = this->k4aColorImage;
	k4a_image_t k4aDepthImage = this->k4aDepthImage;
	this->k4aImageLock.unlock();

	readColorImage(colorImage, k4aColorImage);
	readDepthImage(depthImage, k4aDepthImage);
}

/*
* Retrieves image from colorImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(1080, 1920, 8UC4), empty cv::Mat if error
*/
void KinectEngine::readColorImage(cv::Mat& colorImage, k4a_image_t k4aColorImage) {
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

	// .clone() is necessary
	colorImage = cv::Mat(k4a_image_get_height_pixels(_k4aColorImage), k4a_image_get_width_pixels(_k4aColorImage), CV_8UC4, k4a_image_get_buffer(_k4aColorImage)).clone();

	return;
}

/*
* Retrieves image from depthImageQueue and transform from k4a_image_t to cv::Mat
* @return cv::Mat(576, 640, 16UC1), empty cv::Mat if error
*/
void KinectEngine::readDepthImage(cv::Mat& depthImage, k4a_image_t k4aDepthImage) {
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

	// .clone() is necessary
	depthImage = cv::Mat(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP).clone();
	return;
}

/*
* Takes a new capture and aligned the Color onto Depth
* @return cv::Mat(576, 640, 8UC4), empty cv::Mat if error
*/
void KinectEngine::readColorToDepthImage(cv::Mat& colorToDepthImage, k4a_image_t k4aColorImage, k4a_image_t k4aDepthImage) {
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
void KinectEngine::readDepthToColorImage(cv::Mat& depthToColorImage, k4a_image_t k4aColorImage, k4a_image_t k4aDepthImage) {
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

/*
* Aligned the Depth onto Color and then convert to Point Cloud image
* @return cv::Mat(1080, 1920, 16SC4), empty cv::Mat if error
*/
void KinectEngine::readPointCloudImage(cv::Mat& xyzImage)
{

	qDebug() << "readPointCloudImage";

	// Shallow copy
	k4a_image_t _k4aColorImage = this->k4aColorImage;
	k4a_image_t _k4aDepthImage = this->k4aDepthImage;

	if (_k4aColorImage == NULL || _k4aDepthImage == NULL) {
		xyzImage = cv::Mat{};
		return;
	}

	k4a_transformation_t transformationHandle = k4a_transformation_create(&this->calibration);

	// Old approach of computing on raw Depth image
	/*int width = k4a_image_get_width_pixels(_k4aDepthImage);
	int height = k4a_image_get_height_pixels(_k4aDepthImage);
	k4a_image_t pcdImage;
	if (k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM, width, height, 3 * width * (int)sizeof(int16_t), &pcdImage) != K4A_RESULT_SUCCEEDED) {
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(pcdImage);
		xyzImage = cv::Mat{};
		return;
	}
	qDebug() << "readPointCloudImage step 2a";
	if (k4a_transformation_depth_image_to_point_cloud(transformationHandle, _k4aDepthImage, K4A_CALIBRATION_TYPE_DEPTH, pcdImage) != K4A_WAIT_RESULT_SUCCEEDED) {
		qDebug() << "Failed";
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(pcdImage);
		xyzImage = cv::Mat{};
		return;
	}*/

	// New approach to compute on aligned Depth image
	// 1. get depth to color image first
	k4a_image_t alignedDepthImage;
	if (k4a_image_create(K4A_IMAGE_FORMAT_DEPTH16, k4a_image_get_width_pixels(_k4aColorImage), k4a_image_get_height_pixels(_k4aColorImage), k4a_image_get_width_pixels(_k4aColorImage) * (int)sizeof(uint16_t),
		&alignedDepthImage) != K4A_RESULT_SUCCEEDED) {
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(alignedDepthImage);
		xyzImage = cv::Mat{};
		return;
	}
	if (k4a_transformation_depth_image_to_color_camera(transformationHandle, _k4aDepthImage, alignedDepthImage) != K4A_WAIT_RESULT_SUCCEEDED) {
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(alignedDepthImage);
		xyzImage = cv::Mat{};
		return;
	}

	int width = k4a_image_get_width_pixels(alignedDepthImage);
	int height = k4a_image_get_height_pixels(alignedDepthImage);
	int stride = k4a_image_get_stride_bytes(alignedDepthImage);

	// 2. depth to color to pointcloud
	k4a_image_t pcdImage;
	if (k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM, width, height, 3 * width * (int)sizeof(int16_t), &pcdImage) != K4A_RESULT_SUCCEEDED) {
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(pcdImage);
		xyzImage = cv::Mat{};
		return;
	}
	if (k4a_transformation_depth_image_to_point_cloud(transformationHandle, alignedDepthImage, K4A_CALIBRATION_TYPE_COLOR, pcdImage) != K4A_WAIT_RESULT_SUCCEEDED) {
		qDebug() << "Failed";
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(pcdImage);
		xyzImage = cv::Mat{};
		return;
	}

	/* For Debuggubg */
	width = k4a_image_get_width_pixels(pcdImage);
	height = k4a_image_get_height_pixels(pcdImage);
	stride = k4a_image_get_stride_bytes(pcdImage);
	int16_t* pointCloudImageBuffer = (int16_t*)k4a_image_get_buffer(pcdImage);
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int pixelIndex = h * width + w;
			k4a_float3_t position = {
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 0]),
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 1]),
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 2])
			};
			pointCloudImageBuffer[3 * pixelIndex + 0] = pointCloudImageBuffer[3 * pixelIndex + 0] + 32768;
			pointCloudImageBuffer[3 * pixelIndex + 1] = pointCloudImageBuffer[3 * pixelIndex + 1] + 32768;
			pointCloudImageBuffer[3 * pixelIndex + 2] = pointCloudImageBuffer[3 * pixelIndex + 2] + 32768;
			//qDebug() << pointCloudImageBuffer[3 * pixelIndex + 0] << "," << pointCloudImageBuffer[3 * pixelIndex + 1] << "," << pointCloudImageBuffer[3 * pixelIndex + 2];
		}
	}

	// .clone() is necessary
	//xyzImage = cv::Mat(k4a_image_get_height_pixels(pcdImage), k4a_image_get_width_pixels(pcdImage), CV_16UC3, k4a_image_get_buffer(pcdImage), cv::Mat::AUTO_STEP).clone();
	xyzImage = cv::Mat(k4a_image_get_height_pixels(pcdImage), k4a_image_get_width_pixels(pcdImage), CV_16UC3, k4a_image_get_buffer(pcdImage), cv::Mat::AUTO_STEP).clone();

	k4a_transformation_destroy(transformationHandle);
	k4a_image_release(alignedDepthImage);
	k4a_image_release(pcdImage);

	return;
}

#pragma endregion

QImage convertColorCVToQImage(cv::Mat cvImage) {
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	cv::Mat temp;

	cvtColor(cvImage, temp, cv::COLOR_BGRA2RGB);

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

QImage convertDepthCVToQImage(cv::Mat cvImage)
{
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	//cvImage.convertTo(cvImage, CV_16U, 255.0 / 5000.0, 0.0);

	cv::Mat temp;
	cvtColor(cvImage, temp, cv::COLOR_BGRA2RGBA);

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGBX64);
	qImage.bits();

	return qImage;
}

QImage convertDepthCVToColorizedQImage(cv::Mat cvImage) {
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	cvImage.convertTo(cvImage, CV_8U, 255.0 / 5000.0, 0.0);

	/** Colorize depth image */
	cv::Mat temp;
	colorizeDepth(cvImage, temp);
	/** Colorize depth image END */

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

QImage convertColorToDepthCVToQImage(cv::Mat cvImage) {
	return convertColorCVToQImage(cvImage);
}

QImage convertDepthToColorCVToQImage(cv::Mat cvImage)
{
	return convertDepthCVToQImage(cvImage);
}

QImage convertDepthToColorCVToColorizedQImage(cv::Mat cvImage) {
	return convertDepthCVToColorizedQImage(cvImage);
}

QImage convertDepthToColorCVToColorizedQImageDetailed(cv::Mat cvImage) {
	if (cvImage.empty()) {
		// QImage.isNull() will return true
		return QImage();
	}

	// per unit is now (5000/255) mm = 19.6 mm
	cvImage.convertTo(cvImage, CV_8U, 255.0 / 5000.0, 0.0);

	uchar midDepth = findClosestNonZeroDepth(cvImage, cvImage.cols, cvImage.rows);

	float lowerBound = 5.0; // 1 = 20mm, 5 = 1cm
	float upperBound = 7.5; // 15 = 2cm
	float lowerThreshold = midDepth - lowerBound;
	float upperThreshold = midDepth + upperBound;

	// clamp
	if (lowerThreshold <= 0.0) {
		lowerThreshold = 0.0;
	}
	if (upperThreshold >= 255.0) {
		upperThreshold = 255.0;
	}

	// normalization according to given thresholds
	for (int y = 0; y < cvImage.rows; y++)
	{
		for (int x = 0; x < cvImage.cols; x++)
		{
			uchar d = cvImage.at<uchar>(y, x);

			// black pixels = no depth values recorded, will remain black when colorized image
			if (d == 0.0) {
				continue;
			}

			// out of range, background. Will be colored gray when colorized image
			if (d < lowerThreshold || d > upperThreshold) {
				cvImage.at<uchar>(y, x) = 255;
				continue;
			}

			float result = ((d - lowerThreshold) / (upperThreshold - lowerThreshold)) * 255;
			cvImage.at<uchar>(y, x) = result;
		}
	}


	// finding contour lines
	cv::Mat cannyImg;
	cv::Canny(cvImage, cannyImg, 10, 15);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(cannyImg, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	/** Colorize depth image */
	cv::Mat rgb;
	colorizeDepth(cvImage, rgb);
	/** Colorize depth image END */

	// plotting contour lines in white
	for (size_t i = 0; i < contours.size(); i++)
	{
		cv::Scalar color = cv::Scalar(255, 255, 255);
		cv::drawContours(rgb, contours, (int)i, color, 2, cv::LINE_8, hierarchy, 0);
	}

	// convert from cv::Mat to QImage
	QImage qImage((const uchar*)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

uchar findClosestNonZeroDepth(cv::Mat& cvImage, int cols, int rows)
{
	// get picture center point depth
	int midX = cols / 2;
	int midY = rows / 2;
	uchar midDepth = cvImage.at<uchar>(midY, midX);
	qDebug() << "First Mid Point Depth: " << midDepth << ", dx: " << 0 << ", dy: " << 0;

	int distance = 0;
	int dx = 0;
	int dy = 0;

	int NEARBY_DISTANCE = 60;
	int STEP = 3;

	while (midDepth == 0) {
		// do something else if mid point depth is 0
		//qDebug() << "Use closest point";
		distance += STEP;
		// right side
		if (midDepth == 0) {
			dx = distance;
			for (int dy = -1 * (distance - 1); dy <= distance - 1; dy += STEP) {
				if (midX + dx >= cvImage.cols || midX + dx < 0) {
					continue;
				}
				if (midY + dy >= cvImage.rows || midY + dy < 0) {
					continue;
				}
				if (!allNearbyHasDepthValue(NEARBY_DISTANCE, cvImage, midY + dy, midX + dx, cols, rows)) {
					//qDebug() << "Some nearby depth is 0, dx: " << dx << ", dy: " << dy;
					continue;
				}
				midDepth = cvImage.at<uchar>(midY + dy, midX + dx);
				//qDebug() << "Trying Mid Point Depth: " << midDepth << ", dx: " << dx << ", dy: " << dy;
				if (midDepth != 0) {
					break;
				}
			}
		}

		// left side
		if (midDepth == 0) {
			dx = -1 * distance;
			for (int dy = -1 * (distance - 1); dy <= distance - 1; dy += STEP) {
				if (midX + dx >= cvImage.cols || midX + dx < 0) {
					continue;
				}
				if (midY + dy >= cvImage.rows || midY + dy < 0) {
					continue;
				}
				if (!allNearbyHasDepthValue(NEARBY_DISTANCE, cvImage, midY + dy, midX + dx, cols, rows)) {
					//qDebug() << "Some nearby depth is 0, dx: " << dx << ", dy: " << dy;
					continue;
				}
				midDepth = cvImage.at<uchar>(midY + dy, midX + dx);
				//qDebug() << "Trying Mid Point Depth: " << midDepth << ", dx: " << dx << ", dy: " << dy;
				if (midDepth != 0) {
					break;
				}
			}
		}

		// bottom side
		if (midDepth == 0) {
			dy = distance;
			for (int dx = -1 * (distance - 1); dx <= distance - 1; dx += STEP) {
				if (midX + dx >= cvImage.cols || midX + dx < 0) {
					continue;
				}
				if (midY + dy >= cvImage.rows || midY + dy < 0) {
					continue;
				}
				if (!allNearbyHasDepthValue(NEARBY_DISTANCE, cvImage, midY + dy, midX + dx, cols, rows)) {
					//qDebug() << "Some nearby depth is 0, dx: " << dx << ", dy: " << dy;
					continue;
				}
				midDepth = cvImage.at<uchar>(midY + dy, midX + dx);
				//qDebug() << "Trying Mid Point Depth: " << midDepth << ", dx: " << dx << ", dy: " << dy;
				if (midDepth != 0) {
					break;
				}
			}
		}

		// top side
		if (midDepth == 0) {
			dy = -1 * distance;
			for (int dx = -1 * (distance - 1); dx <= distance - 1; dx += STEP) {
				if (midX + dx >= cvImage.cols || midX + dx < 0) {
					continue;
				}
				if (midY + dy >= cvImage.rows || midY + dy < 0) {
					continue;
				}
				if (!allNearbyHasDepthValue(NEARBY_DISTANCE, cvImage, midY + dy, midX + dx, cols, rows)) {
					//qDebug() << "Some nearby depth is 0, dx: " << dx << ", dy: " << dy;
					continue;
				}
				midDepth = cvImage.at<uchar>(midY + dy, midX + dx);
				//qDebug() << "Trying Mid Point Depth: " << midDepth << ", dx: " << dx << ", dy: " << dy;
				if (midDepth != 0) {
					break;
				}
			}
		}
	}

	qDebug() << "Final Mid Point Depth: " << midDepth << ", dx: " << dx << ", dy: " << dy;

	return midDepth;
}

bool allNearbyHasDepthValue(int distance, cv::Mat& cvImage, int targetY, int targetX, int cols, int rows) {
	for (int dy = targetY - distance; dy <= targetY + distance; ++dy) {
		for (int dx = targetX - distance; dx <= targetX + distance; ++dx) {
			if (targetX + dx >= cvImage.cols || targetX + dx < 0) {
				continue;
			}
			if (targetY + dy >= cvImage.rows || targetY + dy < 0) {
				continue;
			}
			if (cvImage.at<uchar>(targetY + dy, targetX + dx) == 0) {
				return false;
			}
		}
	}
	return true;
}

void colorizeDepth(const cv::Mat& gray, cv::Mat& rgb)
{
	double maxDisp = 255;
	float S = 1.f;
	float V = 1.f;

	rgb.create(gray.size(), CV_8UC3);
	rgb = cv::Scalar::all(0);

	if (maxDisp < 1)
		return;

	for (int y = 0; y < gray.rows; y++)
	{
		for (int x = 0; x < gray.cols; x++)
		{
			uchar d = gray.at<uchar>(y, x);

			// show black
			if (d == 0) {
				rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(0.f, 0.f, 0.f);
				continue;
			}

			// show gray color
			if (d == 255) {
				rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(200.f, 200.f, 200.f);
				continue;
			}

			unsigned int H = 255 - ((uchar)maxDisp - d) * 280 / (uchar)maxDisp;
			//unsigned int H = ((uchar)maxDisp - d) * 280 / (uchar)maxDisp; // red close, purple far
			unsigned int hi = (H / 60) % 6;

			float f = H / 60.f - H / 60;
			float p = V * (1 - S); // 0.f
			float q = V * (1 - f * S); // 1 - f
			float t = V * (1 - (1 - f) * S); // f

			cv::Point3f res;

			//qDebug() << d << " " << H << " " << hi << f;
			if (hi == 0) // R = V, G = t,  B = p
				res = cv::Point3f(p, t, V);
			if (hi == 1) // R = q, G = V,  B = p
				res = cv::Point3f(p, V, q);
			if (hi == 2) // R = p, G = V,  B = t
				res = cv::Point3f(t, V, p);
			if (hi == 3) // R = p, G = q,  B = V
				res = cv::Point3f(V, q, p);
			if (hi == 4) // R = t, G = p,  B = V
				res = cv::Point3f(V, p, t);
			if (hi == 5) // R = V, G = p,  B = q
				res = cv::Point3f(q, p, V);

			uchar b = (uchar)(std::max(0.f, std::min(res.x, 1.f)) * 255.f);
			uchar g = (uchar)(std::max(0.f, std::min(res.y, 1.f)) * 255.f);
			uchar r = (uchar)(std::max(0.f, std::min(res.z, 1.f)) * 255.f);

			rgb.at<cv::Point3_<uchar> >(y, x) = cv::Point3_<uchar>(b, g, r);

		}
	}
}

QVector3D KinectEngine::query3DPoint(int x, int y, cv::Mat depthToColorImage)
{
	ushort d = depthToColorImage.at<ushort>(y, x);

	//k4a_calibration_t calibration;
	//if (k4a_device_get_calibration(this->device, this->config.depth_mode, this->config.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
	//	return QVector3D(0, 0, 0);
	//}

	k4a_float2_t p;
	p.xy.x = (float)x;
	p.xy.y = (float)y;
	k4a_float3_t p3D;
	int valid;
	if (k4a_calibration_2d_to_3d(&calibration, &p, d, K4A_CALIBRATION_TYPE_COLOR, K4A_CALIBRATION_TYPE_COLOR, &p3D, &valid) != K4A_WAIT_RESULT_SUCCEEDED) {
		return QVector3D(0, 0, 0);
	}
	//qDebug() << "p3D(" << p3D.xyz.x << ", " << p3D.xyz.y << ", " << p3D.xyz.z << ")";
	// source_point2d is a valid coordinate
	if (valid == 1) {
		return QVector3D(p3D.xyz.x, p3D.xyz.y, p3D.xyz.z);
	}

	return QVector3D(0, 0, 0);
}

std::deque<point3D> KinectEngine::getGyroSampleQueue()
{
	return this->gyroSampleQueue;
}

std::deque<point3D> KinectEngine::getAccSampleQueue()
{
	return this->accSampleQueue;
}

float* KinectEngine::findPlaneEquationCoefficients(cv::Mat depthToColorImage) {
	float out[4];

	int rows = depthToColorImage.rows;
	int cols = depthToColorImage.cols;

	/** The purpose for counting how many pixels are in each interval is to
	* pick the top 2 intervals for sampling points which likely lie on the wall. */
	/**
	* Intervals should be 0, [1, 250], [251, 500], ... , [4751, 5000], [5001, 5250], [5251, 5500]
	*/
	const int MAX_DEPTH_VALUE = 5500; // Depth sensor maximum is 5XXXmm

	const int NUM_OF_INTERVALS = 23;
	const int SIZE_OF_INTERVALS = 250;
	int countOfDepth[NUM_OF_INTERVALS] = { 0 };

	std::vector<std::vector<std::pair<int, int>>> v;
	for (int i = 0; i < NUM_OF_INTERVALS; i++) {
		std::vector<std::pair<int, int>> s;
		v.push_back(s);
	}

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			uint16_t d = depthToColorImage.at<uint16_t>(y, x);

			if (d == 0) {
				countOfDepth[0]++;
				v[0].push_back({ x, y });
			}
			else {
				int i = (int)((d - 1) / SIZE_OF_INTERVALS) + 1;
				countOfDepth[i]++;
				v[i].push_back({ x, y });
			}
		}
	}

	for (int i = 0; i < NUM_OF_INTERVALS; i++) {
		if (i == 0) {
			qDebug() << "countOfDepth[0] = " << countOfDepth[0];
		}
		else {
			qDebug() << "countOfDepth[" << (i - 1) * SIZE_OF_INTERVALS + 1 << " - " << i * SIZE_OF_INTERVALS << "] = " << countOfDepth[i];
		}
	}
	/**
	* Intervals should be 0, [1, 250], [251, 500], ... , [4751, 5000]
	* END */

	/** Pick the top 2 intervals. Interval "0" is not considered. */
	int largest = -1;
	int secondLargest = -1;
	int largestIndex = -1;
	int secondLargestIndex = -1;
	for (int i = 1; i < NUM_OF_INTERVALS; i++) {
		if (countOfDepth[i] > largest) {
			largest = countOfDepth[i];
			largestIndex = i;
		}
		else if (countOfDepth[i] > secondLargest) {
			secondLargest = countOfDepth[i];
			secondLargestIndex = i;
		}
	}

	qDebug() << "largest index is" << largestIndex;
	qDebug() << "second largest index is" << secondLargestIndex;

	qDebug() << "First set of sample points is" << v[largestIndex];
	qDebug() << "Second set of sample points is" << v[secondLargestIndex];
	/** Pick the top 2 intervals. Interval "0" is not considered. END */

	return out;
}

// https://www.youtube.com/watch?v=rL9UXzZYYo4&ab_channel=TheOrganicChemistryTutor
float* KinectEngine::findPlaneEquationCoefficients(
	float x1, float y1, float z1,
	float x2, float y2, float z2,
	float x3, float y3, float z3)
{
	float out[4];
	float a1 = x2 - x1;
	float b1 = y2 - y1;
	float c1 = z2 - z1;
	float a2 = x3 - x1;
	float b2 = y3 - y1;
	float c2 = z3 - z1;
	float a = b1 * c2 - b2 * c1;
	float b = a2 * c1 - a1 * c2;
	float c = a1 * b2 - b1 * a2;
	float d = (-a * x1 - b * y1 - c * z1);

	out[0] = a;
	out[1] = b;
	out[2] = c;
	out[3] = d;

	return out;
}

// https://www.cuemath.com/geometry/distance-between-point-and-plane/
float KinectEngine::findDistanceBetween3DPointAndPlane(
	float x1, float y1, float z1,
	float a, float b, float c,
	float d) {
	d = fabs((a * x1 + b * y1 +
		c * z1 + d));
	float e = sqrt(a * a + b *
		b + c * c);

	return d / e;
}

void KinectEngine::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
{
	// RANSAC
	int k = 6;
	int threshold = 15;

	int iterationCount = 0;
	int inlierCount = 0;

	int MaxInlierCount = -1;
	float PlaneA, PlaneB, PlaneC, PlaneD;
	float BestPointOne[2];
	float BestPointTwo[2];
	float BestPointThree[2];

	float a, b, c, d;
	float PointOne[2];
	float PointTwo[2];
	float PointThree[2];

	srand((unsigned)time(0));

	int IgnoreLeftAndRightPixel = 150;
	int IgnoreMiddlePixel = 150;

	// 565 and 715 for Realsense
	int MIN = depthToColorImage.cols / 2 - 75;
	int MAX = depthToColorImage.cols / 2 + 75;

	while (iterationCount <= k) {

		inlierCount = 0;

		// First point
		while (true) {
			PointOne[0] = rand() % depthToColorImage.cols;
			PointOne[1] = rand() % depthToColorImage.rows;
			if (PointOne[0] < IgnoreLeftAndRightPixel && PointOne[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}

			if (PointOne[0] > MIN && PointOne[0] < MAX) {
				continue;
			}
			QVector3D vector3D_1 = KinectEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
			if (vector3D_1.x() == 0.0f && vector3D_1.y() == 0.0f && vector3D_1.z() == 0.0f) {
				continue;
			}
			else {
				break;
			}
		}

		// Second point
		while (true) {
			PointTwo[0] = rand() % depthToColorImage.cols;
			PointTwo[1] = rand() % depthToColorImage.rows;
			if (PointTwo[0] < IgnoreLeftAndRightPixel && PointTwo[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointTwo[0] > MIN && PointTwo[0] < MAX) {
				continue;
			}
			QVector3D vector3D_2 = KinectEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
			if (vector3D_2.x() == 0.0f && vector3D_2.y() == 0.0f && vector3D_2.z() == 0.0f) {
				continue;
			}
			if (sqrt(pow(PointTwo[0] - PointOne[0], 2) + pow(PointTwo[1] - PointOne[1], 2) * 1.0) <= 100) {
				continue;
			}
			break;
		}

		// Third point
		while (true) {
			PointThree[0] = rand() % depthToColorImage.cols;
			PointThree[1] = rand() % depthToColorImage.rows;
			if (PointThree[0] < IgnoreLeftAndRightPixel && PointThree[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointThree[0] > MIN && PointThree[0] < MAX) {
				continue;
			}
			QVector3D vector3D_3 = KinectEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);
			if (vector3D_3.x() == 0.0f && vector3D_3.y() == 0.0f && vector3D_3.z() == 0.0f) {
				continue;
			}
			if (sqrt(pow(PointThree[0] - PointOne[0], 2) + pow(PointThree[1] - PointOne[1], 2) * 1.0) <= 100) {
				continue;
			}
			if (sqrt(pow(PointThree[0] - PointTwo[0], 2) + pow(PointThree[1] - PointTwo[1], 2) * 1.0) <= 100) {
				continue;
			}
			break;
		}

		QVector3D vector3D_1 = KinectEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
		QVector3D vector3D_2 = KinectEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
		QVector3D vector3D_3 = KinectEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);

		float* abcd;
		abcd = KinectEngine::getInstance().findPlaneEquationCoefficients(
			vector3D_1.x(), vector3D_1.y(), vector3D_1.z(),
			vector3D_2.x(), vector3D_2.y(), vector3D_2.z(),
			vector3D_3.x(), vector3D_3.y(), vector3D_3.z()
		);
		a = abcd[0];
		b = abcd[1];
		c = abcd[2];
		d = abcd[3];
		/*qDebug() << "Equation of plane is " << a << " x + " << b
			<< " y + " << c << " z + " << d << " = 0.";*/

		for (int y = 0; y < depthToColorImage.rows; y += 2) {
			for (int x = 0; x < depthToColorImage.cols; x += 2) {
				if (x > MIN && x < MAX) {
					continue;
				}
				if (x < IgnoreLeftAndRightPixel || x > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
					continue;
				}
				QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, depthToColorImage);
				if (vector3D.x() == 0.0f && vector3D.y() == 0.0f && vector3D.z() == 0.0f) {
					continue;
				}

				float distance = KinectEngine::getInstance().findDistanceBetween3DPointAndPlane(vector3D.x(), vector3D.y(), vector3D.z(), a, b, c, d);
				if (distance <= threshold) {
					inlierCount++;
				}
			}
		}

		if (inlierCount > MaxInlierCount) {
			PlaneA = a;
			PlaneB = b;
			PlaneC = c;
			PlaneD = d;
			BestPointOne[0] = PointOne[0];
			BestPointOne[1] = PointOne[1];
			BestPointTwo[0] = PointTwo[0];
			BestPointTwo[1] = PointTwo[1];
			BestPointThree[0] = PointThree[0];
			BestPointThree[1] = PointThree[1];

			MaxInlierCount = inlierCount;
		}

		iterationCount++;
		//qDebug() << "Inliers: " << inlierCount;

	}
	//qDebug() << "Max Inliers: " << MaxInlierCount;

	// computer actual image
	out = cv::Mat::zeros(depthToColorImage.rows, depthToColorImage.cols, CV_16UC1);
	float maxDistance = 0.0f;
	for (int y = 0; y < depthToColorImage.rows; y++) {
		for (int x = 0; x < depthToColorImage.cols; x++) {

			QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, depthToColorImage);

			if (vector3D.x() == 0.0f && vector3D.y() == 0.0f && vector3D.z() == 0.0f) {
				out.at<uint16_t>(y, x) = 0.0f;
				continue;
			}

			float distance = KinectEngine::getInstance().findDistanceBetween3DPointAndPlane(vector3D.x(), vector3D.y(), vector3D.z(), PlaneA, PlaneB, PlaneC, PlaneD);
			out.at<uint16_t>(y, x) = distance;
			/*if (distance <= threshold) {
				out.at<uint16_t>(y, x) = 5000;
			}*/
			if (distance > maxDistance) {
				maxDistance = distance;
			}
		}
	}
	//out.at<uint16_t>(BestPointOne[1], BestPointOne[0]) = 5000;
	//out.at<uint16_t>(BestPointTwo[1], BestPointTwo[0]) = 5000;
	//out.at<uint16_t>(BestPointThree[1], BestPointThree[0]) = 5000;
}

cv::Mat KinectEngine::readCVImageFromFile(std::wstring filename)
{
	const wchar_t* widecstr = filename.c_str();

	FILE* fp = _wfopen(widecstr, L"rb");
	if (!fp)
	{
		return cv::Mat();
	}
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	char* buf = new char[sz];
	fseek(fp, 0, SEEK_SET);
	long n = fread(buf, 1, sz, fp);
	cv::_InputArray arr(buf, sz);
	cv::Mat img = cv::imdecode(arr, -1);
	delete[] buf;
	fclose(fp);
	return img;
}

void KinectEngine::writeCalibrationToFile(k4a_calibration_t& calibration)
{
	std::ofstream fw("intrinsics_kinect.txt", std::ofstream::out);
	if (fw.is_open())
	{
		// color camera
		fw << "color_extrinsics_rotation: " << std::endl;
		for (int i = 0; i < 9; ++i) {
			fw << calibration.color_camera_calibration.extrinsics.rotation[i] << std::endl;
		}

		fw << "color_extrinsics_translation: " << std::endl;
		for (int i = 0; i < 3; ++i) {
			fw << calibration.color_camera_calibration.extrinsics.translation[i] << std::endl;
		}

		fw << "color_intrinsics_param: " << std::endl;
		for (int i = 0; i < 15; ++i) {
			fw << calibration.color_camera_calibration.intrinsics.parameters.v[i] << std::endl;
		}

		fw << "color_intrinsics_param_count: " << calibration.color_camera_calibration.intrinsics.parameter_count << std::endl;
		fw << "color_intrinsics_type: " << calibration.color_camera_calibration.intrinsics.type << std::endl;
		fw << "color_intrinsics_metric_radius: " << calibration.color_camera_calibration.metric_radius << std::endl;
		fw << "color_intrinsics_resolution_height: " << calibration.color_camera_calibration.resolution_height << std::endl;
		fw << "color_intrinsics_resolution_width: " << calibration.color_camera_calibration.resolution_width << std::endl;
		//

		fw << "color_resolution: " << calibration.color_resolution << std::endl;

		// depth camera
		fw << "depth_extrinsics_rotation: " << std::endl;
		for (int i = 0; i < 9; ++i) {
			fw << calibration.depth_camera_calibration.extrinsics.rotation[i] << std::endl;
		}

		fw << "depth_extrinsics_translation: " << std::endl;
		for (int i = 0; i < 3; ++i) {
			fw << calibration.depth_camera_calibration.extrinsics.translation[i] << std::endl;
		}

		fw << "depth_intrinsics_param: " << std::endl;
		for (int i = 0; i < 15; ++i) {
			fw << calibration.depth_camera_calibration.intrinsics.parameters.v[i] << std::endl;
		}

		fw << "depth_intrinsics_param_count: " << calibration.depth_camera_calibration.intrinsics.parameter_count << std::endl;
		fw << "depth_intrinsics_type: " << calibration.depth_camera_calibration.intrinsics.type << std::endl;
		fw << "depth_intrinsics_metric_radius: " << calibration.depth_camera_calibration.metric_radius << std::endl;
		fw << "depth_intrinsics_resolution_height: " << calibration.depth_camera_calibration.resolution_height << std::endl;
		fw << "depth_intrinsics_resolution_width: " << calibration.depth_camera_calibration.resolution_width << std::endl;
		//

		fw << "depth_mode: " << calibration.depth_mode << std::endl;

		// extrinsics
		for (int p = 0; p < 4; ++p) {
			for (int q = 0; q < 4; ++q) {
				fw << "extrinsics[" << p << "][" << q << "]: " << std::endl;
				fw << "rotation: " << std::endl;
				for (int i = 0; i < 9; ++i) {
					fw << calibration.extrinsics[p][q].rotation[i] << std::endl;
				}
				fw << "translation: " << std::endl;
				for (int i = 0; i < 3; ++i) {
					fw << calibration.extrinsics[p][q].translation[i] << std::endl;
				}
				fw << std::endl;
			}
		}

		fw.close();
	}
}

void KinectEngine::readIntrinsicsFromFile(std::string path)
{
	// HARDCODE (same as reading from intrinsics_kinect.txt)
	calibration.color_resolution = K4A_COLOR_RESOLUTION_1080P;

	{
		// color camera
		_k4a_calibration_extrinsics_t extrinsics;
		extrinsics.rotation[0] = 0.999983f;
		extrinsics.rotation[1] = 0.00585299f;
		extrinsics.rotation[2] = -0.000753369f;
		extrinsics.rotation[3] = -0.00577472f;
		extrinsics.rotation[4] = 0.996828f;
		extrinsics.rotation[5] = 0.0793804f;
		extrinsics.rotation[6] = 0.00121559f;
		extrinsics.rotation[7] = -0.0793747f;
		extrinsics.rotation[8] = 0.996844f;
		extrinsics.translation[0] = -32.0571f;
		extrinsics.translation[1] = -2.21997f;
		extrinsics.translation[2] = 3.94995f;
		_k4a_calibration_intrinsics_t intrinsics;
		intrinsics.parameters.v[0] = 959.033f;
		intrinsics.parameters.v[1] = 549.389f;
		intrinsics.parameters.v[2] = 909.969f;
		intrinsics.parameters.v[3] = 909.339f;
		intrinsics.parameters.v[4] = 0.70917f;
		intrinsics.parameters.v[5] = -2.86673f;
		intrinsics.parameters.v[6] = 1.62618f;
		intrinsics.parameters.v[7] = 0.588996f;
		intrinsics.parameters.v[8] = -2.70083f;
		intrinsics.parameters.v[9] = 1.55868f;
		intrinsics.parameters.v[10] = 0.0f;
		intrinsics.parameters.v[11] = 0.0f;
		intrinsics.parameters.v[12] = -0.000397904f;
		intrinsics.parameters.v[13] = 0.000974435f;
		intrinsics.parameters.v[14] = 0.0f;
		intrinsics.parameters.param.cx = intrinsics.parameters.v[0];
		intrinsics.parameters.param.cy = intrinsics.parameters.v[1];
		intrinsics.parameters.param.fx = intrinsics.parameters.v[2];
		intrinsics.parameters.param.fy = intrinsics.parameters.v[3];
		intrinsics.parameters.param.k1 = intrinsics.parameters.v[4];
		intrinsics.parameters.param.k2 = intrinsics.parameters.v[5];
		intrinsics.parameters.param.k3 = intrinsics.parameters.v[6];
		intrinsics.parameters.param.k4 = intrinsics.parameters.v[7];
		intrinsics.parameters.param.k5 = intrinsics.parameters.v[8];
		intrinsics.parameters.param.k6 = intrinsics.parameters.v[9];
		intrinsics.parameters.param.codx = intrinsics.parameters.v[10];
		intrinsics.parameters.param.cody = intrinsics.parameters.v[11];
		intrinsics.parameters.param.p2 = intrinsics.parameters.v[12];
		intrinsics.parameters.param.p1 = intrinsics.parameters.v[13];
		intrinsics.parameters.param.metric_radius = intrinsics.parameters.v[14];
		intrinsics.parameter_count = 14;
		intrinsics.type = K4A_CALIBRATION_LENS_DISTORTION_MODEL_BROWN_CONRADY;

		calibration.color_camera_calibration.extrinsics = extrinsics;
		calibration.color_camera_calibration.intrinsics = intrinsics;
		calibration.color_camera_calibration.metric_radius = 1.7f;
		calibration.color_camera_calibration.resolution_height = 1080;
		calibration.color_camera_calibration.resolution_width = 1920;
		calibration.color_resolution = K4A_COLOR_RESOLUTION_1080P;
	}

	{
		// depth camera
		_k4a_calibration_extrinsics_t extrinsics;
		extrinsics.rotation[0] = 1.0f;
		extrinsics.rotation[1] = 0.0f;
		extrinsics.rotation[2] = 0.0f;
		extrinsics.rotation[3] = 0.0f;
		extrinsics.rotation[4] = 1.0f;
		extrinsics.rotation[5] = 0.0f;
		extrinsics.rotation[6] = 0.0f;
		extrinsics.rotation[7] = 0.0f;
		extrinsics.rotation[8] = 1.0f;
		extrinsics.translation[0] = 0.0f;
		extrinsics.translation[1] = 0.0f;
		extrinsics.translation[2] = 0.0f;
		_k4a_calibration_intrinsics_t intrinsics;
		intrinsics.parameters.v[0] = 322.074f;
		intrinsics.parameters.v[1] = 354.181f;
		intrinsics.parameters.v[2] = 504.296f;
		intrinsics.parameters.v[3] = 504.304f;
		intrinsics.parameters.v[4] = 3.40328f;
		intrinsics.parameters.v[5] = 2.33724f;
		intrinsics.parameters.v[6] = 0.125497f;
		intrinsics.parameters.v[7] = 3.73403f;
		intrinsics.parameters.v[8] = 3.45251f;
		intrinsics.parameters.v[9] = 0.652264f;
		intrinsics.parameters.v[10] = 0.0f;
		intrinsics.parameters.v[11] = 0.0f;
		intrinsics.parameters.v[12] = 0.0000413194f;
		intrinsics.parameters.v[13] = 0.0000122039f;
		intrinsics.parameters.v[14] = 0.0f;
		intrinsics.parameters.param.cx = intrinsics.parameters.v[0];
		intrinsics.parameters.param.cy = intrinsics.parameters.v[1];
		intrinsics.parameters.param.fx = intrinsics.parameters.v[2];
		intrinsics.parameters.param.fy = intrinsics.parameters.v[3];
		intrinsics.parameters.param.k1 = intrinsics.parameters.v[4];
		intrinsics.parameters.param.k2 = intrinsics.parameters.v[5];
		intrinsics.parameters.param.k3 = intrinsics.parameters.v[6];
		intrinsics.parameters.param.k4 = intrinsics.parameters.v[7];
		intrinsics.parameters.param.k5 = intrinsics.parameters.v[8];
		intrinsics.parameters.param.k6 = intrinsics.parameters.v[9];
		intrinsics.parameters.param.codx = intrinsics.parameters.v[10];
		intrinsics.parameters.param.cody = intrinsics.parameters.v[11];
		intrinsics.parameters.param.p2 = intrinsics.parameters.v[12];
		intrinsics.parameters.param.p1 = intrinsics.parameters.v[13];
		intrinsics.parameters.param.metric_radius = intrinsics.parameters.v[14];
		intrinsics.parameter_count = 14;
		intrinsics.type = K4A_CALIBRATION_LENS_DISTORTION_MODEL_BROWN_CONRADY;

		calibration.depth_camera_calibration.extrinsics = extrinsics;
		calibration.depth_camera_calibration.intrinsics = intrinsics;
		calibration.depth_camera_calibration.metric_radius = 1.74f;
		calibration.depth_camera_calibration.resolution_height = 576;
		calibration.depth_camera_calibration.resolution_width = 640;
		calibration.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	}

	{
		calibration.extrinsics[0][0].rotation[0] = 1.0f;
		calibration.extrinsics[0][0].rotation[1] = 0.0f;
		calibration.extrinsics[0][0].rotation[2] = 0.0f;
		calibration.extrinsics[0][0].rotation[3] = 0.0f;
		calibration.extrinsics[0][0].rotation[4] = 1.0f;
		calibration.extrinsics[0][0].rotation[5] = 0.0f;
		calibration.extrinsics[0][0].rotation[6] = 0.0f;
		calibration.extrinsics[0][0].rotation[7] = 0.0f;
		calibration.extrinsics[0][0].rotation[8] = 1.0f;
		calibration.extrinsics[0][0].translation[0] = 0.0f;
		calibration.extrinsics[0][0].translation[1] = 0.0f;
		calibration.extrinsics[0][0].translation[2] = 0.0f;
	}

	{
		calibration.extrinsics[0][1].rotation[0] = 0.999983f;
		calibration.extrinsics[0][1].rotation[1] = 0.00585299f;
		calibration.extrinsics[0][1].rotation[2] = -0.000753369f;
		calibration.extrinsics[0][1].rotation[3] = -0.00577472f;
		calibration.extrinsics[0][1].rotation[4] = 0.996828f;
		calibration.extrinsics[0][1].rotation[5] = 0.0793804f;
		calibration.extrinsics[0][1].rotation[6] = 0.00121559f;
		calibration.extrinsics[0][1].rotation[7] = -0.0793747f;
		calibration.extrinsics[0][1].rotation[8] = 0.996844f;
		calibration.extrinsics[0][1].translation[0] = -32.0571f;
		calibration.extrinsics[0][1].translation[1] = -2.21997f;
		calibration.extrinsics[0][1].translation[2] = 3.94995f;
	}

	{
		calibration.extrinsics[0][2].rotation[0] = 5.3255e-05f;
		calibration.extrinsics[0][2].rotation[1] = 0.107066f;
		calibration.extrinsics[0][2].rotation[2] = -0.994252f;
		calibration.extrinsics[0][2].rotation[3] = -0.999952f;
		calibration.extrinsics[0][2].rotation[4] = -0.00974179f;
		calibration.extrinsics[0][2].rotation[5] = -0.00110261f;
		calibration.extrinsics[0][2].rotation[6] = -0.00980385f;
		calibration.extrinsics[0][2].rotation[7] = 0.994204f;
		calibration.extrinsics[0][2].rotation[8] = 0.10706f;
		calibration.extrinsics[0][2].translation[0] = 0.0f;
		calibration.extrinsics[0][2].translation[1] = 0.0f;
		calibration.extrinsics[0][2].translation[2] = 0.0f;
	}

	{
		calibration.extrinsics[0][3].rotation[0] = 0.00440682f;
		calibration.extrinsics[0][3].rotation[1] = 0.104693f;
		calibration.extrinsics[0][3].rotation[2] = -0.994495f;
		calibration.extrinsics[0][3].rotation[3] = -0.999976f;
		calibration.extrinsics[0][3].rotation[4] = -0.00477878f;
		calibration.extrinsics[0][3].rotation[5] = -0.00493418f;
		calibration.extrinsics[0][3].rotation[6] = -0.00526905f;
		calibration.extrinsics[0][3].rotation[7] = 0.994493f;
		calibration.extrinsics[0][3].rotation[8] = 0.10467f;
		calibration.extrinsics[0][3].translation[0] = -50.9456f;
		calibration.extrinsics[0][3].translation[1] = 3.34612f;
		calibration.extrinsics[0][3].translation[2] = 1.42621f;
	}

	{
		calibration.extrinsics[1][0].rotation[0] = 0.999983f;
		calibration.extrinsics[1][0].rotation[1] = -0.00577472f;
		calibration.extrinsics[1][0].rotation[2] = 0.00121559f;
		calibration.extrinsics[1][0].rotation[3] = 0.00585299f;
		calibration.extrinsics[1][0].rotation[4] = 0.996828f;
		calibration.extrinsics[1][0].rotation[5] = -0.0793747f;
		calibration.extrinsics[1][0].rotation[6] = -0.000753369f;
		calibration.extrinsics[1][0].rotation[7] = 0.0793804f;
		calibration.extrinsics[1][0].rotation[8] = 0.996844f;
		calibration.extrinsics[1][0].translation[0] = 32.0389f;
		calibration.extrinsics[1][0].translation[1] = 2.71409f;
		calibration.extrinsics[1][0].translation[2] = -3.78541f;
	}

	{
		calibration.extrinsics[1][1].rotation[0] = 1.0f;
		calibration.extrinsics[1][1].rotation[1] = -2.40107e-10f;
		calibration.extrinsics[1][1].rotation[2] = -5.82077e-11f;
		calibration.extrinsics[1][1].rotation[3] = -2.40107e-10f;
		calibration.extrinsics[1][1].rotation[4] = 1.0f;
		calibration.extrinsics[1][1].rotation[5] = 0.0f;
		calibration.extrinsics[1][1].rotation[6] = -5.82077e-11f;
		calibration.extrinsics[1][1].rotation[7] = 0.0f;
		calibration.extrinsics[1][1].rotation[8] = 1.0f;
		calibration.extrinsics[1][1].translation[0] = 0.0f;
		calibration.extrinsics[1][1].translation[1] = 2.38419e-07f;
		calibration.extrinsics[1][1].translation[2] = 0.0f;
	}

	{
		calibration.extrinsics[1][2].rotation[0] = 0.00142895f;
		calibration.extrinsics[1][2].rotation[1] = 0.0278021f;
		calibration.extrinsics[1][2].rotation[2] = -0.999612f;
		calibration.extrinsics[1][2].rotation[3] = -0.999991f;
		calibration.extrinsics[1][2].rotation[4] = -0.00402398f;
		calibration.extrinsics[1][2].rotation[5] = -0.00154141f;
		calibration.extrinsics[1][2].rotation[6] = -0.00406527f;
		calibration.extrinsics[1][2].rotation[7] = 0.999605f;
		calibration.extrinsics[1][2].rotation[8] = 0.027796f;
		calibration.extrinsics[1][2].translation[0] = 4.05594f;
		calibration.extrinsics[1][2].translation[1] = -32.0597f;
		calibration.extrinsics[1][2].translation[2] = 1.97898f;
	}

	{
		calibration.extrinsics[1][3].rotation[0] = 0.00576873f;
		calibration.extrinsics[1][3].rotation[1] = 0.0253922f;
		calibration.extrinsics[1][3].rotation[2] = -0.999661f;
		calibration.extrinsics[1][3].rotation[3] = -0.999983f;
		calibration.extrinsics[1][3].rotation[4] = 0.000619282f;
		calibration.extrinsics[1][3].rotation[5] = -0.00575486f;
		calibration.extrinsics[1][3].rotation[6] = 0.000472943f;
		calibration.extrinsics[1][3].rotation[7] = 0.999677f;
		calibration.extrinsics[1][3].rotation[8] = 0.0253953f;
		calibration.extrinsics[1][3].translation[0] = -46.7557f;
		calibration.extrinsics[1][3].translation[1] = -28.6864f;
		calibration.extrinsics[1][3].translation[2] = 3.56031f;
	}

	{
		calibration.extrinsics[2][0].rotation[0] = 5.3255e-05f;
		calibration.extrinsics[2][0].rotation[1] = -0.999952f;
		calibration.extrinsics[2][0].rotation[2] = -0.00980385f;
		calibration.extrinsics[2][0].rotation[3] = 0.107066f;
		calibration.extrinsics[2][0].rotation[4] = -0.00974179f;
		calibration.extrinsics[2][0].rotation[5] = 0.994204f;
		calibration.extrinsics[2][0].rotation[6] = -0.994252f;
		calibration.extrinsics[2][0].rotation[7] = -0.00110261f;
		calibration.extrinsics[2][0].rotation[8] = 0.10706f;
		calibration.extrinsics[2][0].translation[0] = 0.0f;
		calibration.extrinsics[2][0].translation[1] = 0.0f;
		calibration.extrinsics[2][0].translation[2] = 0.0f;
	}

	{
		calibration.extrinsics[2][1].rotation[0] = 0.00142895f;
		calibration.extrinsics[2][1].rotation[1] = -0.999991f;
		calibration.extrinsics[2][1].rotation[2] = -0.00406527f;
		calibration.extrinsics[2][1].rotation[3] = 0.0278021f;
		calibration.extrinsics[2][1].rotation[4] = -0.00402398f;
		calibration.extrinsics[2][1].rotation[5] = 0.999605f;
		calibration.extrinsics[2][1].rotation[6] = -0.999612f;
		calibration.extrinsics[2][1].rotation[7] = -0.00154141f;
		calibration.extrinsics[2][1].rotation[8] = 0.027796f;
		calibration.extrinsics[2][1].translation[0] = -32.0571f;
		calibration.extrinsics[2][1].translation[1] = -2.21997f;
		calibration.extrinsics[2][1].translation[2] = 3.94995f;
	}

	{
		calibration.extrinsics[2][2].rotation[0] = 1.0f;
		calibration.extrinsics[2][2].rotation[1] = 0.0f;
		calibration.extrinsics[2][2].rotation[2] = 7.45058e-09f;
		calibration.extrinsics[2][2].rotation[3] = 0.0f;
		calibration.extrinsics[2][2].rotation[4] = 1.0f;
		calibration.extrinsics[2][2].rotation[5] = -4.51109e-10f;
		calibration.extrinsics[2][2].rotation[6] = 7.45058e-09f;
		calibration.extrinsics[2][2].rotation[7] = -4.51109e-10f;
		calibration.extrinsics[2][2].rotation[8] = 1.0f;
		calibration.extrinsics[2][2].translation[0] = 0.0f;
		calibration.extrinsics[2][2].translation[1] = 0.0f;
		calibration.extrinsics[2][2].translation[2] = 0.0f;
	}

	{
		calibration.extrinsics[2][3].rotation[0] = 0.999988f;
		calibration.extrinsics[2][3].rotation[1] = -0.00432997f;
		calibration.extrinsics[2][3].rotation[2] = -0.00242788f;
		calibration.extrinsics[2][3].rotation[3] = 0.00434092f;
		calibration.extrinsics[2][3].rotation[4] = 0.99998f;
		calibration.extrinsics[2][3].rotation[5] = 0.00452428f;
		calibration.extrinsics[2][3].rotation[6] = 0.00240824f;
		calibration.extrinsics[2][3].rotation[7] = -0.00453476f;
		calibration.extrinsics[2][3].rotation[8] = 0.999987f;
		calibration.extrinsics[2][3].translation[0] = -50.9456f;
		calibration.extrinsics[2][3].translation[1] = 3.34612f;
		calibration.extrinsics[2][3].translation[2] = 1.42621f;
	}

	{
		calibration.extrinsics[3][0].rotation[0] = 0.00440682f;
		calibration.extrinsics[3][0].rotation[1] = -0.999976f;
		calibration.extrinsics[3][0].rotation[2] = -0.00526905f;
		calibration.extrinsics[3][0].rotation[3] = 0.104693f;
		calibration.extrinsics[3][0].rotation[4] = -0.00477878f;
		calibration.extrinsics[3][0].rotation[5] = 0.994493f;
		calibration.extrinsics[3][0].rotation[6] = -0.994495f;
		calibration.extrinsics[3][0].rotation[7] = -0.00493418f;
		calibration.extrinsics[3][0].rotation[8] = 0.10467f;
		calibration.extrinsics[3][0].translation[0] = 3.57807f;
		calibration.extrinsics[3][0].translation[1] = 3.9313f;
		calibration.extrinsics[3][0].translation[2] = -50.7979f;
	}

	{
		calibration.extrinsics[3][1].rotation[0] = 0.00576873f;
		calibration.extrinsics[3][1].rotation[1] = -0.999983f;
		calibration.extrinsics[3][1].rotation[2] = 0.000472943f;
		calibration.extrinsics[3][1].rotation[3] = 0.0253922f;
		calibration.extrinsics[3][1].rotation[4] = 0.000619282f;
		calibration.extrinsics[3][1].rotation[5] = 0.999677f;
		calibration.extrinsics[3][1].rotation[6] = -0.999661f;
		calibration.extrinsics[3][1].rotation[7] = -0.00575486f;
		calibration.extrinsics[3][1].rotation[8] = 0.0253953f;
		calibration.extrinsics[3][1].translation[0] = -28.4178f;
		calibration.extrinsics[3][1].translation[1] = -2.35417f;
		calibration.extrinsics[3][1].translation[2] = -46.9954f;
	}

	{
		calibration.extrinsics[3][2].rotation[0] = 0.999988f;
		calibration.extrinsics[3][2].rotation[1] = 0.00434092f;
		calibration.extrinsics[3][2].rotation[2] = 0.00240824f;
		calibration.extrinsics[3][2].rotation[3] = -0.00432997f;
		calibration.extrinsics[3][2].rotation[4] = 0.99998f;
		calibration.extrinsics[3][2].rotation[5] = -0.00453476f;
		calibration.extrinsics[3][2].rotation[6] = -0.00242788f;
		calibration.extrinsics[3][2].rotation[7] = 0.00452428f;
		calibration.extrinsics[3][2].rotation[8] = 0.999987f;
		calibration.extrinsics[3][2].translation[0] = 50.927f;
		calibration.extrinsics[3][2].translation[1] = -3.56018f;
		calibration.extrinsics[3][2].translation[2] = -1.56502f;
	}

	{
		calibration.extrinsics[3][3].rotation[0] = 1.0f;
		calibration.extrinsics[3][3].rotation[1] = 0.0f;
		calibration.extrinsics[3][3].rotation[2] = 0.0f;
		calibration.extrinsics[3][3].rotation[3] = 0.0f;
		calibration.extrinsics[3][3].rotation[4] = 1.0f;
		calibration.extrinsics[3][3].rotation[5] = 0.0f;
		calibration.extrinsics[3][3].rotation[6] = 0.0f;
		calibration.extrinsics[3][3].rotation[7] = 0.0f;
		calibration.extrinsics[3][3].rotation[8] = 1.0f;
		calibration.extrinsics[3][3].translation[0] = 3.8147e-06f;
		calibration.extrinsics[3][3].translation[1] = 0.0f;
		calibration.extrinsics[3][3].translation[2] = -3.57628e-07f;
	}
}
