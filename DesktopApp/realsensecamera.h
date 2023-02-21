#ifndef REALSENSECAMERA_H
#define REALSENSECAMERA_H

#include "stdafx.h"
#include "camera.h"
#include <librealsense2/rs.hpp>

namespace camera {
	struct RealsenseConfig : Config {
		RealsenseConfig() : Config{
			color_width = 1280,
			color_height = 720,
			depth_width = 1280,
			depth_height = 720,
			fps = 30,
		} {}
	};

	class RealsenseCamera :
		public QThread, public Camera
	{
		Q_OBJECT
	public:
		RealsenseCamera();
		~RealsenseCamera();
	private:
		friend class CameraManager;
		// Realsense configuration structure, it will define streams that need to be opened
		rs2::config cfg;
		// Our pipeline, main object used by realsense to handle streams
		rs2::pipeline pipe;
		// Frames returned by our pipeline, they will be packed in this structure
		rs2::frameset frames;

		// Member function that handles thread iteration
		void run() override;
		void startThread() override;
		void stopThread() override;
		void open() override;
		void close() override;

	signals:
		// A signal sent by our class to notify that there are frames that need to be processed
		void framesReady(QImage frameRGB, QImage frameDepth);
	};

	QImage realsenseFrameToQImage(const rs2::frame& f);
}

#endif
