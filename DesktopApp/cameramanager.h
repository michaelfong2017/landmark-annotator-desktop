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

		Camera* getCamera();
		void setCamera(Model model);
		Config* getConfig();

	private:
		CameraManager();
		~CameraManager();
		Camera* camera_ = nullptr;
	};
}

#endif
