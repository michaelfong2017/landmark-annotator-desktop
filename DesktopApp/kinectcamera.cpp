#include "kinectcamera.h"

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
		while (camera_running_)
		{
		}
	}

	void KinectCamera::startThread()
	{
		//start();
	}

	void KinectCamera::stopThread()
	{
		//camera_running_ = false;
	}

	void KinectCamera::open()
	{
	}

	void KinectCamera::close()
	{
	}
}