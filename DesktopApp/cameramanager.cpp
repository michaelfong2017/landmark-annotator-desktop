#include "cameramanager.h"
#include "kinectcamera.h"
#include "realsensecamera.h"
#include "kinectengine.h"
#include "realsenseengine.h"

namespace camera {
	Camera* CameraManager::getCamera()
	{
		return camera_;
	}
	void CameraManager::setCamera(Model model)
	{
		if (model == Model::KINECT) camera_ = new KinectCamera;
		else if (model == Model::REALSENSE) camera_ = new RealsenseCamera;
		else qCritical() << "qCritical - Camera model must be either KINECT or REALSENSE!";
	}
	Config* CameraManager::getConfig()
	{
		return camera_->config_;
	}
	bool CameraManager::isCameraRunning()
	{
		if (camera_ == nullptr) {
			return false;
		}
		return camera_->camera_running_;
	}
	CameraManager::CameraManager()
	{}

	CameraManager::~CameraManager()
	{}

	bool CameraManager::autoSelectAndOpenCamera() {
		bool isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is already running";
			return true;
		}
		else {
			qDebug() << "Camera is not running";
		}

		bool kinectConnected = KinectEngine::getInstance().isDeviceConnected();
		bool realsenseConnected = RealsenseEngine::getInstance().isDeviceConnected();
		if (kinectConnected) {
			qDebug() << "Kinect is connected";
		}
		else {
			qDebug() << "Kinected is not connected";
		}
		if (realsenseConnected) {
			qDebug() << "Realsense is connected";
		}
		else {
			qDebug() << "Realsense is not connected";
		}

		if (kinectConnected) {
			// Select kinect
			qDebug() << "Kinect camera is selected";
			// TODO implement kinectcamera.cpp and select kinect
			camera::CameraManager::getInstance().setCamera(camera::Model::REALSENSE);
			//camera::CameraManager::getInstance().setCamera(camera::Model::KINECT);
			camera::CameraManager::getInstance().getCamera()->open();
			camera::CameraManager::getInstance().getCamera()->startThread();
			qDebug() << "Color width is " << camera::CameraManager::getInstance().getConfig()->color_width;
			qDebug() << "Color height is " << camera::CameraManager::getInstance().getConfig()->color_height;
			qDebug() << "Depth width is " << camera::CameraManager::getInstance().getConfig()->depth_width;
			qDebug() << "Depth height is " << camera::CameraManager::getInstance().getConfig()->depth_height;
			qDebug() << "Fps is " << camera::CameraManager::getInstance().getConfig()->fps;
		}
		else if (realsenseConnected) {
			// Select realsense
			qDebug() << "Realsense camera is selected";
			camera::CameraManager::getInstance().setCamera(camera::Model::REALSENSE);
			camera::CameraManager::getInstance().getCamera()->open();
			camera::CameraManager::getInstance().getCamera()->startThread();
			qDebug() << "Color width is " << camera::CameraManager::getInstance().getConfig()->color_width;
			qDebug() << "Color height is " << camera::CameraManager::getInstance().getConfig()->color_height;
			qDebug() << "Depth width is " << camera::CameraManager::getInstance().getConfig()->depth_width;
			qDebug() << "Depth height is " << camera::CameraManager::getInstance().getConfig()->depth_height;
			qDebug() << "Fps is " << camera::CameraManager::getInstance().getConfig()->fps;
		}

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is opened and running successfully";
			return true;
		}
		else {
			qDebug() << "Camera is not running as expected";
			return false;
		}
	}

	void CameraManager::testCameras() {
		bool kinectConnected = KinectEngine::getInstance().isDeviceConnected();
		bool realsenseConnected = RealsenseEngine::getInstance().isDeviceConnected();
		if (kinectConnected) {
			qDebug() << "Kinect is connected";
		}
		else {
			qDebug() << "Kinected is not connected";
		}
		if (realsenseConnected) {
			qDebug() << "Realsense is connected";
		}
		else {
			qDebug() << "Realsense is not connected";
		}

		bool isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}

		camera::CameraManager::getInstance().setCamera(camera::Model::KINECT);
		camera::CameraManager::getInstance().getCamera()->open();
		camera::CameraManager::getInstance().getCamera()->startThread();
		qDebug() << endl << "width is " << camera::CameraManager::getInstance().getConfig()->color_width << endl;

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}

		camera::CameraManager::getInstance().getCamera()->stopThread();

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}

		camera::CameraManager::getInstance().setCamera(camera::Model::REALSENSE);
		camera::CameraManager::getInstance().getCamera()->open();
		camera::CameraManager::getInstance().getCamera()->startThread();
		qDebug() << endl << "width is " << camera::CameraManager::getInstance().getConfig()->color_width << endl;

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}

		camera::CameraManager::getInstance().getCamera()->stopThread();

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}

		camera::CameraManager::getInstance().setCamera(camera::Model::REALSENSE);
		camera::CameraManager::getInstance().getCamera()->open();
		camera::CameraManager::getInstance().getCamera()->startThread();
		qDebug() << endl << "width is " << camera::CameraManager::getInstance().getConfig()->color_width << endl;

		isCameraRunning = camera::CameraManager::getInstance().isCameraRunning();
		if (isCameraRunning) {
			qDebug() << "Camera is running";
		}
		else {
			qDebug() << "Camera is not running";
		}
	}
}
