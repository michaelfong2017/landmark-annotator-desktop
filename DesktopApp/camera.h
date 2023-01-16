#ifndef CAMERA_H
#define CAMERA_H

#include "stdafx.h"

namespace camera {
	struct Config {
		int color_width;
		int color_height;
		int depth_width;
		int depth_height;
		int fps;
	};

	class Camera {
	public:
		virtual void startThread() = 0;
		virtual void stopThread() = 0;
		virtual void open() = 0;
		virtual void close() = 0;
	protected:
		Camera() : camera_running_(false),
			config_(nullptr)
		{}
		bool camera_running_;
		friend class CameraManager;
		Config* config_;
	};
}

#endif