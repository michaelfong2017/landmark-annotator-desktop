#ifndef KINECTCAMERA_H
#define KINECTCAMERA_H

#include "stdafx.h"
#include "camera.h"
#include <k4a/k4a.hpp>
#include "kinectengine.h"
#include "realsenseengine.h"

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
	public:
		KinectCamera();
		~KinectCamera();
	private:
		friend class CameraManager;

		// Member function that handles thread iteration
		void run() override;
		void startThread() override;
		void stopThread() override;
		void open() override;
		void close() override;

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

	signals:
		// A signal sent by our class to notify that there are frames that need to be processed
		void framesReady(const QImage& frameRGB, const QImage& frameDepth);
	};
}

#endif
