#include "capturetab.h"
#include "saveimagedialog.h"
#include "devicemovingdialog.h"
#include "kinectengine.h"

CaptureTab::CaptureTab(DesktopApp* parent)
{
	this->parent = parent;
	this->recorder = new Recorder(parent);
	this->parent->ui.recordingIndicatorText->setVisible(false);
	this->parent->ui.recordingElapsedTime->setVisible(false);

	this->setDefaultCaptureMode();

	this->registerRadioButtonOnClicked(this->parent->ui.radioButton, &this->qColorImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton2, &this->qDepthImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton3, &this->qColorToDepthImage);
	this->registerRadioButtonOnClicked(this->parent->ui.radioButton4, &this->qDepthToColorImage);

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
		KinectEngine::getInstance().readAllImages(this->capturedColorImage, this->capturedDepthImage, this->capturedColorToDepthImage, this->capturedDepthToColorImage);
		
		// Shallow copy
		cv::Mat color = this->capturedColorImage;
		cv::Mat depth = this->capturedDepthImage;
		cv::Mat colorToDepth = this->capturedColorToDepthImage;
		cv::Mat depthToColor = this->capturedDepthToColorImage;
		//

		/** Assume that capture is all successful, otherwise print a warning. */
		if (color.empty() || depth.empty() || colorToDepth.empty() || depthToColor.empty()) {
			qWarning() << "capturetab captureButton - one of the captured images is null";
		}
		this->parent->ui.saveButtonCaptureTab->setEnabled(true);
		this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
		this->noImageCaptured = false;

		/*
		* Display captured images
		*/
		this->qColorImage = convertColorCVToQImage(color);
		this->qDepthImage = convertDepthCVToQImage(depth);
		this->qColorToDepthImage = convertColorToDepthCVToQImage(colorToDepth);
		this->qDepthToColorImage = convertDepthToColorCVToQImage(depthToColor);
		// For annotatetab instead
		this->qDepthToColorColorizedImage = convertDepthToColorCVToColorizedQImage(depthToColor);
		// For annotatetab instead END

		QImage image;

		// only for initial state
		if (this->parent->ui.radioButton->isChecked()) {
			image = this->getQColorImage();
		}
		else if (this->parent->ui.radioButton2->isChecked()) {
			image = this->getQDepthImage();
		}
		else if (this->parent->ui.radioButton3->isChecked()) {
			image = this->getQColorToDepthImage();
		}
		else {
			image = this->getQDepthToColorImage();
		}

		int width = this->parent->ui.graphicsViewImage->width();
		int height = this->parent->ui.graphicsViewImage->height();

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
		/*
		* Display captured images END
		*/
		});

	QObject::connect(this->parent->ui.annotateButtonCaptureTab, &QPushButton::clicked, [this]() {
		/** Send RGBImageArray and DepthToRGBImageArray to server */
		QImage colorImage = this->getQColorImage();
		QImage depthToColorImage = this->getQDepthToColorImage();

		uploadRGBImageArrayAndDepthToRGBImageArray(manager, QUrl("http://127.0.0.1:8000/uploadimages"), QString("image_id_001"), 4, colorImage, depthToColorImage);
		/** Send to server END */

		// Move to annotate tab whose index is 4
		this->parent->annotateTab->reloadCurrentImage();
		this->parent->ui.tabWidget->setCurrentIndex(4);
		});

	QObject::connect(timer, &QTimer::timeout, [this]() {
		//qDebug() << "timer connect start: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

		KinectEngine::getInstance().captureImages();
		cv::Mat color, depth;
		KinectEngine::getInstance().readColorAndDepthImages(color, depth);
		QImage qColor = convertColorCVToQImage(color);
		QImage qDepth = convertDepthCVToColorizedQImage(depth);

		// Recording time elapsed
		this->parent->ui.recordingElapsedTime->setText(QTime::fromMSecsSinceStartOfDay(this->recordingElapsedTimer.elapsed()).toString("mm:ss"));
		// Recording time elapsed END

		/*
		* Display color image
		*/
		if (!qColor.isNull()) {
			int width = this->parent->ui.graphicsViewVideo4->width();
			int height = this->parent->ui.graphicsViewVideo4->height();
			QImage qColorScaled = qColor.scaled(width, height, Qt::KeepAspectRatio);

			// Deallocate heap memory used by previous GGraphicsScene object
			if (this->parent->ui.graphicsViewVideo4->scene()) {
				delete this->parent->ui.graphicsViewVideo4->scene();
			}
			// Deallocate heap memory used by previous GGraphicsScene object END

			QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qColorScaled));
			QGraphicsScene* scene = new QGraphicsScene;
			scene->addItem(item);

			/** Human cut shape */
			QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape.png");
			QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);
			scene->addPixmap(humanPixmapScaled);
			/** Human cut shape END */

			this->parent->ui.graphicsViewVideo4->setScene(scene);
			this->parent->ui.graphicsViewVideo4->show();
		}
		/*
		* Display color image END
		*/


		/*
		* Display depth image
		*/
		if (!qDepth.isNull()) {
			int width = this->parent->ui.graphicsViewVideo5->width();
			int height = this->parent->ui.graphicsViewVideo5->height();
			QImage qDepthScaled = qDepth.scaled(width, height, Qt::KeepAspectRatio);

			// Deallocate heap memory used by previous GGraphicsScene object
			if (this->parent->ui.graphicsViewVideo5->scene()) {
				delete this->parent->ui.graphicsViewVideo5->scene();
			}
			// Deallocate heap memory used by previous GGraphicsScene object END

			QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(qDepthScaled));
			QGraphicsScene* scene = new QGraphicsScene;
			scene->addItem(item);

			this->parent->ui.graphicsViewVideo5->setScene(scene);
			this->parent->ui.graphicsViewVideo5->show();
		}
		/*
		* Display depth image END
		*/

		/*
		* Record color image video and depth image video
		*/
		// OpenCV cannot save 16-bit video. Therefore, convert to 8-bit.
		cv::Mat depth8bit;
		depth.convertTo(depth8bit, CV_8U, 255.0 / 5000.0, 0.0);
		// If recording mode is on, send temp to the output file stream
		if (this->getRecorder()->getRecordingStatus()) {
			*(this->getRecorder()->getColorVideoWriter()) << color;
			*(this->getRecorder()->getDepthVideoWriter()) << depth8bit;
		}
		/*
		* Record color image video and depth image video END
		*/

		/*
		* IMU sample
		*/
		KinectEngine::getInstance().queueIMUSample();
		std::deque<k4a_float3_t> gyroSampleQueue = KinectEngine::getInstance().getGyroSampleQueue();
		std::deque<k4a_float3_t> accSampleQueue = KinectEngine::getInstance().getAccSampleQueue();
		float temperature = KinectEngine::getInstance().getTemperature();

		if (!gyroSampleQueue.empty() && !accSampleQueue.empty()) {
			//qDebug() << "timer connect 17: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
			/** Alert if gyroscope and accelerometer show that the kinect sensor is being moved */
			alertIfMoving(
				gyroSampleQueue[gyroSampleQueue.size() - 1].xyz.x,
				gyroSampleQueue[gyroSampleQueue.size() - 1].xyz.y,
				gyroSampleQueue[gyroSampleQueue.size() - 1].xyz.z,
				accSampleQueue[accSampleQueue.size() - 1].xyz.x,
				accSampleQueue[accSampleQueue.size() - 1].xyz.y,
				accSampleQueue[accSampleQueue.size() - 1].xyz.z
			);
			/** Alert if gyroscope and accelerometer show that the kinect sensor is being moved END */

			QString text;
			text += ("Temperature: " + QString::number(temperature, 0, 2) + " C\n");
			this->parent->ui.imuText->setText(text);
		}

		if (gyroSampleQueue.size() >= MAX_GYROSCOPE_QUEUE_SIZE) this->drawGyroscopeData(gyroSampleQueue);
		if (accSampleQueue.size() >= MAX_ACCELEROMETER_QUEUE_SIZE) this->drawAccelerometerData(accSampleQueue);
		/*
		* IMU sample END
		*/

		//qDebug() << "timer connect end: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		}
	);

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

void CaptureTab::drawGyroscopeData(std::deque<k4a_float3_t> gyroSampleQueue) {
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
		int leftSegmentHeight = 2 * gyroSampleQueue[i].xyz.x;
		int rightSegmentHeight = 2 * gyroSampleQueue[i + 1].xyz.x;
		painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t y-axis
		leftSegmentHeight = 2 * gyroSampleQueue[i].xyz.y;
		rightSegmentHeight = 2 * gyroSampleQueue[i + 1].xyz.y;
		painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t z-axis
		leftSegmentHeight = 2 * gyroSampleQueue[i].xyz.z;
		rightSegmentHeight = 2 * gyroSampleQueue[i + 1].xyz.z;
		painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
	}

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	scene->addItem(item);

	this->parent->ui.graphicsViewGyroscope->setScene(scene);
}

void CaptureTab::drawAccelerometerData(std::deque<k4a_float3_t> accSampleQueue) {
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
		int leftSegmentHeight = 2 * accSampleQueue[i].xyz.x;
		int rightSegmentHeight = 2 * accSampleQueue[i + 1].xyz.x;
		painter.drawLine(i * segmentLength, segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t y-axis
		leftSegmentHeight = 2 * accSampleQueue[i].xyz.y;
		rightSegmentHeight = 2 * accSampleQueue[i + 1].xyz.y;
		painter.drawLine(i * segmentLength, 3 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 3 * segmentHeight / 2 + rightSegmentHeight);

		// Draw  gyroscope measurement w.r.t z-axis
		leftSegmentHeight = 2 * accSampleQueue[i].xyz.z;
		rightSegmentHeight = 2 * accSampleQueue[i + 1].xyz.z;
		painter.drawLine(i * segmentLength, 5 * segmentHeight / 2 + leftSegmentHeight, (i + 1) * segmentLength, 5 * segmentHeight / 2 + rightSegmentHeight);
	}

	QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
	scene->addItem(item);

	this->parent->ui.graphicsViewAccelerometer->setScene(scene);
}

void CaptureTab::alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ)
{
	//qDebug() << "alertIfMoving - " << gyroX << ", " << gyroY << ", " << gyroZ << ", " << accX << ", " << accY << ", " << accZ;

	if (abs(accX) > 1.0f || abs(accY) > 1.0f || abs(accZ + 9.81) > 1.0f) {
		DeviceMovingDialog dialog(this);
		dialog.exec();
	}
}

void CaptureTab::onManagerFinished(QNetworkReply* reply)
{
	qDebug() << reply->readAll();
}

cv::Mat CaptureTab::getCapturedColorImage() {
	return this->capturedColorImage;
}

cv::Mat CaptureTab::getCapturedDepthImage() {
	return this->capturedDepthImage;
}

cv::Mat CaptureTab::getCapturedColorToDepthImage() {
	return this->capturedColorToDepthImage;
}

cv::Mat CaptureTab::getCapturedDepthToColorImage() {
	return this->capturedDepthToColorImage;
}

QImage CaptureTab::getQColorImage()
{
	return this->qColorImage;
}

QImage CaptureTab::getQDepthImage()
{
	return this->qDepthImage;
}

QImage CaptureTab::getQColorToDepthImage()
{
	return this->qColorToDepthImage;
}

QImage CaptureTab::getQDepthToColorImage()
{
	return this->qDepthToColorImage;
}

QImage CaptureTab::getQDepthToColorColorizedImage()
{
	return this->qDepthToColorColorizedImage;
}

int CaptureTab::getCaptureCount() { return this->captureCount; }

void CaptureTab::setCaptureCount(int newCaptureCount) { this->captureCount = newCaptureCount; }

Recorder* CaptureTab::getRecorder() { return this->recorder; }

QString CaptureTab::getCaptureFilepath() { return this->captureFilepath; }
void CaptureTab::setCaptureFilepath(QString captureFilepath) { this->captureFilepath = captureFilepath; }
