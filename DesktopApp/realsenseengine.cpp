#include "realsenseengine.h"
#include "librealsense2/rsutil.h"
#include "kinectengine.h"
#include "helper.h"

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
	const int w = COLOR_IMAGE_WIDTH_REALSENSE;

	const int h = COLOR_IMAGE_HEIGHT_REALSENSE;

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

	// .clone() is necessary to prevent release in memory before use. Otherwise, later on when this
	// cv image needs to be used (e.g. cvtColor() or clone()), there will be access violation error
	// https://stackoverflow.com/questions/45013214/qt-signal-slot-cvmat-unable-to-read-memory-access-violation
	colorImage = cv::Mat(height, width, CV_8UC4, (void*)colorFrame.get_data(), cv::Mat::AUTO_STEP).clone();
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

	depthImage = cv::Mat(height, width, CV_16U, (void*)depthFrame.get_data(), cv::Mat::AUTO_STEP).clone();
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

// Return 16UC3 point cloud image, the 3 channels correspond to xyz and the positions correspond to uv (texture space)
void RealsenseEngine::readPointCloudImage(cv::Mat& xyzImage)
{
	this->rs2ImageLock.lockForRead();
	// Shallow copy
	rs2::frame colorFrame = this->colorFrame;
	rs2::frame depthFrame = this->depthFrame;
	this->rs2ImageLock.unlock();

	rs2::pointcloud pc;
	rs2::points points = pc.calculate(depthFrame);
	pc.map_to(colorFrame);


	// TODO Comment below, and then comvert point cloud to 16UC3 cv image
	int width = colorFrame.as<rs2::video_frame>().get_width();
	int height = colorFrame.as<rs2::video_frame>().get_height();

	xyzImage = cv::Mat(height, width, CV_8UC4, (void*)colorFrame.get_data(), cv::Mat::AUTO_STEP).clone();
}

std::deque<point3D> RealsenseEngine::getGyroSampleQueue()
{
	return gyroSampleQueue;
}

std::deque<point3D> RealsenseEngine::getAccSampleQueue()
{
	return accSampleQueue;
}

void RealsenseEngine::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
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

	while (iterationCount <= k) {

		inlierCount = 0;

		// First point
		while (true) {
			PointOne[0] = rand() % depthToColorImage.cols;
			PointOne[1] = rand() % depthToColorImage.rows;
			if (PointOne[0] < IgnoreLeftAndRightPixel && PointOne[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointOne[0] > 565 && PointOne[0] < 715) {
				continue;
			}
			QVector3D vector3D_1 = RealsenseEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
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
			if (PointTwo[0] > 565 && PointTwo[0] < 715) {
				continue;
			}
			QVector3D vector3D_2 = RealsenseEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
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
			if (PointThree[0] > 565 && PointThree[0] < 715) {
				continue;
			}
			QVector3D vector3D_3 = RealsenseEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);
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

		QVector3D vector3D_1 = RealsenseEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
		QVector3D vector3D_2 = RealsenseEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
		QVector3D vector3D_3 = RealsenseEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);

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
				if (x > 565 && x < 715) {
					continue;
				}
				if (x < IgnoreLeftAndRightPixel || x > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
					continue;
				}
				QVector3D vector3D = RealsenseEngine::getInstance().query3DPoint(x, y, depthToColorImage);
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

QVector3D RealsenseEngine::query3DPoint(int x, int y, cv::Mat depthToColorImage)
{
	rs2_intrinsics const intrin = p.get_active_profile().get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>().get_intrinsics();
	float point[3];
	float pixel[2]{ static_cast<float>(x), static_cast<float>(y) };
	ushort depth = depthToColorImage.at<ushort>(y, x);
	rs2_deproject_pixel_to_point(point, &intrin, pixel, depth);
	return QVector3D(point[0], point[1], point[2]);
}
