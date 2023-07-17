#ifndef ASTRACAMERA_H
#define ASTRACAMERA_H

#include "stdafx.h"
#include "camera.h"
#include <libobsensor/ObSensor.hpp>
#include "libobsensor/hpp/Pipeline.hpp"
#include "libobsensor/hpp/Error.hpp"

namespace camera {
	struct AstraConfig : Config {
		AstraConfig() : Config{
			color_width = 1920,
			color_height = 1080,
			depth_width = 640,
			depth_height = 480,
			fps = 30,
		} {}
	};

	class AstraCamera :
		public QThread, public Camera
	{
		Q_OBJECT
	public:
		AstraCamera();
		~AstraCamera();

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
		void readIntrinsicsFromFile(std::string path) override;
		/** Functions that both realsense and kinect have END */

		/** Start using the frames below */
		std::queue<std::shared_ptr<ob::FrameSet>> frameSetQueue;
		QReadWriteLock frameSetLock;

		bool imuSuccess = false;

		//rs2_intrinsics intrinsics_color;

	private:
		friend class CameraManager;
		std::shared_ptr< ob::Config > cfg;
		ob::Pipeline p;

		// Member function that handles thread iteration
		void run() override;
		void startThread() override;
		void stopThread() override;
		void open() override;
		void close() override;

		/** Start using the frames below */
	};

	// TODO [astra] astra-specific functions (if applicable)
	//QImage realsenseFrameToQImage(const rs2::frame& f);
}

#endif
