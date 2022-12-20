#include "realsenseengine.h"

RealsenseEngine::RealsenseEngine() : QWidget() {
}

void RealsenseEngine::clear()
{
	// TODO
}

bool RealsenseEngine::isDeviceConnected()
{
	// TODO to be reviewed
	rs2::context ctx;
	auto list = ctx.query_devices(); // Get a snapshot of currently connected devices
	int device_count = list.size();

	rs2::device front, back;

	if (device_count == 0)
		return false;
	else if (device_count == 1)
		front = list.front();
	else if (device_count == 2) {
		front = list.front();
		back = list.back();
	}

	return true;
}

bool RealsenseEngine::isDeviceOpened()
{
	// TODO
	if (isDeviceConnected()) {
		return this->deviceOpenedBefore;
	}
	else {
		this->deviceOpenedBefore = false;
		return false;
	}
}

bool RealsenseEngine::openDevice()
{
	p.start(cfg);

	this->deviceOpenedBefore = true;
	return true;
}

void RealsenseEngine::closeDevice()
{
	// TODO
}

void RealsenseEngine::configDevice()
{
	const int w = COLOR_IMAGE_WIDTH;

	const int h = COLOR_IMAGE_HEIGHT;

	cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGRA8, VIDEOWRITER_FPS);

	cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, VIDEOWRITER_FPS);

	cfg.enable_stream(RS2_STREAM_INFRARED, 1, w, h, RS2_FORMAT_Y8, VIDEOWRITER_FPS);

	cfg.enable_stream(RS2_STREAM_INFRARED, 2, w, h, RS2_FORMAT_Y8, VIDEOWRITER_FPS);
}

void RealsenseEngine::captureImages()
{
	rs2ImageLock.lockForWrite();
	rs2::frameset frames = p.wait_for_frames();
	colorFrame = frames.get_color_frame();
	depthFrame = frames.get_depth_frame();
	rs2ImageLock.unlock();
}

bool RealsenseEngine::queueIMUSample()
{
	rs2::frameset frameset = p.wait_for_frames();

	rs2::motion_frame gyro_frame = frameset.first_or_default(RS2_STREAM_GYRO);
	rs2::motion_frame accel_frame = frameset.first_or_default(RS2_STREAM_ACCEL);

	if (!gyro_frame || !accel_frame) {
		return false;
	}

	rs2_vector gyro_sample = gyro_frame.get_motion_data();
	rs2_vector accel_sample = accel_frame.get_motion_data();

	gyroSampleQueue.push_back(point3D{ gyro_sample.x, gyro_sample.y, gyro_sample.z });
	accSampleQueue.push_back(point3D{ accel_sample.x, accel_sample.y, accel_sample.z });

	return true;
}

void RealsenseEngine::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage)
{
	this->rs2ImageLock.lockForRead();
	// Shallow copy
	rs2::frame colorFrame = this->colorFrame;
	rs2::frame depthFrame = this->depthFrame;
	this->rs2ImageLock.unlock();

	readColorImage(colorImage, colorFrame);
	readDepthImage(depthImage, depthFrame);
	readColorToDepthImage(colorToDepthImage, colorFrame);
	readDepthToColorImage(depthToColorImage, depthFrame);
}

void RealsenseEngine::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
{
	this->rs2ImageLock.lockForRead();
	// Shallow copy
	rs2::frame colorFrame = this->colorFrame;
	rs2::frame depthFrame = this->depthFrame;
	this->rs2ImageLock.unlock();

	readColorImage(colorImage, colorFrame);
	readDepthImage(depthImage, depthFrame);
}

void RealsenseEngine::readColorImage(cv::Mat& colorImage, rs2::frame colorFrame)
{
	// Shallow copy
	rs2::frame _colorFrame = this->colorFrame;

	if (colorFrame != NULL) {
		_colorFrame = colorFrame;
	}
	else {
		if (_colorFrame == NULL) {
			colorImage = cv::Mat{};
			return;
		}
	}

	int width = colorFrame.as<rs2::video_frame>().get_width();
	int height = colorFrame.as<rs2::video_frame>().get_height();

	colorImage = cv::Mat(height, width, CV_8UC4, (void*)colorFrame.get_data(), cv::Mat::AUTO_STEP);
}

void RealsenseEngine::readDepthImage(cv::Mat& depthImage, rs2::frame depthFrame)
{
	// Shallow copy
	rs2::frame _depthFrame = this->depthFrame;

	if (depthFrame != NULL) {
		_depthFrame = depthFrame;
	}
	else {
		if (_depthFrame == NULL) {
			depthImage = cv::Mat{};
			return;
		}
	}

	int width = depthFrame.as<rs2::video_frame>().get_width();
	int height = depthFrame.as<rs2::video_frame>().get_height();

	depthImage = cv::Mat(height, width, CV_16U, (void*)depthFrame.get_data(), cv::Mat::AUTO_STEP);
}

// Just return color image since color image and depth image have the same dimentsion
void RealsenseEngine::readColorToDepthImage(cv::Mat& colorToDepthImage, rs2::frame colorFrame)
{
	readColorImage(colorToDepthImage, colorFrame);
}

// Just return depth image since color image and depth image have the same dimentsion
void RealsenseEngine::readDepthToColorImage(cv::Mat& depthToColorImage, rs2::frame depthFrame)
{
	readDepthImage(depthToColorImage, depthFrame);
}

void RealsenseEngine::readPointCloudImage(cv::Mat& xyzImage)
{
	// TODO
	xyzImage = cv::Mat();
}

std::deque<point3D> RealsenseEngine::getGyroSampleQueue()
{
	return gyroSampleQueue;
}

std::deque<point3D> RealsenseEngine::getAccSampleQueue()
{
	return accSampleQueue;
}

QVector3D RealsenseEngine::query3DPoint(int x, int y, cv::Mat depthToColorImage)
{
	// TODO
	return QVector3D(0, 0, 0);
}
