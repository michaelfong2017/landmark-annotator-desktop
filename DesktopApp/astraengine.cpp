#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "cameramanager.h"
#include "astracamera.h"
#include "astraengine.h"
#include "kinectengine.h"

AstraEngine::AstraEngine() : QWidget() {
}

void AstraEngine::clear()
{
	// TODO
}

bool AstraEngine::isDeviceConnected()
{
	qDebug() << "SDK version: " << ob::Version::getMajor() << "." << ob::Version::getMinor() << "." << ob::Version::getPatch();
	ob::Context ctx;
	auto devList = ctx.queryDeviceList();
	int device_count = devList->deviceCount();
	qDebug() << "Device count: " << device_count;
	if (device_count <= 0) {
		return false;
	}
	
	auto dev = devList->getDevice(0);

	{
		// Print device information
		auto devInfo = dev->getDeviceInfo();
		//Get the device name
		qDebug() << "Device name: " << devInfo->name();
		//Get the pid, vid, uid of the device
		qDebug() << "Device pid: " << devInfo->pid() << " vid: " << devInfo->vid() << " uid: " << devInfo->uid();
		//Get the fireware version number of the device 
		auto fwVer = devInfo->firmwareVersion();
		qDebug() << "Firmware version: " << fwVer;
		//Get the serial number of the device
		auto sn = devInfo->serialNumber();
		qDebug() << "Serial number: " << sn;
		// Print device information END
	}

	// Get a list of supported sensors
	qDebug() << "Sensor types: ";
	auto sensorList = dev->getSensorList();
	for (uint32_t i = 0; i < sensorList->count(); i++) {
		auto sensor = sensorList->getSensor(i);
		switch (sensor->type()) {
		case OB_SENSOR_COLOR:
			qDebug() << "\tColor sensor";
			break;
		case OB_SENSOR_DEPTH:
			qDebug() << "\tDepth sensor";
			break;
		case OB_SENSOR_IR:
			qDebug() << "\tIR sensor";
			break;
		case OB_SENSOR_GYRO:
			qDebug() << "\tGyro sensor";
			break;
		case OB_SENSOR_ACCEL:
			qDebug() << "\tAccel sensor";
			break;
		default:
			break;
		}
	}

	return true;
}

bool AstraEngine::isDeviceOpened()
{
	if (isDeviceConnected()) {
		return this->deviceOpenedBefore;
	}
	else {
		this->deviceOpenedBefore = false;
		return false;
	}

	return true;
}

// Legacy and useless although still being referenced
bool AstraEngine::openDevice()
{
	return true;
}

// Legacy and useless although still being referenced
void AstraEngine::closeDevice()
{
	// TODO
}

// Legacy and useless although still being referenced
void AstraEngine::configDevice()
{
}

void AstraEngine::captureImages()
{
	camera::AstraCamera* camera = static_cast<camera::AstraCamera*>(camera::CameraManager::getInstance().getCamera());

	camera->frameSetLock.lockForRead();
	if (!camera->frameSetQueue.empty()) {
		framesLock.lockForWrite();
		frames = std::move(camera->frameSetQueue.front());
		framesLock.unlock();
		camera->frameSetQueue.pop();
	}
	camera->frameSetLock.unlock();
	qDebug() << "frames: " << frames.get();
}

bool AstraEngine::queueIMUSample()
{
	//rs2::frameset frameset = p.wait_for_frames();

	//rs2::motion_frame gyro_frame = frameset.first_or_default(RS2_STREAM_GYRO);
	//rs2::motion_frame accel_frame = frameset.first_or_default(RS2_STREAM_ACCEL);

	//camera::RealsenseCamera* camera = static_cast<camera::RealsenseCamera*>(camera::CameraManager::getInstance().getCamera());

	//rs2_vector gyro_sample = camera->gyro_sample;
	//rs2_vector accel_sample = camera->accel_sample;

	//if (!camera->imuSuccess) {
	//	return false;
	//}

	////if (!gyro_frame || !accel_frame) {
	////	return false;
	////}

	////rs2_vector gyro_sample = gyro_frame.get_motion_data();
	////rs2_vector accel_sample = accel_frame.get_motion_data();

	//gyroSampleQueue.push_back(point3D{ gyro_sample.x, gyro_sample.y, gyro_sample.z });
	//accSampleQueue.push_back(point3D{ accel_sample.x, accel_sample.y, accel_sample.z });

	return true;
}

void AstraEngine::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage)
{
	if (frames == NULL) {
		colorImage = cv::Mat{};
		depthImage = cv::Mat{};
		colorToDepthImage = cv::Mat{};
		depthToColorImage = cv::Mat{};
		return;
	}

	framesLock.lockForRead();

	auto colorFrame = frames->colorFrame();
	auto depthFrame = frames->depthFrame();

	// colorToDepth image is not used but still obtained here
	//rs2::align align_to_depth(RS2_STREAM_DEPTH);
	//rs2::frameset aligned_to_depth_frames = align_to_depth.process(this->frames);
	//rs2::frame colorToDepthFrame = aligned_to_depth_frames.get_color_frame();

	//rs2::align align_to_color(RS2_STREAM_COLOR);
	//rs2::frameset aligned_to_color_frames = align_to_color.process(this->frames);
	//rs2::frame depthToColorFrame = aligned_to_color_frames.get_depth_frame();

	framesLock.unlock();

	readColorImage(colorImage, colorFrame);
	readDepthImage(depthImage, depthFrame);
	//readColorToDepthImage(colorToDepthImage, colorToDepthFrame);
	//readDepthToColorImage(depthToColorImage, depthToColorFrame);
}

void AstraEngine::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
{
	if (frames == NULL) {
		colorImage = cv::Mat{};
		depthImage = cv::Mat{};
		return;
	}

	framesLock.lockForRead();

	auto colorFrame = frames->colorFrame();
	auto depthFrame = frames->depthFrame();

	framesLock.unlock();

	readColorImage(colorImage, colorFrame);
	readDepthImage(depthImage, depthFrame);
}

void AstraEngine::readColorImage(cv::Mat& colorImage, std::shared_ptr<ob::ColorFrame> colorFrame)
{
	if (colorFrame == NULL) {
		colorImage = cv::Mat{};
		return;
	}

	//Create Format Conversion Filter
	ob::FormatConvertFilter formatConverFilter;
	if (colorFrame->format() == OB_FORMAT_MJPG) {
		formatConverFilter.setFormatConvertType(FORMAT_MJPG_TO_BGRA);
	}
	else {
		qWarning() << "Only color format OB_FORMAT_MJPG is supported!";
		colorImage = cv::Mat{};
		return;
	}
	colorFrame = formatConverFilter.process(colorFrame)->as<ob::ColorFrame>();

	int width = colorFrame->width();
	int height = colorFrame->height();

	//// .clone() is necessary to prevent release in memory before use. Otherwise, later on when this
	//// cv image needs to be used (e.g. cvtColor() or clone()), there will be access violation error
	//// https://stackoverflow.com/questions/45013214/qt-signal-slot-cvmat-unable-to-read-memory-access-violation
	colorImage = cv::Mat(height, width, CV_8UC4, (void*)colorFrame->data(), cv::Mat::AUTO_STEP).clone();
}

void AstraEngine::readDepthImage(cv::Mat& depthImage, std::shared_ptr<ob::DepthFrame> depthFrame)
{
	if (depthFrame == NULL) {
		depthImage = cv::Mat{};
		return;
	}

	int width = depthFrame->width();
	int height = depthFrame->height();

	depthImage = cv::Mat(height, width, CV_16U, (void*)depthFrame->data(), cv::Mat::AUTO_STEP).clone();
}

// Just return color image since color image and depth image have the same dimentsion
void AstraEngine::readColorToDepthImage(cv::Mat& colorToDepthImage, std::shared_ptr<ob::ColorFrame> colorFrame)
{
	readColorImage(colorToDepthImage, colorFrame);
}

// Just return depth image since color image and depth image have the same dimentsion
void AstraEngine::readDepthToColorImage(cv::Mat& depthToColorImage, std::shared_ptr<ob::DepthFrame> depthFrame)
{
	readDepthImage(depthToColorImage, depthFrame);
}

// Return 16UC3 point cloud image, the 3 channels correspond to xyz and the positions correspond to uv (texture space)
void AstraEngine::readPointCloudImage(cv::Mat& xyzImage)
{
	//this->rs2ImageLock.lockForRead();
	//// Shallow copy
	//rs2::frame colorFrame = this->frames.get_color_frame();
	//rs2::frame depthFrame = this->frames.get_depth_frame();
	//this->rs2ImageLock.unlock();

	//rs2::pointcloud pc;
	//rs2::points points = pc.calculate(depthFrame);
	//pc.map_to(colorFrame);


	//int width = colorFrame.as<rs2::video_frame>().get_width();
	//int height = colorFrame.as<rs2::video_frame>().get_height();
	//xyzImage = cv::Mat(height, width, CV_16UC3);

	//auto vertices = points.get_vertices();
	//for (int y = 0; y < xyzImage.rows; y++)
	//{
	//	for (int x = 0; x < xyzImage.cols; x++)
	//	{
	//		cv::Vec3s& color = xyzImage.at<cv::Vec3s>(y, x);

	//		color[0] = static_cast<short>(vertices[y * width + x].x);
	//		color[1] = static_cast<short>(vertices[y * width + x].y);
	//		color[2] = static_cast<short>(vertices[y * width + x].z);

	//		// set pixel
	//		xyzImage.at<cv::Vec3s>(y, x) = color;
	//	}
	//}
}

std::deque<point3D> AstraEngine::getGyroSampleQueue()
{
	return gyroSampleQueue;
}

std::deque<point3D> AstraEngine::getAccSampleQueue()
{
	return accSampleQueue;
}

void AstraEngine::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
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
			QVector3D vector3D_1 = AstraEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
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
			QVector3D vector3D_2 = AstraEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
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
			QVector3D vector3D_3 = AstraEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);
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

		QVector3D vector3D_1 = AstraEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
		QVector3D vector3D_2 = AstraEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
		QVector3D vector3D_3 = AstraEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);

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
				QVector3D vector3D = AstraEngine::getInstance().query3DPoint(x, y, depthToColorImage);
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

			QVector3D vector3D = AstraEngine::getInstance().query3DPoint(x, y, depthToColorImage);

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

QVector3D AstraEngine::query3DPoint(int x, int y, cv::Mat depthToColorImage)
{
	//camera::RealsenseCamera* camera = static_cast<camera::RealsenseCamera*>(camera::CameraManager::getInstance().getCamera());
	//rs2_intrinsics intrin = camera->intrinsics_color;
	//float point[3];
	//float pixel[2]{ static_cast<float>(x), static_cast<float>(y) };
	//ushort depth = depthToColorImage.at<ushort>(y, x);

	//rs2_deproject_pixel_to_point(point, &this->intrin, pixel, depth);
	//// 52685 means no depth value in the depth image
	//return QVector3D(point[0], point[1], point[2] == 52685 ? 0 : point[2]);

	return QVector3D();
}

void AstraEngine::writeIntrinsicsToFile(rs2_intrinsics& intrin)
{
	//std::ofstream fw("intrinsics_realsense_color.txt", std::ofstream::out);
	//if (fw.is_open())
	//{
	//	fw << "width: " << intrin.width << std::endl;
	//	fw << "height: " << intrin.height << std::endl;
	//	fw << "model: " << intrin.model << std::endl;
	//	if (intrin.model == RS2_DISTORTION_BROWN_CONRADY || intrin.model == RS2_DISTORTION_INVERSE_BROWN_CONRADY) {
	//		fw << "[k1, k2, p1, p2, k3]: " << "[" << intrin.coeffs[0] << ", " << intrin.coeffs[1] << ", " << intrin.coeffs[2] << ", " << intrin.coeffs[3] << ", " << intrin.coeffs[4] << "]" << std::endl;
	//	}
	//	else if (intrin.model == RS2_DISTORTION_FTHETA) {
	//		fw << "[k1, k2, k3, k4, 0]: " << "[" << intrin.coeffs[0] << ", " << intrin.coeffs[1] << ", " << intrin.coeffs[2] << ", " << intrin.coeffs[3] << ", " << intrin.coeffs[4] << "]" << std::endl;
	//	}
	//	fw << "fx: " << intrin.fx << std::endl;
	//	fw << "fy: " << intrin.fy << std::endl;
	//	fw << "ppx: " << intrin.ppx << std::endl;
	//	fw << "ppy: " << intrin.ppy << std::endl;
	//	fw.close();
	//}
}

void AstraEngine::readIntrinsicsFromFile(std::string path)
{
	//std::ifstream f(path);
	//if (f.is_open()) {
	//	std::string line;
	//	while (std::getline(f, line)) {
	//		size_t colon = line.find(":");
	//		std::string key = line.substr(0, colon);
	//		std::string value = line.substr(colon + 2, line.size());
	//		if (key == "width") {
	//			intrin.width = std::stoi(value);
	//		}
	//		else if (key == "height") {
	//			intrin.height = std::stoi(value);
	//		}
	//		else if (key == "model") {
	//			if (value == "Brown Conrady") {
	//				intrin.model = RS2_DISTORTION_BROWN_CONRADY;
	//			}
	//			else if (value == "Inverse Brown Conrady") {
	//				intrin.model = RS2_DISTORTION_INVERSE_BROWN_CONRADY;
	//			}
	//			// TODO other model type
	//		}
	//		// RS2_DISTORTION_BROWN_CONRADY
	//		else if (key == "[k1, k2, p1, p2, k3]") {
	//			// TODO read from value
	//			// currently hardcode
	//			intrin.coeffs[0] = 0.0f;
	//			intrin.coeffs[1] = 0.0f;
	//			intrin.coeffs[2] = 0.0f;
	//			intrin.coeffs[3] = 0.0f;
	//			intrin.coeffs[4] = 0.0f;
	//		}
	//		// TODO other model type
	//		else if (key == "fx") {
	//			intrin.fx = std::stof(value);
	//		}
	//		else if (key == "fy") {
	//			intrin.fy = std::stof(value);
	//		}
	//		else if (key == "ppx") {
	//			intrin.ppx = std::stof(value);
	//		}
	//		else if (key == "ppy") {
	//			intrin.ppy = std::stof(value);
	//		}
	//	}
	//	f.close();
	//}
}