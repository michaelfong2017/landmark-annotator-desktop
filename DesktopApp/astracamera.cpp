#include "astracamera.h"
#include "astraengine.h"
#include "twolinesdialog.h"

namespace camera {
	AstraCamera::AstraCamera()
	{
		config_ = new AstraConfig;
	}
	AstraCamera::~AstraCamera()
	{
		qDebug() << endl << "~AstraCamera destructor called" << endl;
	}
	void AstraCamera::run()
	{
		while (camera_running_)
		{
			std::shared_ptr<ob::FrameSet> frameSet = p.waitForFrames(100);

			frameSetLock.lockForWrite();
			// Ensure that the size of the queue is at most 1
			if (!frameSetQueue.empty()) {
				frameSetQueue.pop();
			}
			frameSetQueue.push(std::move(frameSet));
			frameSetLock.unlock();
		}
	}

	void AstraCamera::startThread()
	{
		start();
	}

	void AstraCamera::stopThread()
	{
		camera_running_ = false;
	}

	void AstraCamera::open()
	{
		auto colorProfiles = p.getStreamProfileList(OB_SENSOR_COLOR);
		auto colorProfile = colorProfiles->getVideoStreamProfile(config_->color_width, config_->color_height, OB_FORMAT_MJPG, config_->fps);
		auto depthProfiles = p.getStreamProfileList(OB_SENSOR_DEPTH);
		auto depthProfile = depthProfiles->getVideoStreamProfile(config_->depth_width, config_->depth_height, OB_FORMAT_Y16, config_->fps);
		auto irProfiles = p.getStreamProfileList(OB_SENSOR_IR);
		auto irProfile = irProfiles->getVideoStreamProfile(config_->depth_width, config_->depth_height, OB_FORMAT_Y16, config_->fps);

		qDebug() << "Chosen color width: " << colorProfile->width();
		qDebug() << "Chosen color height: " << colorProfile->height();
		qDebug() << "Chosen depth width: " << depthProfile->width();
		qDebug() << "Chosen depth height: " << depthProfile->height();

		cfg = std::make_shared< ob::Config >();
		cfg->enableStream(colorProfile);
		cfg->enableStream(depthProfile);
		cfg->enableStream(irProfile);

		cfg->setAlignMode(ALIGN_D2C_SW_MODE);
		
		// Start our pipeline
		p.start(cfg);

		//queue_frameset = rs2::frame_queue(1);

		//intrinsics_color = p.get_active_profile().get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>().get_intrinsics();

		// Running once is enough
		//RealsenseEngine::getInstance().writeIntrinsicsToFile(intrinsics_color);

		camera_running_ = true;
	}

	void AstraCamera::close()
	{
		// TODO
	}

	void AstraCamera::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
	{
		AstraEngine::getInstance().computeNormalizedDepthImage(depthToColorImage, out);
	}

	QVector3D AstraCamera::query3DPoint(int x, int y, cv::Mat depthToColorImage)
	{
		return AstraEngine::getInstance().query3DPoint(x, y, depthToColorImage);
	}

	void AstraCamera::readPointCloudImage(cv::Mat& xyzImage)
	{
		AstraEngine::getInstance().readPointCloudImage(xyzImage);
	}

	void AstraCamera::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage)
	{
		AstraEngine::getInstance().readAllImages(colorImage, depthImage, colorToDepthImage, depthToColorImage);
	}

	void AstraCamera::captureImages()
	{
		AstraEngine::getInstance().captureImages();
	}

	void AstraCamera::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
	{
		AstraEngine::getInstance().readColorAndDepthImages(colorImage, depthImage);
	}

	bool AstraCamera::queueIMUSample()
	{
		return AstraEngine::getInstance().queueIMUSample();
	}

	std::deque<point3D> AstraCamera::getGyroSampleQueue()
	{
		return AstraEngine::getInstance().getGyroSampleQueue();
	}

	std::deque<point3D> AstraCamera::getAccSampleQueue()
	{
		return AstraEngine::getInstance().getAccSampleQueue();
	}

	void AstraCamera::readIntrinsicsFromFile(std::string path)
	{
		AstraEngine::getInstance().readIntrinsicsFromFile(path);
	}
}