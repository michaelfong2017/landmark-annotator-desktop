#ifndef KINECTCAMERA_H
#define KINECTCAMERA_H

#include "stdafx.h"
#include "camera.h"
#include <k4a/k4a.hpp>

namespace camera {
	struct KinectConfig : Config {
		KinectConfig() : Config{
			color_width = 1920,
			color_height = 1080,
			depth_width = 640,
			depth_height = 576,
			fps = 30,
		} {}
	};

	class KinectCamera :
		public QThread, public Camera
	{
	private:
		friend class CameraManager;
		KinectCamera();
		~KinectCamera();

		// Member function that handles thread iteration
		void run() override;
		void startThread() override;
		void stopThread() override;
		void open() override;
		void close() override;

	signals:
		// A signal sent by our class to notify that there are frames that need to be processed
		void framesReady(const QImage& frameRGB, const QImage& frameDepth);
	};
}

#endif
