#include "realsensecamera.h"

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
			//frames = pipe.wait_for_frames();

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
			rs2::frameset frames = pipe.wait_for_frames();
			rs2::depth_frame depth = frames.get_depth_frame();
			float width = depth.get_width();
			float height = depth.get_height();
			float dist_to_center = depth.get_distance(width / 2, height / 2);
			qDebug() << "The camera is facing an object " << dist_to_center << " meters away \r";
			// Test END
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
		// Enable depth stream with given resolution. Pixel will have a bit depth of 16 bit
		cfg.enable_stream(RS2_STREAM_DEPTH, config_->depth_width, config_->depth_height, RS2_FORMAT_Z16, config_->fps);

		// Enable RGB stream as frames with 3 channel of 8 bit
		cfg.enable_stream(RS2_STREAM_COLOR, config_->color_width, config_->color_height, RS2_FORMAT_RGB8, config_->fps);

		// Start our pipeline
		pipe.start(cfg);

		camera_running_ = true;
	}

	void RealsenseCamera::close()
	{
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
