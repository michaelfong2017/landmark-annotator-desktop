#include "desktopapp.h"
#include "stdafx.h"
#include "patientdatatab.h"
#include "capturetab.h"
#include "annotatetab.h"
#include "alignmenttab.h"
#include "kinectengine.h"

DesktopApp::DesktopApp(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	KinectEngine::getInstance().configDevice();
	KinectEngine::getInstance().openDevice();

	// Test refactoring
	KinectEngine::getInstance().captureImages();
	KinectEngine::getInstance().captureImages();
	KinectEngine::getInstance().captureImages();
	KinectEngine::getInstance().captureImages();
	KinectEngine::getInstance().captureImages();
	cv::Mat c, d, c2d, d2c;
	KinectEngine::getInstance().readAllImages(c, d, c2d, d2c);
	cv::imwrite("refactor_test_color.png", c);
	cv::imwrite("refactor_test_depth.png", d);
	cv::Mat c2dBGR;
	cvtColor(c2d, c2dBGR, cv::COLOR_BGRA2BGR);
	cv::imwrite("refactor_test_color2depth.png", c2dBGR);
	cv::imwrite("refactor_test_depth2color.png", d2c);
	//KinectEngine::getInstance().closeDevice();
	// Test refactoring END

	this->patientDataTab = new PatientDataTab(this);
	this->captureTab = new CaptureTab(this);
	this->annotateTab = new AnnotateTab(this);
	this->alignmentTab = new AlignmentTab(this);

	// Comment out during development
	//this->patient.setValidity(false);

	if (this->ui.tabWidget->currentIndex() == 1) captureTab->timer->start(0);

	QObject::connect(ui.tabWidget, &QTabWidget::currentChanged, [this]() {
		switch (this->ui.tabWidget->currentIndex()) {
		case 1:
			// current tab is captureTab
			if (!this->patient.getValidity()) //If patient data is not ready
				this->ui.tabWidget->setCurrentIndex(0);
			this->captureTab->timer->start(0);
			break;
		case 2:
			// current tab is annotateTab
			if (!this->patient.getValidity()) //If patient data is not ready
				this->ui.tabWidget->setCurrentIndex(0);

			if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
				this->ui.tabWidget->setCurrentIndex(1);
			this->captureTab->timer->stop();
			break;
		case 3:
			// current tab is alignmentTab
			if (!this->patient.getValidity()) //If patient data is not ready
				this->ui.tabWidget->setCurrentIndex(0);

			if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
				this->ui.tabWidget->setCurrentIndex(1);
		default:
			if (this->captureTab->getRecorder()->getRecordingStatus()) //If capture tab is recording
				this->ui.tabWidget->setCurrentIndex(1);
			this->captureTab->timer->stop();
			break;
		}
		});

}

void DesktopApp::setTextOnGraphicsViews(std::string text) {
	QGraphicsTextItem* graphicsText = new QGraphicsTextItem;
	graphicsText->setPlainText(QString::fromStdString(text));

	QGraphicsScene* scene = new QGraphicsScene;
	scene->addItem(graphicsText);

	ui.graphicsViewVideo4->setScene(scene);
	ui.graphicsViewVideo5->setScene(scene);
	ui.graphicsViewImage->setScene(scene);
	ui.graphicsViewAnnotation->setScene(scene);
}

QImage DesktopApp::getQIRImage() {
	k4a_image_t k4aIRImage = this->irImageQueue.back();

	double min, max;
	cv::Mat matIRImageRaw = cv::Mat(k4a_image_get_height_pixels(k4aIRImage), k4a_image_get_width_pixels(k4aIRImage), CV_16U, k4a_image_get_buffer(k4aIRImage), cv::Mat::AUTO_STEP);

	cv::minMaxIdx(matIRImageRaw, &min, &max);
	cv::Mat matIRImage;
	cv::convertScaleAbs(matIRImageRaw, matIRImage, 255 / max);

	cv::Mat temp;
	cvtColor(matIRImage, temp, cv::COLOR_GRAY2RGB);

	QImage qImage((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	qImage.bits();

	return qImage;
}

