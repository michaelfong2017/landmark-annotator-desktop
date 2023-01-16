#include "cameramanager.h"
#include "kinectcamera.h"
#include "realsensecamera.h"

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
	CameraManager::CameraManager()
	{}

	CameraManager::~CameraManager()
	{}
}
