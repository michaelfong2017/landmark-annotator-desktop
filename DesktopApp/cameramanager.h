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
		void setCamera(Model model);
		Config* getConfig();
		bool isCameraRunning();
		// Return true if successfully open camera or camera is already running
		bool autoSelectAndOpenCamera();
		void testCameras();

	private:
		CameraManager();
		Camera* camera_;
	};
}

#endif
