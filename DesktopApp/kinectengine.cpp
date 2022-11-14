#include "kinectengine.h"
#include <QtWidgets/QWidget>
#include <k4a/k4a.hpp>
#include "stdafx.h"

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
		this->gyroSampleQueue.push_back(imuSample.gyro_sample);
		this->accSampleQueue.push_back(imuSample.acc_sample);

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

	// Not sure if .clone() is necessary
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

void KinectEngine::readPointCloudImage(cv::Mat& xyzImage, k4a_image_t k4aDepthImage)
{
	// Shallow copy
	k4a_image_t _k4aDepthImage = this->k4aDepthImage;

	if (k4aDepthImage != NULL) {
		_k4aDepthImage = k4aDepthImage;
	}
	else {
		if (_k4aDepthImage == NULL) {
			xyzImage = cv::Mat{};
			return;
		}
	}

	k4a_transformation_t transformationHandle = k4a_transformation_create(&this->calibration);

	k4a_image_t alignmentImage;
	if (k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM, 
		k4a_image_get_width_pixels(_k4aDepthImage), 
		k4a_image_get_height_pixels(_k4aDepthImage), 
		3 * k4a_image_get_width_pixels(_k4aDepthImage) * (int)sizeof(int16_t),
		&alignmentImage) != K4A_RESULT_SUCCEEDED) {

		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(alignmentImage);
		xyzImage = cv::Mat{};
		return;
	}

	if (k4a_transformation_depth_image_to_point_cloud(transformationHandle, _k4aDepthImage, K4A_CALIBRATION_TYPE_DEPTH, alignmentImage) != K4A_WAIT_RESULT_SUCCEEDED) {
		k4a_transformation_destroy(transformationHandle);
		k4a_image_release(alignmentImage);
		xyzImage = cv::Mat{};
		return;
	}

	int width = k4a_image_get_width_pixels(alignmentImage);
	int height = k4a_image_get_height_pixels(alignmentImage);
	int stride = k4a_image_get_stride_bytes(alignmentImage);
	int16_t* pointCloudImageBuffer = (int16_t*)k4a_image_get_buffer(alignmentImage);

	/*for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
		{
			int pixelIndex = h * width + w;
			k4a_float3_t position = {
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 0]),
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 1]),
				static_cast<float>(pointCloudImageBuffer[3 * pixelIndex + 2]) 
			};
			qDebug() << pointCloudImageBuffer[3 * pixelIndex + 0] << " " << pointCloudImageBuffer[3 * pixelIndex + 1] << " " << pointCloudImageBuffer[3 * pixelIndex + 2];
		}
	}*/

	// .clone() is necessary
	xyzImage = cv::Mat(k4a_image_get_height_pixels(alignmentImage), k4a_image_get_width_pixels(alignmentImage), CV_16SC3, k4a_image_get_buffer(alignmentImage), cv::Mat::AUTO_STEP).clone();
	
	k4a_transformation_destroy(transformationHandle);
	k4a_image_release(alignmentImage);

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

	// get picture center point depth
	int midX = cvImage.cols / 2;
	int midY = cvImage.rows / 2;
	uchar midDepth = cvImage.at<uchar>(midY, midX);

	qDebug() << "Mid Point Depth: " << midDepth;
	if (midDepth == 0) {
		// do something else if mid point depth is 0
		qDebug() << "Use closest point";
	}

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

	k4a_calibration_t calibration;
	if (k4a_device_get_calibration(this->device, this->config.depth_mode, this->config.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
		return QVector3D(0, 0, 0);
	}

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

std::deque<k4a_float3_t> KinectEngine::getGyroSampleQueue()
{
	return this->gyroSampleQueue;
}

std::deque<k4a_float3_t> KinectEngine::getAccSampleQueue()
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
			qDebug() << "countOfDepth[" << (i-1)*SIZE_OF_INTERVALS + 1 << " - " << i*SIZE_OF_INTERVALS << "] = " << countOfDepth[i];
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
	float x1, float y1,	float z1, 
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
	float x1, float y1,	float z1, 
	float a, float b, float c,
	float d) {
	d = fabs((a * x1 + b * y1 +
		c * z1 + d));
	float e = sqrt(a * a + b *
		b + c * c);

	return d / e;
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
