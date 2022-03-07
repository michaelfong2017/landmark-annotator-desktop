#ifndef RECORDER
#define RECORDER

#define MAX_RECORDING_SECONDS 10
#define VIDEOWRITER_FPS 15
#define COLOR_IMAGE_WIDTH 1280
#define COLOR_IMAGE_HEIGHT 720 
#define DEPTH_IMAGE_WIDTH 640
#define DEPTH_IMAGE_HEIGHT 576

#include "stdafx.h"
#include "desktopapp.h"
#include "helper.h"

class Recorder {
private:
	bool isRecording;
	int counter;
	DesktopApp* parent;
	cv::VideoWriter* colorVideoWriter;
	cv::VideoWriter* depthVideoWriter;
	QString colorOutputFilename;
	QString depthOutputFilename;

public:
	QTimer* timer;
	Recorder(DesktopApp*);
	bool getRecordingStatus();
	void prepareRecorder();
	void stopRecorder();
	cv::VideoWriter* getColorVideoWriter();
	cv::VideoWriter* getDepthVideoWriter();
	QString getColorOutputFilename();
};

#endif
