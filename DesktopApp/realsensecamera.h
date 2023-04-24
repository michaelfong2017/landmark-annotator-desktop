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

		/** Functions that both realsense and kinect have */
		void computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out) override;
		QVector3D query3DPoint(int x, int y, cv::Mat depthToColorImage) override;
		void readPointCloudImage(cv::Mat& xyzImage) override;
		void readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage) override;
		void captureImages() override;
		void readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage) override;
		bool queueIMUSample() override;
		std::deque<point3D> getGyroSampleQueue() override;
		std::deque<point3D> getAccSampleQueue() override;
		/** Functions that both realsense and kinect have END */

		/** Start using the frames below */
		QReadWriteLock rs2ImageLock;
		rs2::frame_queue queue_frameset;
		rs2_vector gyro_sample;
		rs2_vector accel_sample;
		bool imuSuccess = false;

		rs2_intrinsics intrinsics_color;

	private:
		friend class CameraManager;
		// Realsense configuration structure, it will define streams that need to be opened
		rs2::config cfg;
		// Our pipeline, main object used by realsense to handle streams
		rs2::pipeline p;
		// Frames returned by our pipeline, they will be packed in this structure
		rs2::frameset frames;

		// Member function that handles thread iteration
		void run() override;
		void startThread() override;
		void stopThread() override;
		void open() override;
		void close() override;

		/** Start using the frames below */
	};

	QImage realsenseFrameToQImage(const rs2::frame& f);
}

#endif
