#include "capturetab.h"
#include "saveimagedialog.h"
#include "kinectengine.h"
#include <Windows.h>

const int COLUMN_COUNT = 4;

CaptureTab::CaptureTab(DesktopApp* parent)
{
	this->parent = parent;
	this->recorder = new Recorder(parent);
	this->parent->ui.recordingIndicatorText->setVisible(false);
	this->parent->ui.recordingElapsedTime->setVisible(false);
	this->parent->ui.progressBar->setVisible(false);

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

	this->isUploading = false;

	this->parent->ui.showInExplorer->hide();
	this->captureFilepath = QString();

	/** Select image table view */
	tableView = this->parent->ui.captureTab->findChild<QTableView*>("tableViewSelectImage");
	dataModel = new QStandardItemModel(0, COLUMN_COUNT, this);
	tableView->setModel(this->dataModel);

	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	/** Set only the "Image Type" column editable */
	//for (int c = 0; c < dataModel->columnCount(); c++)
	//{
	//	if (c != 1)
	//		tableView->setItemDelegateForColumn(c, new NotEditableDelegate(tableView));
	//}

	//tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	/** Set only the "Image Type" column editable END */

	tableView->verticalHeader()->hide();

	/** Handle row selection, including up/down arrow press */
	bool value = connect(tableView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(onSlotRowSelected(const QModelIndex&, const QModelIndex&)));
	/** Handle row selection END */

	tableView->setTabKeyNavigation(false);
	/** Select image table view END */

	// Clear data and re-fetch all data every time
	dataModel->clear();

	/** Headers */
	QStringList headerLabels = { "", "index", "Image Type", "Creation Time" };

	for (int i = 0; i < COLUMN_COUNT; i++)
	{
		QString text = headerLabels.at(i);
		QStandardItem* item = new QStandardItem(text);
		QFont fn = item->font();
		fn.setPixelSize(14);
		item->setFont(fn);

		dataModel->setHorizontalHeaderItem(i, item);
	}

	/** This must be put here (below) */
	tableView->setColumnWidth(0, 18);
	tableView->setColumnWidth(2, 150);
	tableView->setColumnWidth(3, tableView->width() - 150 - 50);
	/** This must be put here (below) END */
	/** Headers END */

	tableView->hideColumn(1);

	tableView->show();

	QObject::connect(this->parent->ui.saveButtonCaptureTab, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}
		SaveImageDialog dialog(this);
		dialog.exec();
		});

	QObject::connect(this->parent->ui.saveVideoButton, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}
		if (this->recorder->getRecordingStatus()) {
			// Current status is recording
			QString dateTimeString = Helper::getCurrentDateTimeString();
			QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);

			/** Re-enable changing tab */
			this->parent->ui.tabWidget->setTabEnabled(0, true);
			this->parent->ui.tabWidget->setTabEnabled(1, true);
			this->parent->ui.tabWidget->setTabEnabled(2, true);
			this->parent->ui.tabWidget->setTabEnabled(4, true);
			this->parent->ui.tabWidget->setTabEnabled(5, true);
			/** Re-enable changing tab END */

			// Modify UI to disable recording status
			this->parent->ui.recordingIndicatorText->setVisible(false);
			this->parent->ui.recordingElapsedTime->setVisible(false);
			this->parent->ui.captureTab->setStyleSheet("");

			this->recorder->stopRecorder();
			this->parent->ui.saveVideoButton->setText("Record");

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

			/** Disable changing tab */
			this->parent->ui.tabWidget->setTabEnabled(0, false);
			this->parent->ui.tabWidget->setTabEnabled(1, false);
			this->parent->ui.tabWidget->setTabEnabled(2, false);
			this->parent->ui.tabWidget->setTabEnabled(4, false);
			this->parent->ui.tabWidget->setTabEnabled(5, false);
			/** Disable changing tab END */

			// Modify UI to indicate recording status
			this->parent->ui.recordingIndicatorText->setVisible(true);
			this->parent->ui.recordingElapsedTime->setVisible(true);
			this->parent->ui.captureTab->setStyleSheet("#captureTab {border: 2px solid red}");

			this->recorder->prepareRecorder();
			this->parent->ui.saveVideoButton->setText("Stop");

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

	QObject::connect(this->parent->ui.showInExplorer, &QPushButton::clicked, [this]() {
		QString filepath = this->getCaptureFilepath();

		QStringList args;

		args << "/select," << QDir::toNativeSeparators(filepath);

		QProcess* process = new QProcess(this);
		process->startDetached("explorer.exe", args);
		});

	QObject::connect(this->parent->ui.captureButton, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}

		this->imageType = this->parent->ui.imageTypeComboBox->currentText().split("-")[0].trimmed().toInt();
		this->imageName = this->parent->ui.imageTypeComboBox->currentText();
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
			TwoLinesDialog dialog;
			dialog.setLine1("Capture failed!");
			dialog.exec();
			return;
		}
		this->parent->ui.saveButtonCaptureTab->setEnabled(true);
		this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
		this->noImageCaptured = false;

		this->RANSACImage = computeNormalizedDepthImage(depthToColor);

		/*this->RANSACImage.convertTo(this->RANSACImage, CV_8U, 255.0 / 5000.0, 0.0);
		cv::imshow("ransac", this->RANSACImage);
		cv::waitKey(0);
		cv::destroyWindow("ransac");*/

		/** Insert index to data model */
		QList<QStandardItem*> itemList;
		QStandardItem* dataItem;
		for (int i = 0; i < COLUMN_COUNT; i++)
		{
			QString text;
			switch (i) {
			case 0:
				text = QString::number(dataModel->rowCount() + 1);
				break;
			case 1:
				text = QString::number(dataModel->rowCount());
				break;
			case 2:
				//text = QString::number(imageType);
				text = imageName;
				break;
			case 3:
				QDateTime dateTime = dateTime.currentDateTime();
				text = dateTime.toString("yyyy-MM-dd HH:mm:ss");
				break;
			}
			dataItem = new QStandardItem(text);
			QFont fn = dataItem->font();
			fn.setPixelSize(14);
			dataItem->setFont(fn);
			itemList << dataItem;
		}

		dataModel->insertRow(0, itemList);
		/** Insert index to data model END */


		this->qColorImage = convertColorCVToQImage(color);
		this->qDepthImage = convertDepthCVToQImage(depth);
		this->qColorToDepthImage = convertColorToDepthCVToQImage(colorToDepth);
		this->qDepthToColorImage = convertDepthToColorCVToQImage(depthToColor);
		// For annotatetab instead
		this->qDepthToColorColorizedImage = convertDepthToColorCVToColorizedQImage(depthToColor);
		// For annotatetab instead END

		/** Store histories of images for selection */
		CaptureHistory captureHistory;
		captureHistory.imageType = imageType;
		captureHistory.imageName = imageName;
		captureHistory.capturedColorImage = capturedColorImage;
		captureHistory.capturedDepthImage = capturedDepthImage;
		captureHistory.capturedColorToDepthImage = capturedColorToDepthImage;
		captureHistory.capturedDepthToColorImage = capturedDepthToColorImage;
		captureHistory.RANSACImage = RANSACImage;
		captureHistory.qColorImage = qColorImage;
		captureHistory.qDepthImage = qDepthImage;
		captureHistory.qColorToDepthImage = qColorToDepthImage;
		captureHistory.qDepthToColorImage = qDepthToColorImage;
		captureHistory.qDepthToColorColorizedImage = qDepthToColorColorizedImage;
		captureHistories.push_back(captureHistory);
		/** Store histories of images for selection END */

		displayCapturedImages();
		});

	QObject::connect(this->parent->ui.annotateButtonCaptureTab, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}
		qDebug() << "Analysis button clicked";
		this->isUploading = true;
		
		this->disableButtonsForUploading();

		/** Select image table view update UI to green background, showing successful image analysis */
		storedTableViewRow = imageBeingAnalyzedTableViewRow;
		/** Select image table view update UI to green background, showing successful image analysis END */
		/** Store the image type of the image being analyzed */
		imageTypeBeingAnalyzed = imageType;
		/** Store the image type of the image being analyzed END */

		/* Convert to the special 4 channels image and upload */
		cv::Mat color3 = this->capturedColorImage;
		std::vector<cv::Mat>channelsForColor2(3);
		cv::split(color3, channelsForColor2);

		cv::Mat depthToColor3 = this->capturedDepthToColorImage;
		std::vector<cv::Mat>channelsForDepth1(1);
		cv::split(depthToColor3, channelsForDepth1);

		cv::Mat normalizedDepthToColor = this->RANSACImage;
		std::vector<cv::Mat>channelsForDepth2(1);
		cv::split(normalizedDepthToColor, channelsForDepth2);

		int width = COLOR_IMAGE_WIDTH;
		int height = COLOR_IMAGE_HEIGHT;

		cv::Mat FourChannelPNG = cv::Mat::ones(height, width, CV_16UC4);;
		std::vector<cv::Mat>channels3(4);
		cv::split(FourChannelPNG, channels3);


		// channelsForColor2 = BGR
		for (int i = 0; i < width * height; i++) {
			channels3[0].at<uint16_t>(i) = (channelsForColor2[2].at<uint8_t>(i) << 8) | channelsForColor2[1].at<uint8_t>(i);
			channels3[1].at<uint16_t>(i) = (channelsForColor2[0].at<uint8_t>(i) << 8);
		}
		channels3[2] = channelsForDepth1[0];
		channels3[3] = channelsForDepth2[0];

		cv::merge(channels3, FourChannelPNG);

		qDebug() << "Merging image completed";

		QNetworkClient::getInstance().uploadImage(FourChannelPNG, this, SLOT(onUploadImage(QNetworkReply*)));
		/* Convert to the special 4 channels image and upload END */

		// Moving to annotate tab will be done after the series of requests is sent to obtain the landmark predictions */
	});

	QObject::connect(timer, &QTimer::timeout, [this]() {
		//qDebug() << "timer connect start: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		if (!KinectEngine::getInstance().isDeviceOpened()) {
			return;
		}

		KinectEngine::getInstance().captureImages();
		cv::Mat color, depth;
		KinectEngine::getInstance().readColorAndDepthImages(color, depth);
		int cropPerSide = KinectEngine::getInstance().COLOR_IMAGE_CROP_WIDTH_PER_SIDE;
		QImage qColor = convertColorCVToQImage(color);

		// Crop left and right
		qColor = qColor.copy(cropPerSide, 0, COLOR_IMAGE_WIDTH - 2 * cropPerSide, COLOR_IMAGE_HEIGHT);
		// Crop left and right END

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
			QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape4.png");
			QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);

			// Crop left and right
			// Don't crop human cut shape if the original image also does not need the crop
			if (KinectEngine::getInstance().COLOR_IMAGE_CROP_WIDTH_PER_SIDE != 0) {
				cropPerSide = (humanPixmapScaled.width() - humanPixmapScaled.height()) / 2;
				humanPixmapScaled = humanPixmapScaled.copy(cropPerSide, 0, humanPixmapScaled.width() - 2 * cropPerSide, humanPixmapScaled.height());
			}
			// Crop left and right END

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
		bool queueIMUSuccess = KinectEngine::getInstance().queueIMUSample();
		std::deque<k4a_float3_t> gyroSampleQueue = KinectEngine::getInstance().getGyroSampleQueue();
		std::deque<k4a_float3_t> accSampleQueue = KinectEngine::getInstance().getAccSampleQueue();

		if (queueIMUSuccess && !gyroSampleQueue.empty() && !accSampleQueue.empty()) {
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

		}
		/*
		* IMU sample END
		*/

		//qDebug() << "timer connect end: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		}
	);

}

void CaptureTab::clearCaptureHistories() {
	qDebug() << "How many records: " << captureHistories.size();
	for (int i = 0; i < captureHistories.size(); i++) {
		dataModel->removeRow(0, QModelIndex());
	}

	captureHistories.clear();

	/** Reset variables stored in CaptureTab */
	capturedColorImage.release();
	capturedDepthImage.release();
	capturedColorToDepthImage.release();
	capturedDepthToColorImage.release();
	qColorImage = QImage();
	qDepthImage = QImage();
	qColorToDepthImage = QImage();
	qDepthToColorImage = QImage();
	qDepthToColorColorizedImage = QImage();
	RANSACImage.release();
	imageType = -1; // Update on image selection
	imageName = "";
	imageTypeBeingAnalyzed = -1; // Update on analysis button pressed
	/** Reset variables stored in CaptureTab END */

	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->parent->ui.graphicsViewImage->scene()) {
		delete this->parent->ui.graphicsViewImage->scene();
	}
	this->parent->ui.patientNameInCapture->setText("Current Patient: " + this->parent->patientTab->getCurrentPatientName());

}

void CaptureTab::setDefaultCaptureMode() {
	parent->ui.radioButton->setChecked(true);
	parent->ui.radioButton2->setChecked(false);
	parent->ui.radioButton3->setChecked(false);
	parent->ui.radioButton4->setChecked(false);
}

void CaptureTab::disableButtonsForUploading() {

	this->parent->ui.progressBar->setVisible(true);
	this->parent->ui.progressBar->setValue(1);

	this->parent->ui.captureButton->setEnabled(false);
	this->parent->ui.saveVideoButton->setEnabled(false);
	this->parent->ui.saveButtonCaptureTab->setEnabled(false);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(false);
	this->parent->ui.radioButton->setEnabled(false);
	this->parent->ui.radioButton2->setEnabled(false);
	this->parent->ui.radioButton3->setEnabled(false);
	this->parent->ui.radioButton4->setEnabled(false);

	this->parent->ui.tabWidget->setTabEnabled(0, false);
	this->parent->ui.tabWidget->setTabEnabled(1, false);
	this->parent->ui.tabWidget->setTabEnabled(2, false);
	this->parent->ui.tabWidget->setTabEnabled(4, false);
	this->parent->ui.tabWidget->setTabEnabled(5, false);
	
}

void CaptureTab::enableButtonsForUploading() {

	this->parent->ui.progressBar->setValue(1);
	this->parent->ui.progressBar->setVisible(false);

	this->parent->ui.captureButton->setEnabled(true);
	this->parent->ui.saveVideoButton->setEnabled(true);
	this->parent->ui.saveButtonCaptureTab->setEnabled(true);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
	this->parent->ui.radioButton->setEnabled(true);
	this->parent->ui.radioButton2->setEnabled(true);
	this->parent->ui.radioButton3->setEnabled(true);
	this->parent->ui.radioButton4->setEnabled(true);

	this->parent->ui.tabWidget->setTabEnabled(0, true);
	this->parent->ui.tabWidget->setTabEnabled(1, true);
	this->parent->ui.tabWidget->setTabEnabled(2, true);
	this->parent->ui.tabWidget->setTabEnabled(4, true);
	this->parent->ui.tabWidget->setTabEnabled(5, true);

}

void CaptureTab::registerRadioButtonOnClicked(QRadioButton* radioButton, QImage* image) {
	QObject::connect(radioButton, &QRadioButton::clicked, [this, image]() {

		if (this->isUploading) {
			return;
		}

		int width = this->parent->ui.graphicsViewImage->width(), height = this->parent->ui.graphicsViewImage->height();

		QImage imageScaled = (*image).scaled(width, height, Qt::KeepAspectRatio);

		// Deallocate heap memory used by previous GGraphicsScene object
		if (this->parent->ui.graphicsViewImage->scene()) {
			delete this->parent->ui.graphicsViewImage->scene();
		}

		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
		QGraphicsScene* scene = new ClipGraphicsScene();
		scene->addItem(item);

		this->parent->ui.graphicsViewImage->setScene(scene);
		this->parent->ui.graphicsViewImage->show();
		});
}

DesktopApp* CaptureTab::getParent()
{
	return this->parent;
}

/*
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
}*/

void CaptureTab::alertIfMoving(float gyroX, float gyroY, float gyroZ, float accX, float accY, float accZ)
{
	//qDebug() << "alertIfMoving - " << gyroX << ", " << gyroY << ", " << gyroZ << ", " << accX << ", " << accY << ", " << accZ;

	if (abs(accX) > 1.0f || abs(accY) > 1.0f || abs(accZ + 9.81) > 1.0f) {
		TwoLinesDialog dialog;
		dialog.setLine1("Azure Kinect sensor not horizontally level.");
		dialog.setLine2("Please adjust the sensor and press OK.");
		dialog.exec();
	}
}

void CaptureTab::onManagerFinished(QNetworkReply* reply)
{
	qDebug() << reply->readAll();
}

void CaptureTab::onUploadImage(QNetworkReply* reply) {
	qDebug() << "onUploadImage";
	
	QString url = reply->readAll();

	/** TIMEOUT */
	if (url == nullptr) {
		/** Select image table view update UI to red background, showing unsuccessful image analysis */
		for (int i = 0; i < dataModel->columnCount(); i++) {
			QModelIndex index;
			index = dataModel->index(storedTableViewRow, i);
			dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
		}
		/** Select image table view update UI to red background, showing unsuccessful image analysis END */

		TwoLinesDialog dialog;
		dialog.setLine1("Analysis Step 1 Timeout!");
		dialog.exec();

		this->enableButtonsForUploading();

		this->isUploading = false;
		/******/

		return;
	}
	/** TIMEOUT END */

	this->parent->ui.progressBar->setValue(33);

	qDebug() << url;

	reply->deleteLater();

	if (url.contains("error")) {
		qCritical() << "onUploadImage received error reply!";
		TwoLinesDialog dialog;
		dialog.setLine1("onUploadImage received error reply!");
		dialog.exec();
		return;
	}

	QNetworkClient::getInstance().bindImageUrl(this->parent->patientTab->getCurrentPatientId(), url, this->imageTypeBeingAnalyzed, this, SLOT(onBindImageUrl(QNetworkReply*)));
}

void CaptureTab::onBindImageUrl(QNetworkReply* reply) {
	qDebug() << "onBindImageUrl";
	
	QByteArray response_data = reply->readAll();
	reply->deleteLater();

	/** TIMEOUT */
	if (response_data == nullptr) {
		/** Select image table view update UI to red background, showing unsuccessful image analysis */
		for (int i = 0; i < dataModel->columnCount(); i++) {
			QModelIndex index;
			index = dataModel->index(storedTableViewRow, i);
			dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
		}
		/** Select image table view update UI to red background, showing unsuccessful image analysis END */

		TwoLinesDialog dialog;
		dialog.setLine1("Analysis Step 2 Timeout!");
		dialog.exec();

		this->enableButtonsForUploading();

		this->isUploading = false;
		/******/

		return;
	}
	/** TIMEOUT END */

	this->parent->ui.progressBar->setValue(66);

	QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

	qDebug() << jsonResponse;

	QJsonObject obj = jsonResponse.object();
	int imageId = obj["id"].toInt();

	qDebug() << "Sleep 2 seconds";

	Sleep(2000);

	/** For sending findLandmarkPredictions() more than once */
	this->currentImageId = imageId;
	this->landmarkRequestSent = 1;
	/** For sending findLandmarkPredictions() more than once END */

	QNetworkClient::getInstance().findLandmarkPredictions(imageId, this, SLOT(onFindLandmarkPredictions(QNetworkReply*)));
}

void CaptureTab::onFindLandmarkPredictions(QNetworkReply* reply) {
	qDebug() << "onFindLandmarkPredictions";

	QByteArray response_data = reply->readAll();
	reply->deleteLater();

	QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);
	qDebug() << jsonResponse;

	QJsonObject obj = jsonResponse.object();
	QString aiImageUrl = obj["aiImageUrl"].toString();

	/** TIMEOUT */
	if (response_data == nullptr) {
		/** For sending findLandmarkPredictions() more than once */
		if (landmarkRequestSent < MAX_LANDMARK_REQUEST_SENT) {
			Sleep(2500);
			landmarkRequestSent++;
			QNetworkClient::getInstance().findLandmarkPredictions(this->currentImageId, this, SLOT(onFindLandmarkPredictions(QNetworkReply*)));
			return;
		}
		/** For sending findLandmarkPredictions() more than once END */

		/** Select image table view update UI to red background, showing unsuccessful image analysis */
		for (int i = 0; i < dataModel->columnCount(); i++) {
			QModelIndex index;
			index = dataModel->index(storedTableViewRow, i);
			dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
		}
		/** Select image table view update UI to red background, showing unsuccessful image analysis END */

		TwoLinesDialog dialog;
		dialog.setLine1("Analysis Step 3 Timeout!");
		dialog.exec();

		/******/
		this->parent->ui.progressBar->setValue(1);
		this->parent->ui.progressBar->setVisible(false);
		this->parent->ui.captureButton->setEnabled(true);
		this->parent->ui.saveVideoButton->setEnabled(true);
		this->parent->ui.saveButtonCaptureTab->setEnabled(true);
		this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
		this->parent->ui.radioButton->setEnabled(true);
		this->parent->ui.radioButton2->setEnabled(true);
		this->parent->ui.radioButton3->setEnabled(true);
		this->parent->ui.radioButton4->setEnabled(true);
		/** Re-enable changing tab */
		this->parent->ui.tabWidget->setTabEnabled(0, true);
		this->parent->ui.tabWidget->setTabEnabled(1, true);
		this->parent->ui.tabWidget->setTabEnabled(2, true);
		this->parent->ui.tabWidget->setTabEnabled(4, true);
		this->parent->ui.tabWidget->setTabEnabled(5, true);
		/** Re-enable changing tab END */
		this->isUploading = false;
		/******/

		return;
	}
	/** TIMEOUT END */

	int imageId = obj["id"].toInt();
	this->parent->annotateTab->setAiImageUrl(aiImageUrl);

	QString aiOriginResult = obj["aiOriginResult"].toString();

	qDebug() << "imageId:" << imageId;
	qDebug() << "aiImageUrl:" << aiImageUrl;
	qDebug() << "aiOriginResult:" << aiOriginResult;

	if (aiOriginResult == "") {
		switch (COLOR_IMAGE_WIDTH) {
		case 1920:
			aiOriginResult = "[[960.0, 300.0], [1050.0, 450.0], [870.0, 450.0], [1050.0, 750.0], [870.0, 750.0], [960.0, 900.0]]";
			break;
		case 1280:
			aiOriginResult = "[[640.0, 200.0], [700.0, 300.0], [580.0, 300.0], [700.0, 500.0], [580.0, 500.0], [640.0, 600.0]]";
			break;
		}
	}

	AnnotateTab* annotateTab = this->parent->annotateTab;
	annotateTab->imageId = imageId;

	QStringList list = aiOriginResult.split(",");
	for (int i = 0; i < list.size(); i++) {
		QString chopped = list[i].remove("[").remove("]");
		float f = chopped.toFloat();

		switch (i) {
			case 0: annotateTab->predictedCX = f; break;
			case 1: annotateTab->predictedCY = f; break;
			case 2: annotateTab->predictedA2X = f; break;
			case 3: annotateTab->predictedA2Y = f; break;
			case 4: annotateTab->predictedA1X = f; break;
			case 5: annotateTab->predictedA1Y = f; break;
			case 6: annotateTab->predictedB2X = f; break;
			case 7: annotateTab->predictedB2Y = f; break;
			case 8: annotateTab->predictedB1X = f; break;
			case 9: annotateTab->predictedB1Y = f; break;
			case 10: annotateTab->predictedDX = f; break;
			case 11: annotateTab->predictedDY = f; break;
		}
	}

	/** Select image table view update UI to green background, showing successful image analysis */
	for (int i = 0; i < dataModel->columnCount(); i++) {
		QModelIndex index;
		index = dataModel->index(storedTableViewRow, i);
		dataModel->setData(index, QColor(Qt::green), Qt::BackgroundRole);
	}
	/** Select image table view update UI to green background, showing successful image analysis END */

	this->enableButtonsForUploading();

	this->isUploading = false;

	// Move to annotate tab which index is 4
	this->parent->annotateTab->reloadCurrentImage();
	this->parent->ui.tabWidget->setCurrentIndex(4);
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

cv::Mat CaptureTab::computeNormalizedDepthImage(cv::Mat depthToColorImage) {

	// RANSAC
	int k = 6;
	int threshold = 15;

	int iterationCount = 0;
	int inlierCount = 0;

	int MaxInlierCount = -1;
	float PlaneA, PlaneB, PlaneC, PlaneD;
	float BestPointOne[2];
	float BestPointTwo[2];
	float BestPointThree[2];

	float a, b, c, d;
	float PointOne[2];
	float PointTwo[2];
	float PointThree[2];

	srand((unsigned)time(0));

	int IgnoreLeftAndRightPixel = 200;
	int IgnoreMiddlePixel = 200;

	while (iterationCount <= k) {
		
		inlierCount = 0;

		// First point
		while (true) {
			PointOne[0] = rand() % depthToColorImage.cols;
			PointOne[1] = rand() % depthToColorImage.rows;
			if (PointOne[0] < IgnoreLeftAndRightPixel && PointOne[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointOne[0] > 860 && PointOne[0] < 1060) {
				continue;
			}
			QVector3D vector3D_1 = KinectEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
			if (vector3D_1.x() == 0.0f && vector3D_1.y() == 0.0f && vector3D_1.z() == 0.0f) {
				continue;
			}
			else {
				break;
			}
		}

		// Second point
		while (true) {
			PointTwo[0] = rand() % depthToColorImage.cols;
			PointTwo[1] = rand() % depthToColorImage.rows;
			if (PointTwo[0] < IgnoreLeftAndRightPixel && PointTwo[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointTwo[0] > 860 && PointTwo[0] < 1060) {
				continue;
			}
			QVector3D vector3D_2 = KinectEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
			if (vector3D_2.x() == 0.0f && vector3D_2.y() == 0.0f && vector3D_2.z() == 0.0f) {
				continue;
			}
			if (sqrt(pow(PointTwo[0] - PointOne[0], 2) + pow(PointTwo[1] - PointOne[1], 2) * 1.0) <= 100) {
				continue;
			}
			break;
		}

		// Third point
		while (true) {
			PointThree[0] = rand() % depthToColorImage.cols;
			PointThree[1] = rand() % depthToColorImage.rows;
			if (PointThree[0] < IgnoreLeftAndRightPixel && PointThree[0] > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
				continue;
			}
			if (PointThree[0] > 860 && PointThree[0] < 1060) {
				continue;
			}
			QVector3D vector3D_3 = KinectEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);
			if (vector3D_3.x() == 0.0f && vector3D_3.y() == 0.0f && vector3D_3.z() == 0.0f) {
				continue;
			}
			if (sqrt(pow(PointThree[0] - PointOne[0], 2) + pow(PointThree[1] - PointOne[1], 2) * 1.0) <= 100) {
				continue;
			}
			if (sqrt(pow(PointThree[0] - PointTwo[0], 2) + pow(PointThree[1] - PointTwo[1], 2) * 1.0) <= 100) {
				continue;
			}
			break;
		}

		QVector3D vector3D_1 = KinectEngine::getInstance().query3DPoint(PointOne[0], PointOne[1], depthToColorImage);
		QVector3D vector3D_2 = KinectEngine::getInstance().query3DPoint(PointTwo[0], PointTwo[1], depthToColorImage);
		QVector3D vector3D_3 = KinectEngine::getInstance().query3DPoint(PointThree[0], PointThree[1], depthToColorImage);

		float* abcd;
		abcd = KinectEngine::getInstance().findPlaneEquationCoefficients(
			vector3D_1.x(), vector3D_1.y(), vector3D_1.z(),
			vector3D_2.x(), vector3D_2.y(), vector3D_2.z(),
			vector3D_3.x(), vector3D_3.y(), vector3D_3.z()
		);
		a = abcd[0];
		b = abcd[1];
		c = abcd[2];
		d = abcd[3];
		qDebug() << "Equation of plane is " << a << " x + " << b
			<< " y + " << c << " z + " << d << " = 0.";
	
		for (int y = 0; y < depthToColorImage.rows; y+=2) {
			for (int x = 0; x < depthToColorImage.cols; x+=2) {
				if (x > 860 && x < 1060) {
					continue;
				}
				if (x < IgnoreLeftAndRightPixel || x > depthToColorImage.cols - IgnoreLeftAndRightPixel) {
					continue;
				}
				QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, depthToColorImage);
				if (vector3D.x() == 0.0f && vector3D.y() == 0.0f && vector3D.z() == 0.0f) {
					continue;
				}

				float distance = KinectEngine::getInstance().findDistanceBetween3DPointAndPlane(vector3D.x(), vector3D.y(), vector3D.z(), a, b, c, d);
				if (distance <= threshold) {
					inlierCount++;
				}
			}
		}

		if (inlierCount > MaxInlierCount) {
			PlaneA = a;
			PlaneB = b;
			PlaneC = c;
			PlaneD = d;
			BestPointOne[0] = PointOne[0];
			BestPointOne[1] = PointOne[1];
			BestPointTwo[0] = PointTwo[0];
			BestPointTwo[1] = PointTwo[1];
			BestPointThree[0] = PointThree[0];
			BestPointThree[1] = PointThree[1];

			MaxInlierCount = inlierCount;
		}

		iterationCount++;
		qDebug() << "Inliers: " << inlierCount;
	
	}
	qDebug() << "Max Inliers: " << MaxInlierCount;

	// computer actual image
	cv::Mat out = cv::Mat::zeros(depthToColorImage.rows, depthToColorImage.cols, CV_16UC1);
	float maxDistance = 0.0f;
	for (int y = 0; y < depthToColorImage.rows; y++) {
		for (int x = 0; x < depthToColorImage.cols; x++) {
			
			QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, depthToColorImage);
			
			if (vector3D.x() == 0.0f && vector3D.y() == 0.0f && vector3D.z() == 0.0f) {
				out.at<uint16_t>(y, x) = 0.0f;
				continue;
			}

			float distance = KinectEngine::getInstance().findDistanceBetween3DPointAndPlane(vector3D.x(), vector3D.y(), vector3D.z(), PlaneA, PlaneB, PlaneC, PlaneD);
			out.at<uint16_t>(y, x) = distance;
			/*if (distance <= threshold) {
				out.at<uint16_t>(y, x) = 5000;
			}*/
			if (distance > maxDistance) {
				maxDistance = distance;
			}
		}
	}
	//out.at<uint16_t>(BestPointOne[1], BestPointOne[0]) = 5000;
	//out.at<uint16_t>(BestPointTwo[1], BestPointTwo[0]) = 5000;
	//out.at<uint16_t>(BestPointThree[1], BestPointThree[0]) = 5000;

	return out;

}

void CaptureTab::displayCapturedImages() {
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
	QGraphicsScene* scene = new ClipGraphicsScene();
	scene->addItem(item);

	this->parent->ui.graphicsViewImage->setScene(scene);
	this->parent->ui.graphicsViewImage->show();
}

void CaptureTab::onSlotRowSelected(const QModelIndex& current, const QModelIndex& previous) {
	
	if (this->isUploading) {
		return;
	}
	
	int row = current.row();

	/** Select image table view update UI to green background, showing successful image analysis */
	imageBeingAnalyzedTableViewRow = row;
	/** Select image table view update UI to green background, showing successful image analysis END */

	QModelIndex curIndex = dataModel->index(row, 1);

	selectedImageIndex = dataModel->data(curIndex).toInt();
	qDebug() << "Selected image index is" << selectedImageIndex;

	CaptureHistory captureHistory = captureHistories[selectedImageIndex];
	imageType = captureHistory.imageType;
	imageName = captureHistory.imageName;
	capturedColorImage = captureHistory.capturedColorImage;
	capturedDepthImage = captureHistory.capturedDepthImage;
	capturedColorToDepthImage = captureHistory.capturedColorToDepthImage;
	capturedDepthToColorImage = captureHistory.capturedDepthToColorImage;
	RANSACImage = captureHistory.RANSACImage;
	qColorImage = captureHistory.qColorImage;
	qDepthImage = captureHistory.qDepthImage;
	qColorToDepthImage = captureHistory.qColorToDepthImage;
	qDepthToColorImage = captureHistory.qDepthToColorImage;
	qDepthToColorColorizedImage = captureHistory.qDepthToColorColorizedImage;

	displayCapturedImages();
}
