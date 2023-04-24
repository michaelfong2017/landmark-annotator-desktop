#include "realsensecamera.h"
#include "realsenseengine.h"

namespace camera {
	RealsenseCamera::RealsenseCamera()
	{	
		config_ = new RealsenseConfig;
	}
	RealsenseCamera::~RealsenseCamera()
	{
		qDebug() << endl << "~RealsenseCamera destructor called" << endl;
	}
	void RealsenseCamera::run()
	{
		while (camera_running_)
		{
			// Wait for frames and get them as soon as they are ready
			//frames = p.wait_for_frames();

			//// Let's get our depth frame
			//rs2::depth_frame depth = frames.get_depth_frame();
			//// And our rgb frame
			//rs2::frame color = frames.get_color_frame();

			//// Let's convert them to QImage
			//QImage q_color = realsenseFrameToQImage(color);
			//QImage q_depth = realsenseFrameToQImage(depth);

			//// And finally we'll emit our signal
			//emit framesReady(q_color, q_depth);

			// Test
			//rs2::frameset frames = p.wait_for_frames();
			//rs2::depth_frame depth = frames.get_depth_frame();
			//float width = depth.get_width();
			//float height = depth.get_height();
			//float dist_to_center = depth.get_distance(width / 2, height / 2);
			//qDebug() << "The camera is facing an object " << dist_to_center << " meters away \r";
			// Test END

			rs2::frameset frames = p.wait_for_frames();

			rs2ImageLock.lockForWrite();
			//colorFrame = frames.get_color_frame();
			//depthFrame = frames.get_depth_frame();
			queue_frameset.enqueue(frames);
			rs2ImageLock.unlock();

			rs2::motion_frame gyro_frame = frames.first_or_default(RS2_STREAM_GYRO);
			rs2::motion_frame accel_frame = frames.first_or_default(RS2_STREAM_ACCEL);

			if (gyro_frame && accel_frame) {
				gyro_sample = gyro_frame.get_motion_data();
				accel_sample = accel_frame.get_motion_data();
				// TODO find out why imu is never successful (never enter here)
				imuSuccess = true;
			}
			else {
				imuSuccess = false;
			}
		}
	}

	void RealsenseCamera::startThread()
	{
		start();
	}

	void RealsenseCamera::stopThread()
	{
		camera_running_ = false;
	}

	void RealsenseCamera::open()
	{
		cfg.enable_stream(RS2_STREAM_COLOR, config_->color_width, config_->color_height, RS2_FORMAT_BGRA8, config_->fps);
		cfg.enable_stream(RS2_STREAM_DEPTH, config_->depth_width, config_->depth_height, RS2_FORMAT_Z16, config_->fps);
		cfg.enable_stream(RS2_STREAM_INFRARED, 1, config_->color_width, config_->color_height, RS2_FORMAT_Y8, config_->fps);
		cfg.enable_stream(RS2_STREAM_INFRARED, 2, config_->color_width, config_->color_height, RS2_FORMAT_Y8, config_->fps);

		// Start our pipeline
		p.start(cfg);

		queue_frameset = rs2::frame_queue(1);

		intrinsics_color = p.get_active_profile().get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>().get_intrinsics();

		// Running once is enough
		//RealsenseEngine::getInstance().writeIntrinsicsToFile(intrinsics_color);

		camera_running_ = true;
	}

	void RealsenseCamera::close()
	{
	}

	void RealsenseCamera::computeNormalizedDepthImage(const cv::Mat depthToColorImage, cv::Mat& out)
	{
		RealsenseEngine::getInstance().computeNormalizedDepthImage(depthToColorImage, out);
	}

	QVector3D RealsenseCamera::query3DPoint(int x, int y, cv::Mat depthToColorImage)
	{
		return RealsenseEngine::getInstance().query3DPoint(x, y, depthToColorImage);
	}

	void RealsenseCamera::readPointCloudImage(cv::Mat& xyzImage)
	{
		RealsenseEngine::getInstance().readPointCloudImage(xyzImage);
	}

	void RealsenseCamera::readAllImages(cv::Mat& colorImage, cv::Mat& depthImage, cv::Mat& colorToDepthImage, cv::Mat& depthToColorImage)
	{
		RealsenseEngine::getInstance().readAllImages(colorImage, depthImage, colorToDepthImage, depthToColorImage);
	}

	void RealsenseCamera::captureImages()
	{
		RealsenseEngine::getInstance().captureImages();
	}

	void RealsenseCamera::readColorAndDepthImages(cv::Mat& colorImage, cv::Mat& depthImage)
	{
		RealsenseEngine::getInstance().readColorAndDepthImages(colorImage, depthImage);
	}

	bool RealsenseCamera::queueIMUSample()
	{
		return RealsenseEngine::getInstance().queueIMUSample();
	}

	std::deque<point3D> RealsenseCamera::getGyroSampleQueue()
	{
		return RealsenseEngine::getInstance().getGyroSampleQueue();
	}

	std::deque<point3D> RealsenseCamera::getAccSampleQueue()
	{
		return RealsenseEngine::getInstance().getAccSampleQueue();
	}

	QImage realsenseFrameToQImage(const rs2::frame& f)
	{
		auto vf = f.as<rs2::video_frame>();
		const int w = vf.get_width();
		const int h = vf.get_height();

		if (f.get_profile().format() == RS2_FORMAT_RGB8)
		{
			auto r = QImage((uchar*)f.get_data(), w, h, w * 3, QImage::Format_RGB888);
			return r;
		}
		else if (f.get_profile().format() == RS2_FORMAT_Z16)
		{
			// only if you have Qt > 5.13
			auto r = QImage((uchar*)f.get_data(), w, h, w * 2, QImage::Format_Grayscale16);
			return r;
		}

		qCritical() << "qCritical - Frame format is not supported yet!";
	}
}
