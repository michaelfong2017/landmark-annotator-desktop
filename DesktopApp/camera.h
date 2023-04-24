#ifndef CAMERA_H
#define CAMERA_H

#include "stdafx.h"
#include "types.h"

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
		/** Functions that both realsense and kinect have */
		virtual void computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out) = 0;
		virtual QVector3D query3DPoint(int x, int y, cv::Mat depthToColorImage) = 0;
		virtual void readPointCloudImage(cv::Mat& xyzImage) = 0;
		virtual void readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage) = 0;
		virtual void captureImages() = 0;
		virtual void readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage) = 0;
		virtual bool queueIMUSample() = 0;
		virtual std::deque<point3D> getGyroSampleQueue() = 0;
		virtual std::deque<point3D> getAccSampleQueue() = 0;
		virtual void readIntrinsicsFromFile(std::string path) = 0;
		/** Functions that both realsense and kinect have END */
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