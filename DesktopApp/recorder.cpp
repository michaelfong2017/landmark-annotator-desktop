#include "recorder.h"

Recorder::Recorder(DesktopApp* parent) {
	this->isRecording = false;
	this->counter = 0;
	this->parent = parent;
    this->timer = new QTimer;

	QObject::connect(this->timer, &QTimer::timeout, [this]() {
		this->counter += 1;

		if (this->counter == MAX_RECORDING_SECONDS) {
			// Stop recording here
			this->counter = 0;
			this->parent->ui.saveVideoButton->click();
		}
	});
}

bool Recorder::getRecordingStatus() {
	return this->isRecording;
}

void Recorder::prepareRecorder() {
	// Initialize output filename
	QString dateTimeString = Helper::getCurrentDateTimeString();
	QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
	this->colorOutputFilename = visitFolderPath + "/recording_color_" + dateTimeString + ".mp4";
	this->depthOutputFilename = visitFolderPath + "/recording_depth_" + dateTimeString + ".mp4";

	// Initialize opencv VideoWriter
	cv::Size colorSize(
		COLOR_IMAGE_WIDTH,
		COLOR_IMAGE_HEIGHT
	);

	cv::Size depthSize(
		DEPTH_IMAGE_WIDTH,
		DEPTH_IMAGE_HEIGHT
	);

	this->colorVideoWriter = new cv::VideoWriter(
		this->colorOutputFilename.toStdString(),
		cv::VideoWriter::fourcc('H', '2', '6', '4'),
		VIDEOWRITER_FPS,
		colorSize
	);

	this->depthVideoWriter = new cv::VideoWriter(
		this->depthOutputFilename.toStdString(),
		cv::VideoWriter::fourcc('H', '2', '6', '4'),
		VIDEOWRITER_FPS,
		depthSize
	);

	this->isRecording = true;
}

void Recorder::stopRecorder() {
	this->isRecording = false;

	this->timer->stop();
	this->counter = 0;

	this->colorVideoWriter->release();
	this->depthVideoWriter->release();
}

cv::VideoWriter* Recorder::getColorVideoWriter() { return this->colorVideoWriter; }

cv::VideoWriter* Recorder::getDepthVideoWriter() { return this->depthVideoWriter; }

QString Recorder::getColorOutputFilename() { return this->colorOutputFilename; }