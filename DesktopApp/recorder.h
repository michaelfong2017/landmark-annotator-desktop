#ifndef RECORDER
#define RECORDER

#define MAX_RECORDING_SECONDS 10

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
	QString tempColorOutputFilename;
	QString tempDepthOutputFilename;

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
