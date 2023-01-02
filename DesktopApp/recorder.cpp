#include "recorder.h"
#include "kinectengine.h"
#include "realsenseengine.h"

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
		COLOR_IMAGE_WIDTH_REALSENSE,
		COLOR_IMAGE_HEIGHT_REALSENSE
	);

	cv::Size depthSize(
		DEPTH_IMAGE_WIDTH_REALSENSE,
		DEPTH_IMAGE_HEIGHT_REALSENSE
	);

	/** Handle Chinese name when saving video */
	QString tempVisitFolderPath = Helper::getVisitFolderPath(this->parent->tempVideoSavePath);
	this->tempColorOutputFilename = tempVisitFolderPath + "/recording_color_" + dateTimeString + ".mp4";
	this->tempDepthOutputFilename = tempVisitFolderPath + "/recording_depth_" + dateTimeString + ".mp4";
	/** Handle Chinese name when saving video END */

	this->colorVideoWriter = new cv::VideoWriter(
		tempColorOutputFilename.toStdString(),
		cv::VideoWriter::fourcc('H', '2', '6', '4'),
		VIDEOWRITER_FPS,
		colorSize
	);

	this->depthVideoWriter = new cv::VideoWriter(
		tempDepthOutputFilename.toStdString(),
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

	/** Handle Chinese name when saving video */
	QFile cfile(this->tempColorOutputFilename);
	cfile.rename(this->colorOutputFilename);
	QFile dfile(this->tempDepthOutputFilename);
	dfile.rename(this->depthOutputFilename);
	QDir tempDir(this->parent->tempVideoSavePath);

	qDebug() << "tempDir: " << tempDir;
	/** Handle Chinese name when saving video END */
}

cv::VideoWriter* Recorder::getColorVideoWriter() { return this->colorVideoWriter; }

cv::VideoWriter* Recorder::getDepthVideoWriter() { return this->depthVideoWriter; }

QString Recorder::getColorOutputFilename() { return this->colorOutputFilename; }