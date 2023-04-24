#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <camera.h>

namespace camera {
	enum class Model { KINECT, REALSENSE };
	class CameraManager
	{
	public:
		static CameraManager& getInstance() {
			static CameraManager instance;
			return instance;
		}
		CameraManager(CameraManager const&) = delete;
		void operator=(CameraManager const&) = delete;

		~CameraManager();

		Camera* getCamera();
		Camera* getCameraForIntrinsics();
		void setCamera(Model model);
		void setCameraForIntrinsics(Model model);
		Config* getConfig();
		bool isCameraRunning();
		// Return true if successfully open camera or camera is already running
		bool autoSelectAndOpenCamera();
		void testCameras();

	private:
		CameraManager();
		Camera* camera_;
		Camera* camera_for_intrinsics_;
	};
}

#endif
