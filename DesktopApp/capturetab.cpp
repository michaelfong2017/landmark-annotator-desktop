#include "capturetab.h"
#include "saveimagedialog.h"
#include "devicemovingdialog.h"

CaptureTab::CaptureTab(DesktopApp* parent)
{
	this->parent = parent;
	this->recorder = new Recorder(parent);
	this->parent->ui.recordingIndicatorText->setVisible(false);
	this->parent->ui.recordingElapsedTime->setVisible(false);

	this->setDefaultCaptureMode();

	this->registerRadioButtonOnClicked(this->parent->ui.radioButton, &this->colorImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton2, &this->depthImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton3, &this->colorToDepthImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton4, &this->depthToColorImage);

	/** QNetworkAccessManager */
	connect(&manager, &QNetworkAccessManager::finished, this, &CaptureTab::onManagerFinished);
	/***/

	this->captureCount = 0;
	this->noImageCaptured = true;
	this->timer = new QTimer;

	this->parent->ui.showInExplorer->hide();
	this->captureFilepath = QString();

	QObject::connect(this->parent->ui.saveButtonCaptureTab, &QPushButton::clicked, [this]() {
		SaveImageDialog dialog(this);
		dialog.exec();
		});

	QObject::connect(this->parent->ui.saveVideoButton, &QPushButton::clicked, [this]() {
		if (this->recorder->getRecordingStatus()) {
			// Current status is recording
			QString dateTimeString = Helper::getCurrentDateTimeString();
			QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);

			// Modify UI to disable recording status
			this->parent->ui.recordingIndicatorText->setVisible(false);
			this->parent->ui.recordingElapsedTime->setVisible(false);
			this->parent->ui.captureTab->setStyleSheet("");

			this->recorder->stopRecorder();
			this->parent->ui.saveVideoButton->setText("start recording");

			this->parent->ui.saveInfoCaptureTab->setText("Recording is saved under\n" + visitFolderPath + "\nat " + dateTimeString);

			this->parent->ui.showInExplorer->show();
			this->setCaptureFilepath(this->recorder->getColorOutputFilename());

			// Enable analysis button
			if (!this->noImageCaptured) {
				this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
			}

			// Recording gif
			QLabel* recordingGif = this->parent->ui.recordingGif;
			QMovie* movie = recordingGif->movie();
			if (movie != nullptr) {
				movie->stop();
				delete movie;
			}
			// Recording gif END
			// Recording time elapsed
			this->recordingElapsedTimer.invalidate();
			// Recording time elapsed END
		}
		else {
			// Current status is NOT recording

			// Modify UI to indicate recording status
			this->parent->ui.recordingIndicatorText->setVisible(true);
			this->parent->ui.recordingElapsedTime->setVisible(true);
			this->parent->ui.captureTab->setStyleSheet("#captureTab {border: 2px solid red}");

			this->recorder->prepareRecorder();
			this->parent->ui.saveVideoButton->setText("stop recording");

			// Disable analysis button
			this->parent->ui.annotateButtonCaptureTab->setEnabled(false);

			// Recording gif
			QMovie* movie = new QMovie(":/DesktopApp/resources/recording.gif");
			movie->setScaledSize(QSize(20, 20));
			QLabel* recordingGif = this->parent->ui.recordingGif;
			recordingGif->setMovie(movie);
			movie->start();
			// Recording gif END

			// Recording time elapsed
			this->recordingElapsedTimer.start();
			// Recording time elapsed END

			this->recorder->timer->start(1000);
		}
		});

	/** Michael Fong Show In Explorer
	BEGIN */
	QObject::connect(this->parent->ui.showInExplorer, &QPushButton::clicked, [this]() {
		QString filepath = this->getCaptureFilepath();

		QStringList args;

		args << "/select," << QDir::toNativeSeparators(filepath);

		QProcess* process = new QProcess(this);
		process->startDetached("explorer.exe", args);
		});
	/** Michael Fong Show In Explorer
	END */

	QObject::connect(this->parent->ui.captureButton, &QPushButton::clicked, [this]() {

		//this->colorImage = this->parent->getQColorImage();
		//this->depthImage = this->parent->getQDepthImage();
		this->cvDepthImage = this->parent->getCVDepthImage();
		//this->colorToDepthImage = this->parent->getQColorToDepthImage();
		//this->depthToColorImage = this->parent->getQDepthToColorImage();
		this->cvDepthToColorImage = this->parent->getCVDepthToColorImage();

		cv::Mat temp2;
		colorizeDepth(this->cvDepthToColorImage, temp2);
		QImage qImage2((const uchar*)temp2.data, temp2.cols, temp2.rows, temp2.step, QImage::Format_RGB888);
		qImage2.bits();
		this->depthToColorImageColorized = qImage2;

		this->CapturedRawColorImage = this->parent->getRawColorImage().clone();
		this->CapturedRawDepthImage = this->parent->getRawDepthImage().clone();
		this->CapturedRawColorToDepthImage = this->parent->getRawColorToDepthImage().clone();
		this->CapturedRawDepthToColorImage = this->parent->getRawDepthToColorImage().clone();

		// Transform the 4 Captured cv::Mat images to QImage
		cv::Mat tempCV1 = this->CapturedRawColorImage;
		cvtColor(tempCV1, tempCV1, cv::COLOR_BGRA2RGB);
		QImage ColorQImage((const uchar*)tempCV1.data, tempCV1.cols, tempCV1.rows, tempCV1.step, QImage::Format_RGB888);
		ColorQImage.bits();
		this->colorImage = ColorQImage;

		cv::Mat tempCV2 = this->CapturedRawDepthImage;
		tempCV2.convertTo(tempCV2, CV_8U, 255.0 / 5000.0, 0.0); // should be removed once we have QImage 16bit support
		QImage DepthQImage((const uchar*)tempCV2.data, tempCV2.cols, tempCV2.rows, tempCV2.step, QImage::Format_Grayscale8);
		DepthQImage.bits();
		this->depthImage = DepthQImage;

		cv::Mat tempCV3 = this->CapturedRawColorToDepthImage;
		cvtColor(tempCV3, tempCV3, cv::COLOR_BGRA2RGB);
		QImage ColorToDepthQImage((const uchar*)tempCV3.data, tempCV3.cols, tempCV3.rows, tempCV3.step, QImage::Format_RGB888);
		ColorToDepthQImage.bits();
		this->colorToDepthImage = ColorToDepthQImage;

		cv::Mat tempCV4 = this->CapturedRawDepthToColorImage;
		tempCV4.convertTo(tempCV4, CV_8U, 255.0 / 5000.0, 0.0); // should be removed once we have QImage 16bit support
		QImage DepthToColorQImage((const uchar*)tempCV4.data, tempCV4.cols, tempCV4.rows, tempCV4.step, QImage::Format_Grayscale8);
		DepthToColorQImage.bits();
		this->depthToColorImage = DepthToColorQImage;

		/** Assume that capture is all successful, otherwise print a warning. */
		if (this->colorImage.isNull() || this->depthImage.isNull() || this->colorToDepthImage.isNull() || this->depthToColorImage.isNull()
			|| this->cvDepthImage.empty() || this->cvDepthToColorImage.empty() || this->depthImageColorized.isNull() || this->depthToColorImageColorized.isNull()) {
			qWarning() << "capturetab captureButton - one of the captured images is null";
		}
		this->parent->ui.saveButtonCaptureTab->setEnabled(true);
		this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
		this->noImageCaptured = false;

		QImage image;

		// only for initial state
		if (this->parent->ui.radioButton->isChecked()) {
			image = this->colorImage;
		}
		else if (this->parent->ui.radioButton2->isChecked()) {
			image = this->depthImage;
		}
		else if (this->parent->ui.radioButton3->isChecked()) {
			image = this->colorToDepthImage;
		}
		else {
			image = this->depthToColorImage;
		}

		int width = this->parent->ui.graphicsViewImage->width(), height = this->parent->ui.graphicsViewImage->height();

		QImage imageScaled = image.scaled(width, height, Qt::KeepAspectRatio);

		// Deallocate heap memory used by previous GGraphicsScene object
		if (this->parent->ui.graphicsViewImage->scene()) {
			delete this->parent->ui.graphicsViewImage->scene();
		}

		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
		QGraphicsScene* scene = new QGraphicsScene;
		scene->addItem(item);

		this->parent->ui.graphicsViewImage->setScene(scene);
		this->parent->ui.graphicsViewImage->show();
	});

	QObject::connect(this->parent->ui.annotateButtonCaptureTab, &QPushButton::clicked, [this]() {
		/** Send RGBImageArray and DepthToRGBImageArray to server */
		QImage colorImage = this->getColorImage();
		QImage depthToColorImage = this->getDepthToColorImage();

		uploadRGBImageArrayAndDepthToRGBImageArray(manager, QUrl("http://127.0.0.1:8000/uploadimages"), QString("image_id_001"), 4, colorImage, depthToColorImage);
		/** Send to server END */

		// Move to annotate tab whose index is 3
		this->parent->annotateTab->reloadCurrentImage();
		this->parent->ui.tabWidget->setCurrentIndex(3);
		this->parent->ui.annotateButtonAnnotateTab->click();
		});

	QObject::connect(timer, &QTimer::timeout, [this]() {
		// Recording time elapsed
		this->parent->ui.recordingElapsedTime->setText(QTime::fromMSecsSinceStartOfDay(this->recordingElapsedTimer.elapsed()).toString("mm:ss"));
		// Recording time elapsed END

		//qDebug() << "timer connect start: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		if (this->parent->deviceCount > 0) {
			switch (k4a_device_get_capture(this->parent->device, &this->parent->capture, K4A_WAIT_INFINITE)) {
			case K4A_WAIT_RESULT_SUCCEEDED:
				break;
			}

			if (this->parent->capture) {
				//qDebug() << "timer connect 1: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				k4a_image_t k4aColorImage = k4a_capture_get_color_image(this->parent->capture);

				//qDebug() << "timer connect 2: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				if (k4aColorImage != NULL) {
					this->parent->colorImageQueue.push(k4aColorImage);
					//qDebug() << "timer connect 3: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

					int width = this->parent->ui.graphicsViewVideo4->width(), height = this->parent->ui.graphicsViewVideo4->height();
					//qDebug() << "timer connect 4: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					QImage qColorImageNotScaled = this->parent->getQColorImage();
					//qDebug() << "timer connect 5: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					QImage qColorImage = qColorImageNotScaled.scaled(width, height, Qt::KeepAspectRatio);

					//qDebug() << "timer connect 6: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					// Deallocate heap memory used by previous GGraphicsScene object
					if (this->parent->ui.graphicsViewVideo4->scene()) {
						delete this->parent->ui.graphicsViewVideo4->scene();
					}

					//qDebug() << "timer connect 7: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qColorImage));
					QGraphicsScene* scene = new QGraphicsScene;
					scene->addItem(item);

					//qDebug() << "timer connect 8: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					this->parent->ui.graphicsViewVideo4->setScene(scene);
					this->parent->ui.graphicsViewVideo4->show();

					//qDebug() << "timer connect 9: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					while (this->parent->colorImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
						//qDebug() << "timer connect while start: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
						k4a_image_release(this->parent->colorImageQueue.front());
						//qDebug() << "timer connect while 1: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
						this->parent->colorImageQueue.pop();
						//qDebug() << "timer connect while end: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					}
				}

				//qDebug() << "timer connect 10: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				k4a_image_t k4aDepthImage = k4a_capture_get_depth_image(this->parent->capture);

				//qDebug() << "timer connect 11: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				if (k4aDepthImage != NULL) {
					this->parent->depthImageQueue.push(k4aDepthImage);

					int width = this->parent->ui.graphicsViewVideo5->width(), height = this->parent->ui.graphicsViewVideo5->height();
					//qDebug() << "timer connect 12: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					/** Colorize live depth image preview */
					cv::Mat cvDepth = this->parent->getCVDepthImage();
					cv::Mat temp;
					colorizeDepth(cvDepth, temp);
					QImage qDepthImageNotScaled((const uchar*)temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
					qDepthImageNotScaled.bits();
					this->depthImageColorized = qDepthImageNotScaled;
					/** Colorize END */

					//qDebug() << "timer connect 13: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
					QImage qDepthImage = qDepthImageNotScaled.scaled(width, height, Qt::KeepAspectRatio);
					//qDebug() << "timer connect 14: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

					// Deallocate heap memory used by previous GGraphicsScene object
					if (this->parent->ui.graphicsViewVideo5->scene()) {
						delete this->parent->ui.graphicsViewVideo5->scene();
					}

					QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qDepthImage));
					QGraphicsScene* scene = new QGraphicsScene;
					scene->addItem(item);

					this->parent->ui.graphicsViewVideo5->setScene(scene);
					this->parent->ui.graphicsViewVideo5->show();

					while (this->parent->depthImageQueue.size() > MAX_IMAGE_QUEUE_SIZE) {
						k4a_image_release(this->parent->depthImageQueue.front());
						this->parent->depthImageQueue.pop();
					}
				}

				//qDebug() << "timer connect 15: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				k4a_capture_release(this->parent->capture);
				//qDebug() << "timer connect 16: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
			}
			else {
				qDebug() << "No capture found\n";
			}

			// Capture a imu sample
			switch (k4a_device_get_imu_sample(this->parent->device, &this->parent->imuSample, K4A_WAIT_INFINITE)) {
			case K4A_WAIT_RESULT_SUCCEEDED:
				break;
			}

			if (&this->parent->imuSample != NULL) {
				//qDebug() << "timer connect 17: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
				/** Alert if gyroscope and accelerometer show that the kinect sensor is being moved */
				alertIfMoving(
					this->parent->imuSample.gyro_sample.xyz.x,
					this->parent->imuSample.gyro_sample.xyz.y,
					this->parent->imuSample.gyro_sample.xyz.z,
					this->parent->imuSample.acc_sample.xyz.x,
					this->parent->imuSample.acc_sample.xyz.y,
					this->parent->imuSample.acc_sample.xyz.z
				);
				/** Alert if gyroscope and accelerometer show that the kinect sensor is being moved END */

				this->parent->gyroSampleQueue.push_back(this->parent->imuSample.gyro_sample);
				this->parent->accSampleQueue.push_back(this->parent->imuSample.acc_sample);
				//qDebug() << "timer connect 18: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

				QString text;
				text += ("Temperature: " + QString::number(this->parent->imuSample.temperature, 0, 2) + " C\n");
				this->parent->ui.imuText->setText(text);
			}
			//qDebug() << "timer connect 19: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

			while (this->parent->gyroSampleQueue.size() > MAX_GYROSCOPE_QUEUE_SIZE) this->parent->gyroSampleQueue.pop_front();

			//qDebug() << "timer connect 20: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

			while (this->parent->accSampleQueue.size() > MAX_ACCELEROMETER_QUEUE_SIZE) this->parent->accSampleQueue.pop_front();

			//qDebug() << "timer connect 21: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

			if (this->parent->gyroSampleQueue.size() >= MAX_GYROSCOPE_QUEUE_SIZE) this->drawGyroscopeData();

			//qDebug() << "timer connect 22: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

			if (this->parent->accSampleQueue.size() >= MAX_ACCELEROMETER_QUEUE_SIZE) this->drawAccelerometerData();

			//qDebug() << "timer connect 23: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		}
		//qDebug() << "timer connect end: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		});
}

void CaptureTab::setDefaultCaptureMode() {
	parent->ui.radioButton->setChecked(true);
	parent->ui.radioButton2->setChecked(false);
	parent->ui.radioButton3->setChecked(false);
	parent->ui.radioButton4->setChecked(false);
}

void CaptureTab::registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image) {
	QObject::connect(radioButton, &QRadioButton::clicked, [this, image]() {
		int width = this->parent->ui.graphicsViewImage->width(), height = this->parent->ui.graphicsViewImage->height();

		QImage imageScaled = (*image).scaled(width, height, Qt::KeepAspectRatio);

		// Deallocate heap memory used by previous GGraphicsScene object
		if (this->parent->ui.graphicsViewImage->scene()) {
			delete this->parent->ui.graphicsViewImage->scene();
		}

		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
		QGraphicsScene* scene = new QGraphicsScene;
		scene->addItem(item);

		this->parent->ui.graphicsViewImage->setScene(scene);
		this->parent->ui.graphicsViewImage->show();
		});
}

DesktopApp* CaptureTab::getParent()
{
	return this->parent;
}

QImage CaptureTab::getQCapturedColorImage() {
	return this->colorImage;
}

QImage CaptureTab::getQCapturedDepthToColorImageColorized() {
	return this->depthToColorImageColorized;
}

void CaptureTab::drawGyroscopeData() {
	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->parent->ui.graphicsViewGyroscope->scene()) {
		delete this->parent->ui.graphicsViewGyroscope->scene();
	}

	int width = this->parent->ui.graphicsViewGyroscope->width(), height = this->parent->ui.graphicsViewGyroscope->height();
	QGraphicsScene* scene = new QGraphicsScene();
	QImage image = QPixmap(0.95 * width, 0.95 * height).toImage();

	QPainter painter(&image);
	painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));

	for (int i = 0; i < MAX_GYROSCOPE_QUEUE_SIZE - 1; ++i) {
		int segmentLength = width / MAX_GYROSCOPE_QUEUE_SIZE;
		int segmentHeight = height / 3;

		// Draw  gyroscope measurement w.r.t x-axis
		int leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.x;
		int rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.x;
		painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t y-axis
		leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.y;
		rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.y;
		painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t z-axis
		leftSegmentHeight = 2 * this->parent->gyroSampleQueue[i].xyz.z;
		rightSegmentHeight = 2 * this->parent->gyroSampleQueue[i + 1].xyz.z;
		painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
	}

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	scene->addItem(item);

	this->parent->ui.graphicsViewGyroscope->setScene(scene);
}

void CaptureTab::drawAccelerometerData() {
	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->parent->ui.graphicsViewAccelerometer->scene()) {
		delete this->parent->ui.graphicsViewAccelerometer->scene();
	}

	int width = this->parent->ui.graphicsViewAccelerometer->width(), height = this->parent->ui.graphicsViewAccelerometer->height();
	QGraphicsScene* scene = new QGraphicsScene();
	QImage image = QPixmap(0.95 * width, 0.95 * height).toImage();

	QPainter painter(&image);
	painter.setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap));

	for (int i = 0; i < MAX_ACCELEROMETER_QUEUE_SIZE - 1; ++i) {
		int segmentLength = width / MAX_ACCELEROMETER_QUEUE_SIZE;
		int segmentHeight = height / 3;

		// Draw  gyroscope measurement w.r.t x-axis
		int leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.x;
		int rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.x;
		painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t y-axis
		leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.y;
		rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.y;
		painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t z-axis
		leftSegmentHeight = 2 * this->parent->accSampleQueue[i].xyz.z;
		rightSegmentHeight = 2 * this->parent->accSampleQueue[i + 1].xyz.z;
		painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
	}

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	scene->addItem(item);

	this->parent->ui.graphicsViewAccelerometer->setScene(scene);
}

void CaptureTab::alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ)
{
	//qDebug() << "alertIfMoving - " << gyroX << ", " << gyroY << ", " << gyroZ << ", " << accX << ", " << accY << ", " << accZ;

	if (abs(gyroX) > 1.0f || abs(gyroY) > 1.0f || abs(gyroZ) > 1.0f || abs(accX) > 1.0f || abs(accY) > 1.0f || abs(accZ + 9.81f) > 1.0f) {
		DeviceMovingDialog dialog(this);
		dialog.exec();
	}
}

void CaptureTab::onManagerFinished(QNetworkReply* reply)
{
	qDebug() << reply->readAll();
}

k4a_image_t* CaptureTab::getK4aPointCloud() {
	return &(this->k4aPointCloud);
}

k4a_image_t* CaptureTab::getK4aDepthToColor() {
	return &(this->k4aDepthToColor);
}

QVector3D CaptureTab::query3DPoint(int x, int y) {
	cv::Mat cvDepthToColorImage = this->getCVDepthToColorImage();
	uchar d = cvDepthToColorImage.at<uchar>(y, x);
	//qDebug() << "d: " << d;
	
	k4a_calibration_t calibration;
	if (k4a_device_get_calibration(this->parent->device, this->parent->deviceConfig.depth_mode, this->parent->deviceConfig.color_resolution, &calibration) != K4A_RESULT_SUCCEEDED) {
		return QVector3D(0, 0, 0);
	}

	k4a_float2_t p;
	p.xy.x = (float)x;
	p.xy.y = (float)y;
	k4a_float3_t p3D;
	int valid;
	if (k4a_calibration_2d_to_3d(&calibration, &p, d, K4A_CALIBRATION_TYPE_COLOR, K4A_CALIBRATION_TYPE_COLOR, &p3D, &valid) != K4A_WAIT_RESULT_SUCCEEDED) {
		return QVector3D(0, 0, 0);
	}
	//qDebug() << "p3D(" << p3D.xyz.x << ", " << p3D.xyz.y << ", " << p3D.xyz.z << ")";
	// source_point2d is a valid coordinate
	if (valid == 1) {
		return QVector3D(p3D.xyz.x, p3D.xyz.y, p3D.xyz.z);
	}

	return QVector3D(0, 0, 0);
}

QImage CaptureTab::getColorImage()
{
	return this->colorImage;
}

QImage CaptureTab::getDepthImage()
{
	return this->depthImage;
}

cv::Mat CaptureTab::getCapturedRawColorImage() {
	return this->CapturedRawColorImage;
}

cv::Mat CaptureTab::getCapturedRawDepthImage() {
	return this->CapturedRawDepthImage;
}

cv::Mat CaptureTab::getCapturedRawColorToDepthImage() {
	return this->CapturedRawColorToDepthImage;
}

cv::Mat CaptureTab::getCapturedRawDepthToColorImage() {
	return this->CapturedRawDepthToColorImage;
}

QImage CaptureTab::getDepthImageColorized()
{
	return this->depthImageColorized;
}

cv::Mat CaptureTab::getCVDepthImage()
{
	return this->cvDepthImage;
}

QImage CaptureTab::getColorToDepthImage()
{
	return this->colorToDepthImage;
}

QImage CaptureTab::getDepthToColorImage()
{
	return this->depthToColorImage;
}

QImage CaptureTab::getDepthToColorImageColorized()
{
	return this->depthToColorImageColorized;
}

cv::Mat CaptureTab::getCVDepthToColorImage()
{
	return this->cvDepthToColorImage;
}

int CaptureTab::getCaptureCount() { return this->captureCount; }

void CaptureTab::setCaptureCount(int newCaptureCount) { this->captureCount = newCaptureCount; }

Recorder* CaptureTab::getRecorder() { return this->recorder; }

QString CaptureTab::getCaptureFilepath() { return this->captureFilepath; }
void CaptureTab::setCaptureFilepath(QString captureFilepath) { this->captureFilepath = captureFilepath; }
