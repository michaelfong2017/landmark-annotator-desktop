#include "desktopapp.h"
#include "stdafx.h"
#include "logintab.h"
#include "patientlisttab.h"
#include "patienttab.h"
#include "capturetab.h"
#include "annotatetab.h"
#include "alignmenttab.h"
#include "kinectengine.h"

DesktopApp::DesktopApp(QWidget* parent)
	: QWidget(parent)
{

	//this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
	ui.setupUi(this);

	KinectEngine::getInstance().configDevice();
	bool isOpen = KinectEngine::getInstance().openDevice();
	while (!isOpen) {
		TwoLinesDialog dialog;
		dialog.setLine1("Kinect device cannot be opened!");
		dialog.setLine2("Please check it and try again.");
		dialog.exec();
		KinectEngine::getInstance().closeDevice();
		isOpen = KinectEngine::getInstance().openDevice();
	}

	// Test refactoring
	// 
	//KinectEngine::getInstance().captureImages();
	//KinectEngine::getInstance().captureImages();
	//KinectEngine::getInstance().captureImages();
	//KinectEngine::getInstance().captureImages();
	//KinectEngine::getInstance().captureImages();
	//cv::Mat c, d, c2d, d2c;
	//KinectEngine::getInstance().readAllImages(c, d, c2d, d2c);
	//cv::imwrite("refactor_test_color.png", c);
	//cv::imwrite("refactor_test_depth.png", d);
	//cv::Mat c2dBGR;
	//cvtColor(c2d, c2dBGR, cv::COLOR_BGRA2BGR);
	//cv::imwrite("refactor_test_color2depth.png", c2dBGR);
	//cv::imwrite("refactor_test_depth2color.png", d2c);
	// 
	//KinectEngine::getInstance().closeDevice();
	// Test refactoring END
	
	this->loginTab = new LoginTab(this);
	this->patientListTab = new PatientListTab(this);
	this->patientTab = new PatientTab(this);
	this->captureTab = new CaptureTab(this);
	this->annotateTab = new AnnotateTab(this);
	this->alignmentTab = new AlignmentTab(this);

	if (this->ui.tabWidget->currentIndex() == 2) captureTab->timer->start(0);

	/** Must login to begin */
	this->ui.tabWidget->setTabEnabled(1, false);
	this->ui.tabWidget->setTabEnabled(2, false);
	this->ui.tabWidget->setTabEnabled(3, false);
	this->ui.tabWidget->setTabEnabled(4, false);
	/** Must login to begin END */

	/** Hide unused Alignment tab */
	this->ui.tabWidget->setTabEnabled(5, false);
	this->ui.tabWidget->setTabVisible(5, false);
	/** Hide unused Alignment tab END */
	/** Hide unused bottom buttons in Analysis tab */
	//this->ui.horizontalLayout_3->setEnabled(false);
	this->ui.reportButton->setVisible(false);
	this->ui.annotateButtonAnnotateTab->setVisible(false);
	this->ui.saveButtonAnnotateTab->setVisible(false);
	/** Hide unused bottom buttons in Analysis tab END */

	QObject::connect(ui.tabWidget, &QTabWidget::currentChanged, [this]() {
		switch (this->ui.tabWidget->currentIndex()) {
		case 0:
			// current tab is loginTab	
			break;
		case 1:
			// current tab is patientListTab

			/** Re-enable the tabs after login */
			this->ui.tabWidget->setTabEnabled(1, true);
			this->ui.tabWidget->setTabEnabled(2, true);
			this->ui.tabWidget->setTabEnabled(3, true);
			this->ui.tabWidget->setTabEnabled(4, true);
			/** Re-enable the tabs after login END */

			this->patientListTab->onEnterTab();
			break;
		case 2:
			// current tab is patientTab
			this->patientTab->onEnterTab();
			break;
		case 3:
			// current tab is captureTab
			this->captureTab->timer->start(0);
			break;
		case 4:
			// current tab is annotateTab
			this->captureTab->timer->stop();
			break;
		case 5:
			// current tab is alignmentTab
			break;
		default:
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

