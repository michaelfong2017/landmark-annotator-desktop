#include "kinectcamera.h"
#include "twolinesdialog.h"

namespace camera {
	KinectCamera::KinectCamera()
	{
		config_ = new KinectConfig;
	}
	KinectCamera::~KinectCamera()
	{
		qDebug() << endl << "~KinectCamera destructor called" << endl;
	}
	void KinectCamera::run()
	{
		//while (camera_running_)
		//{
		//}
	}

	void KinectCamera::startThread()
	{
		start();
	}

	void KinectCamera::stopThread()
	{
		camera_running_ = false;
	}

	void KinectCamera::open()
	{
		if (!KinectEngine::getInstance().isDeviceOpened()) {
			KinectEngine::getInstance().configDevice();
			bool isSuccess = KinectEngine::getInstance().openDevice();

			if (!isSuccess) {
				TwoLinesDialog dialog;
				dialog.setLine1("Kinect device cannot be opened!");
				dialog.setLine2("Please check it and try again.");
				dialog.exec();
				return;
			}
		}
		camera_running_ = true;
	}

	void KinectCamera::close()
	{
	}

	void KinectCamera::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
	{
		KinectEngine::getInstance().computeNormalizedDepthImage(depthToColorImage, out);
	}

	QVector3D KinectCamera::query3DPoint(int x, int y, cv::Mat depthToColorImage)
	{
		return KinectEngine::getInstance().query3DPoint(x, y, depthToColorImage);
	}

	void KinectCamera::readPointCloudImage(cv::Mat& xyzImage)
	{
		KinectEngine::getInstance().readPointCloudImage(xyzImage);
	}

	void KinectCamera::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage)
	{
		KinectEngine::getInstance().readAllImages(colorImage, depthImage, colorToDepthImage, depthToColorImage);
	}

	void KinectCamera::captureImages()
	{
		KinectEngine::getInstance().captureImages();
	}

	void KinectCamera::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
	{
		KinectEngine::getInstance().readColorAndDepthImages(colorImage, depthImage);
	}

	bool KinectCamera::queueIMUSample()
	{
		return KinectEngine::getInstance().queueIMUSample();
	}

	std::deque<point3D> KinectCamera::getGyroSampleQueue()
	{
		return KinectEngine::getInstance().getGyroSampleQueue();
	}

	std::deque<point3D> KinectCamera::getAccSampleQueue()
	{
		return KinectEngine::getInstance().getAccSampleQueue();
	}

	void KinectCamera::readIntrinsicsFromFile(std::string path)
	{
		KinectEngine::getInstance().readIntrinsicsFromFile(path);
	}
}