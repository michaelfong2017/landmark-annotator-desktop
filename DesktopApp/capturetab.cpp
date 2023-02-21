#include "capturetab.h"
#include "saveimagedialog.h"
#include "kinectengine.h"
#include "realsenseengine.h"
#include <Windows.h>
#include "uploadrequest.h"
#include "types.h"

const int COLUMN_COUNT = 5;

CaptureTab::CaptureTab(DesktopApp* parent)
{
	this->parent = parent;
	this->recorder = new Recorder(parent);
	this->parent->ui.recordingIndicatorText->setVisible(false);
	this->parent->ui.recordingElapsedTime->setVisible(false);
	this->parent->ui.progressBar->setValue(0);
	this->parent->ui.progressBar->setTextVisible(false);

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
	setHeaders();

	/** This must be put here (below) */
	tableView->setColumnWidth(0, 18);
	tableView->setColumnWidth(2, 150);
	tableView->setColumnWidth(3, 120);
	//tableView->setColumnWidth(3, tableView->width() - 150 - 50);
	tableView->horizontalHeader()->setStretchLastSection(true);
	/** This must be put here (below) END */
	/** Headers END */

	tableView->hideColumn(1);

	tableView->show();

	this->uploadProgressDialog = new UploadProgressDialog;

	QObject::connect(this->parent->ui.saveButtonCaptureTab, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}
		SaveImageDialog dialog(this, false, this->hasPointCloud);
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
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTLISTTAB, true);
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTTAB, true);
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, true);
			/** Re-enable changing tab END */

			// Modify UI to disable recording status
			this->parent->ui.recordingIndicatorText->setVisible(false);
			this->parent->ui.recordingElapsedTime->setVisible(false);
			this->parent->ui.captureTab->setStyleSheet("");

			this->recorder->stopRecorder();
			this->parent->ui.saveVideoButton->setText(tr("Record"));

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
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTLISTTAB, false);
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTTAB, false);
			this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, false);
			/** Disable changing tab END */

			// Modify UI to indicate recording status
			this->parent->ui.recordingIndicatorText->setVisible(true);
			this->parent->ui.recordingElapsedTime->setVisible(true);
			this->parent->ui.captureTab->setStyleSheet("#captureTab {border: 2px solid red}");

			this->recorder->prepareRecorder();
			this->parent->ui.saveVideoButton->setText(tr("Stop"));

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

	QObject::connect(this->parent->ui.importButton, &QPushButton::clicked, [this]() {
		qDebug() << "Import button clicked";

		QString patientFolderPath = this->getParent()->savePath.absolutePath();

		// After realsense camera is opened, native file dialog will freeze the application
		// and therefore cannot be used. QFileDialog::DontUseNativeDialog is thus needed.
		// See https://stackoverflow.com/questions/31983412/code-freezes-on-trying-to-open-qdialog
		QString chosenColorSavePath = QFileDialog::getOpenFileName(this, tr("Select Color Image"),
			patientFolderPath,
			tr("Images (*.png *.jpg)"), 0, QFileDialog::DontUseNativeDialog);

		QString chosenFolder = QFileInfo(chosenColorSavePath).absoluteDir().absolutePath();
		QString chosenFilename = QUrl(chosenColorSavePath).fileName();

		qDebug() << "chosenFolder: " << chosenFolder;
		qDebug() << "chosenFilename: " << chosenFilename;

		if (chosenColorSavePath.split("_").length() < 2) {
			qWarning() << "capturetab importButton - The selected color image does not have a filename matching the expected format (with correct timestamp prefix)";
			return;
		}

		QString filenamePrefix = chosenFilename.split("_")[0] + "_" + chosenFilename.split("_")[1];
	
		QString colorSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_color.png");
		QString depthSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_depth.png");
		QString colorToDepthSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_color_aligned.png");
		QString depthToColorSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_depth_aligned.png");
		QString fourChannelPNGSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_four_channel.png");
		QString pointCloudPNGSavePath = QDir(chosenFolder).filePath(filenamePrefix + "_point_cloud.png");

		this->creationTime = filenamePrefix;

		this->capturedColorImage = KinectEngine::getInstance().readCVImageFromFile(colorSavePath.toStdWString());
		this->capturedDepthImage = KinectEngine::getInstance().readCVImageFromFile(depthSavePath.toStdWString());
		this->capturedColorToDepthImage = KinectEngine::getInstance().readCVImageFromFile(colorToDepthSavePath.toStdWString());
		this->capturedDepthToColorImage = KinectEngine::getInstance().readCVImageFromFile(depthToColorSavePath.toStdWString());
		this->FourChannelPNG = KinectEngine::getInstance().readCVImageFromFile(fourChannelPNGSavePath.toStdWString());
		this->PointCloudPNG = KinectEngine::getInstance().readCVImageFromFile(pointCloudPNGSavePath.toStdWString());

		if (this->PointCloudPNG.dims == 0) {
			//this->PointCloudPNG = cv::Mat::ones(1080, 1920, CV_16SC4);
			this->PointCloudPNG = cv::Mat::ones(720, 1280, CV_16SC4);
			this->hasPointCloud = false;
		}
		else {
			this->hasPointCloud = true;
		}

		cv::cvtColor(this->capturedColorImage, this->capturedColorImage, cv::COLOR_BGR2BGRA);
		this->capturedDepthImage.convertTo(this->capturedDepthImage, CV_16UC1);
		cv::cvtColor(this->capturedColorToDepthImage, this->capturedColorToDepthImage, cv::COLOR_BGR2BGRA);
		this->capturedDepthToColorImage.convertTo(this->capturedDepthToColorImage, CV_16UC1);
		cv::cvtColor(this->FourChannelPNG, this->FourChannelPNG, cv::COLOR_BGR2BGRA);
		this->PointCloudPNG.convertTo(this->PointCloudPNG, CV_16SC4);


		afterSensorImagesAcquired();


		/** Import does not need auto-save */
		SaveImageDialog dialog(this, false, this->hasPointCloud);
		/** Import does not need auto-save END */
		});

	QObject::connect(this->parent->ui.captureButton, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}

		/** UI */
		disableButtonsForUploading();
		this->parent->ui.progressBar->setValue(50);
		/** UI END */

		this->imageName = this->parent->ui.imageTypeComboBox->currentText();
		this->imageType = getImageTypeFromDescription(this->imageName);
		RealsenseEngine::getInstance().readAllImages(this->capturedColorImage, this->capturedDepthImage, this->capturedColorToDepthImage, this->capturedDepthToColorImage);
		
		// Shallow copy
		/*cv::Mat color = this->capturedColorImage;
		cv::Mat depth = this->capturedDepthImage;
		cv::Mat colorToDepth = this->capturedColorToDepthImage;
		cv::Mat depthToColor = this->capturedDepthToColorImage;*/
		//

		/** Assume that capture is all successful, otherwise print a warning. */
		if (this->capturedColorImage.empty() || this->capturedDepthImage.empty() || this->capturedColorToDepthImage.empty() || this->capturedDepthToColorImage.empty()) {
			qWarning() << "capturetab captureButton - one of the captured images is null";
			TwoLinesDialog dialog;
			dialog.setLine1("Capture failed!");
			dialog.exec();
			return;
		}

		
		// Point Cloud 
		if (this->parent->ui.enablePointCloudcheckBox->isChecked()) {
			qDebug() << "High Resolution checked. get Point Cloud";
			this->PointCloudPNG = computePointCloudFromDepth();
			this->hasPointCloud = true;
		}
		else {
			//this->PointCloudPNG = cv::Mat::ones(1080, 1920, CV_16SC4);
			this->PointCloudPNG = cv::Mat::ones(720, 1280, CV_16SC4);
			this->hasPointCloud = false;
		}

		RealsenseEngine::getInstance().computeNormalizedDepthImage(this->capturedDepthToColorImage, this->RANSACImage);

		this->parent->ui.saveButtonCaptureTab->setEnabled(true);
		this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
		this->noImageCaptured = false;

		/* Cropping all images to 800 * 1080 START*/
		/* Cropping all images to 534 * 720 START*/
		//float widthOfPatientBack = 800;
		float widthOfPatientBack = 534;
		//cv::Rect rect((COLOR_IMAGE_WIDTH / 2) - (widthOfPatientBack / 2), 0, widthOfPatientBack, 1080);
		cv::Rect rect((COLOR_IMAGE_WIDTH_REALSENSE / 2) - (widthOfPatientBack / 2), 0, widthOfPatientBack, 720);
		this->capturedColorImage = this->capturedColorImage(rect);
		this->capturedDepthToColorImage = this->capturedDepthToColorImage(rect);
		this->RANSACImage = this->RANSACImage(rect);
		this->PointCloudPNG = this->PointCloudPNG(rect);
		/* Cropping all images to 800 * 1080 END*/

		// For Debugging
		/*this->RANSACImage.convertTo(this->RANSACImage, CV_8U, 255.0 / 5000.0, 0.0);
		cv::imshow("ransac", this->RANSACImage);
		cv::waitKey(0);
		cv::destroyWindow("ransac");*/

		/* Convert to the special 4 channels image and upload START*/
		cv::Mat color3 = this->capturedColorImage;
		std::vector<cv::Mat>channelsForColor2(3);
		cv::split(color3, channelsForColor2);

		cv::Mat depthToColor3 = this->capturedDepthToColorImage;
		std::vector<cv::Mat>channelsForDepth1(1);
		cv::split(depthToColor3, channelsForDepth1);

		cv::Mat normalizedDepthToColor = this->RANSACImage;
		std::vector<cv::Mat>channelsForDepth2(1);
		cv::split(normalizedDepthToColor, channelsForDepth2);

		int w = this->capturedColorImage.cols;
		int h = this->capturedColorImage.rows;

		FourChannelPNG = cv::Mat::ones(h, w, CV_16UC4);
		std::vector<cv::Mat>channels3(4);
		cv::split(FourChannelPNG, channels3);

		qDebug() << "this->capturedColorImage" << this->capturedColorImage.cols;
		qDebug() << "depthToColor3" << depthToColor3.cols;
		qDebug() << "normalizedDepthToColor" << normalizedDepthToColor.cols;

		// channelsForColor2 = BGR
		for (int i = 0; i < w * h; i++) {
			channels3[0].at<uint16_t>(i) = (channelsForColor2[2].at<uint8_t>(i) << 8) | channelsForColor2[1].at<uint8_t>(i);
			channels3[1].at<uint16_t>(i) = (channelsForColor2[0].at<uint8_t>(i) << 8);
		}
		channels3[2] = channelsForDepth1[0];
		channels3[3] = channelsForDepth2[0];

		cv::merge(channels3, FourChannelPNG);

		qDebug() << "Merging FourChannelPNG completed: "
			<< FourChannelPNG.cols << ", "
			<< FourChannelPNG.rows << ", "
			<< FourChannelPNG.channels();
		/* Convert to the special 4 channels image and upload END */

		this->creationTime = Helper::getCurrentDateTimeString();

		afterSensorImagesAcquired();

		/** Import does not need auto-save */
		SaveImageDialog dialog(this, true, this->hasPointCloud);
		/** Import does not need auto-save END */
	});

	QObject::connect(this->parent->ui.annotateButtonCaptureTab, &QPushButton::clicked, [this]() {
		if (this->isUploading) {
			return;
		}
		qDebug() << "Analysis button clicked";

		if (this->capturedColorImage.empty()) {
			return;
		}

		//this->isUploading = true;
		
		//this->disableButtonsForUploading();

		/** Select image table view update UI to green background, showing successful image analysis */
		storedTableViewRow = imageBeingAnalyzedTableViewRow;
		/** Select image table view update UI to green background, showing successful image analysis END */
		/** Store the image type of the image being analyzed */
		imageTypeBeingAnalyzed = imageType;
		/** Store the image type of the image being analyzed END */

		/** Cropping part 2 */
		/*float cropUIToImageScale = (float)this->getCapturedColorImage().rows / this->parent->ui.graphicsViewImage->sceneRect().height();
		cv::Rect rect(this->clip_rect.left() * cropUIToImageScale, this->clip_rect.top() * cropUIToImageScale, this->clip_rect.width() * cropUIToImageScale, this->clip_rect.height() * cropUIToImageScale);

		if (this->clip_rect.width() == this->max_clip_width) {
			rect.width = this->getCapturedColorImage().cols;
		}
		if (this->clip_rect.width() == this->max_clip_width) {
			rect.height = this->getCapturedColorImage().rows;
		}

		qDebug() << "croppppp original (width, height): (" << this->getCapturedColorImage().cols << ", " << this->getCapturedColorImage().rows << ")";
		qDebug() << "croppppp cropUIToImageScale: " << cropUIToImageScale;
		qDebug() << "croppppp rect(x, y, width, height): (" << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height << ")";

		this->capturedColorImage = this->capturedColorImage(rect);
		this->capturedDepthToColorImage = this->capturedDepthToColorImage(rect);
		this->RANSACImage = this->RANSACImage(rect);
		this->cropRect = rect;*/
		/** Cropping part 2 END */

		

		int captureNumber = dataModel->rowCount() - imageBeingAnalyzedTableViewRow;
		int uploadNumber = ++uploadProgressDialog->latestUploadNumber;


		QDateTime dateTime = dateTime.currentDateTime();
		QString str = dateTime.toString();
		str.replace(" ", "");
		str.replace(":", "");

		/** Remove previously completed uploads */
		for (auto i = begin(this->uploadProgressDialog->completedUploadNumbers); i != end(this->uploadProgressDialog->completedUploadNumbers); ++i) {
			int completedUploadNumber = *i;

			std::map<int, std::unique_ptr<uploadrequest>>::iterator it = this->uploadProgressDialog->requests.find(completedUploadNumber);
			if (it != this->uploadProgressDialog->requests.end()) {
				std::unique_ptr<uploadrequest> req = std::move(it->second);
				this->uploadProgressDialog->requests.erase(it);
			}
		}
		this->uploadProgressDialog->completedUploadNumbers.clear();
		/** Remove previously completed uploads END */

		std::unique_ptr<uploadrequest> req = std::make_unique<uploadrequest>(QNetworkClient::getInstance().userToken, "", this->parent->patientTab->getCurrentPatientId(),
			imageType, str, this->FourChannelPNG, this->capturedColorImage, this->PointCloudPNG,
			uploadNumber, captureNumber, this->parent->patientTab->getCurrentPatientName(), this->uploadProgressDialog);
		this->uploadProgressDialog->requests.insert(std::make_pair(uploadNumber, std::move(req)));
		
		std::map<int, std::unique_ptr<uploadrequest>>::iterator it = this->uploadProgressDialog->requests.find(uploadNumber);
		it->second.get()->start();

		/*new uploadrequest(QNetworkClient::getInstance().userToken, "", this->parent->patientTab->getCurrentPatientId(), 
			imageType, imageName, FourChannelPNG, 
			uploadNumber, this->parent->patientTab->getCurrentPatientName(), this->uploadProgressDialog)*/;

		//this->uploadProgressDialog->requests.push_back(r);
		//QNetworkClient::getInstance().uploadImage(FourChannelPNG, this, SLOT(onUploadImage(QNetworkReply*)));
		/* Convert to the special 4 channels image and upload END */

		// Moving to annotate tab will be done after the series of requests is sent to obtain the landmark predictions */
	});

	QObject::connect(timer, &QTimer::timeout, [this]() {
		//qDebug() << "timer connect start: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
		//if (!KinectEngine::getInstance().isDeviceOpened()) {
		//	return;
		//}

		//KinectEngine::getInstance().captureImages();
		//cv::Mat color, depth;
		//KinectEngine::getInstance().readColorAndDepthImages(color, depth);


		//if (!RealsenseEngine::getInstance().isDeviceOpened()) {
		//	return;
		//}

		RealsenseEngine::getInstance().captureImages();
		cv::Mat color, depth;
		RealsenseEngine::getInstance().readColorAndDepthImages(color, depth);


		int cropPerSide = KinectEngine::getInstance().COLOR_IMAGE_CROP_WIDTH_PER_SIDE;

		
		QImage qColor = convertColorCVToQImage(color);

		// Crop left and right
		qColor = qColor.copy(cropPerSide, 0, COLOR_IMAGE_WIDTH_REALSENSE - 2 * cropPerSide, COLOR_IMAGE_HEIGHT_REALSENSE);
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
			this->imageName = this->parent->ui.imageTypeComboBox->currentText();
			this->imageType = getImageTypeFromDescription(this->imageName);
			QString humanFilepath = "";
			switch (this->imageType) {
			case 7:
				humanFilepath = ":/DesktopApp/resources/HumanCutShape4.png";
				break;
			case 8:
				humanFilepath = ":/DesktopApp/resources/LeftHumanShape2.png";
				break;
			case 9:
				humanFilepath = ":/DesktopApp/resources/RightHumanShape2.png";
				break;
			}

			if (this->imageType >= 7 && this->imageType <= 9) {
				QPixmap humanPixmap(humanFilepath);
				QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);

				// Crop left and right
				// Don't crop human cut shape if the original image also does not need the crop
				if (KinectEngine::getInstance().COLOR_IMAGE_CROP_WIDTH_PER_SIDE != 0) {
					cropPerSide = (humanPixmapScaled.width() - humanPixmapScaled.height()) / 2;
					humanPixmapScaled = humanPixmapScaled.copy(cropPerSide, 0, humanPixmapScaled.width() - 2 * cropPerSide, humanPixmapScaled.height());
				}
				// Crop left and right END

				scene->addPixmap(humanPixmapScaled);
			}
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
		//bool queueIMUSuccess = KinectEngine::getInstance().queueIMUSample();
		//std::deque<point3D> gyroSampleQueue = KinectEngine::getInstance().getGyroSampleQueue();
		//std::deque<point3D> accSampleQueue = KinectEngine::getInstance().getAccSampleQueue();


		bool queueIMUSuccess = RealsenseEngine::getInstance().queueIMUSample();
		std::deque<point3D> gyroSampleQueue = RealsenseEngine::getInstance().getGyroSampleQueue();
		std::deque<point3D> accSampleQueue = RealsenseEngine::getInstance().getAccSampleQueue();


		if (queueIMUSuccess && !gyroSampleQueue.empty() && !accSampleQueue.empty()) {
			//qDebug() << "timer connect 17: " << QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
			/** Alert if gyroscope and accelerometer show that the kinect sensor is being moved */
			alertIfMoving(
				gyroSampleQueue[gyroSampleQueue.size() - 1].x,
				gyroSampleQueue[gyroSampleQueue.size() - 1].y,
				gyroSampleQueue[gyroSampleQueue.size() - 1].z,
				accSampleQueue[accSampleQueue.size() - 1].x,
				accSampleQueue[accSampleQueue.size() - 1].y,
				accSampleQueue[accSampleQueue.size() - 1].z
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

void CaptureTab::afterSensorImagesAcquired() {
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
			text = this->hasPointCloud ? "Yes" : "No";
			break;
		case 4:
			//QDateTime dateTime = dateTime.currentDateTime();
			//text = dateTime.toString("yyyy-MM-dd HH:mm:ss");
			text = Helper::dateTimeFilepathToDisplay(this->creationTime);
			break;
		}
		dataItem = new QStandardItem(text);
		QFont fn = dataItem->font();
		fn.setPixelSize(14);
		dataItem->setFont(fn);
		itemList << dataItem;
	}

	dataModel->insertRow(0, itemList);

	imageBeingAnalyzedTableViewRow = 0;
	/** Insert index to data model END */

	this->qColorImage = convertColorCVToQImage(this->capturedColorImage);
	this->qDepthImage = convertDepthCVToQImage(this->capturedDepthImage);
	this->qColorToDepthImage = convertColorToDepthCVToQImage(this->capturedColorToDepthImage);
	this->qDepthToColorImage = convertDepthToColorCVToQImage(this->capturedDepthToColorImage);

	/** Initialize clip_rect whenever a new image is captured */
	int width = this->parent->ui.graphicsViewImage->width();
	int height = this->parent->ui.graphicsViewImage->height();
	QImage imageScaled = qColorImage.scaled(width, height, Qt::KeepAspectRatio);
	max_clip_width = imageScaled.width();
	max_clip_height = imageScaled.height();
	clip_rect = QRect(0, 0, max_clip_width, max_clip_height);
	/** Initialize clip_rect whenever a new image is captured */

	/** Store histories of images for selection */
	CaptureHistory captureHistory;
	captureHistory.imageType = imageType;
	captureHistory.imageName = imageName;
	captureHistory.capturedColorImage = capturedColorImage;
	captureHistory.capturedDepthImage = capturedDepthImage;
	captureHistory.capturedColorToDepthImage = capturedColorToDepthImage;
	captureHistory.capturedDepthToColorImage = capturedDepthToColorImage;
	captureHistory.PointCloudPNG = PointCloudPNG;
	captureHistory.hasPointCloud = hasPointCloud;
	captureHistory.FourChannelPNG = FourChannelPNG;
	captureHistory.RANSACImage = RANSACImage;
	captureHistory.qColorImage = qColorImage;
	captureHistory.qDepthImage = qDepthImage;
	captureHistory.qColorToDepthImage = qColorToDepthImage;
	captureHistory.qDepthToColorImage = qDepthToColorImage;
	captureHistory.clip_rect = clip_rect;
	captureHistories.push_back(captureHistory);

	selectedImageIndex = captureHistories.size() - 1;
	/** Store histories of images for selection END */
	displayCapturedImages();

	/** UI */
	enableButtonsForUploading();
	/** UI END */
}

void CaptureTab::clearCaptureHistories() {
	qDebug() << "Clear records: " << captureHistories.size();
	for (int i = 0; i < captureHistories.size(); i++) {
		dataModel->removeRow(0, QModelIndex());
	}

	captureHistories.clear();

	/** Reset variables stored in CaptureTab */
	imageType = -1; // Update on image selection
	imageName = "";
	capturedColorImage.release();
	capturedDepthImage.release();
	capturedColorToDepthImage.release();
	capturedDepthToColorImage.release();
	qColorImage = QImage();
	qDepthImage = QImage();
	qColorToDepthImage = QImage();
	qDepthToColorImage = QImage();
	RANSACImage.release();
	//PointCloudPNG.release();
	hasPointCloud = false;
	imageTypeBeingAnalyzed = -1; // Update on analysis button pressed
	/** Reset variables stored in CaptureTab END */

	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->parent->ui.graphicsViewImage->scene()) {
		delete this->parent->ui.graphicsViewImage->scene();
	}
	this->parent->ui.patientNameInCapture->setText(tr("Current Patient: ") + this->parent->patientTab->getCurrentPatientName());

}

std::vector<CaptureHistory> CaptureTab::getCaptureHistories()
{
	return this->captureHistories;
}

void CaptureTab::setCaptureHistories(int selectedImageIndex, CaptureHistory captureHistory)
{
	this->captureHistories[selectedImageIndex] = captureHistory;
}

int CaptureTab::getSelectedImageIndex()
{
	return this->selectedImageIndex;
}

QRect CaptureTab::corner(int number)
{
	switch (number) {
	case 0:
		return QRect(clip_rect.topLeft() + std::get<0>(handle_offsets), QSize(8, 8));
	case 1:
		return QRect(clip_rect.topRight() + std::get<1>(handle_offsets), QSize(8, 8));
	case 2:
		return QRect(clip_rect.bottomLeft() + std::get<2>(handle_offsets), QSize(8, 8));
	case 3:
		return QRect(clip_rect.bottomRight() + std::get<3>(handle_offsets), QSize(8, 8));
	default:
		return QRect();
	}
}

void CaptureTab::setDefaultCaptureMode() {
	parent->ui.radioButton->setChecked(true);
	parent->ui.radioButton2->setChecked(false);
	parent->ui.radioButton3->setChecked(false);
	parent->ui.radioButton4->setChecked(false);
}

void CaptureTab::disableButtonsForUploading() {

	this->parent->ui.progressBar->setTextVisible(true);
	this->parent->ui.progressBar->setValue(1);

	this->parent->ui.importButton->setEnabled(false);
	this->parent->ui.captureButton->setEnabled(false);
	this->parent->ui.saveVideoButton->setEnabled(false);
	this->parent->ui.saveButtonCaptureTab->setEnabled(false);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(false);
	this->parent->ui.radioButton->setEnabled(false);
	this->parent->ui.radioButton2->setEnabled(false);
	this->parent->ui.radioButton3->setEnabled(false);
	this->parent->ui.radioButton4->setEnabled(false);

	this->parent->ui.logOutButton3->setEnabled(false);

	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTLISTTAB, false);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTTAB, false);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, false);
	
	/** Disable table view row selection */
	tableView->setSelectionMode(QAbstractItemView::NoSelection);
	/** Disable table view row selection END */
}

void CaptureTab::enableButtonsForUploading() {

	this->parent->ui.progressBar->setValue(0);
	this->parent->ui.progressBar->setTextVisible(false);

	this->parent->ui.importButton->setEnabled(true);
	this->parent->ui.captureButton->setEnabled(true);
	this->parent->ui.saveVideoButton->setEnabled(true);
	this->parent->ui.saveButtonCaptureTab->setEnabled(true);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
	this->parent->ui.radioButton->setEnabled(true);
	this->parent->ui.radioButton2->setEnabled(true);
	this->parent->ui.radioButton3->setEnabled(true);
	this->parent->ui.radioButton4->setEnabled(true);

	this->parent->ui.logOutButton3->setEnabled(true);

	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTLISTTAB, true);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTTAB, true);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, true);

	/** Enable table view row selection */
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	/** Enable table view row selection END */
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

		QGraphicsPixmapItem* pixmapItem = new ClipGraphicsPixmapItem(QPixmap::fromImage(imageScaled), this);
		QGraphicsScene* scene = new ClipGraphicsScene(this, pixmapItem);

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
			//dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
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
			//dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
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
	QString landmarks = obj["aiOriginResult"].toString();

	/** TIMEOUT */
	if (response_data == nullptr) {

		/** Select image table view update UI to red background, showing unsuccessful image analysis */
		for (int i = 0; i < dataModel->columnCount(); i++) {
			QModelIndex index;
			index = dataModel->index(storedTableViewRow, i);
			//dataModel->setData(index, QColor(Qt::red), Qt::BackgroundRole);
		}
		/** Select image table view update UI to red background, showing unsuccessful image analysis END */

		TwoLinesDialog dialog;
		dialog.setLine1("Analysis Step 3 Timeout!");
		dialog.exec();

		this->enableButtonsForUploading();
		this->isUploading = false;

		return;
	}
	/** TIMEOUT END */

	if (landmarks == "") {

		qDebug() << "No landmarks";

		/** For sending findLandmarkPredictions() more than once */
		if (landmarkRequestSent < MAX_LANDMARK_REQUEST_SENT) {
			Sleep(2500);
			landmarkRequestSent++;
			QNetworkClient::getInstance().findLandmarkPredictions(this->currentImageId, this, SLOT(onFindLandmarkPredictions(QNetworkReply*)));
			return;
		}
		/** For sending findLandmarkPredictions() more than once END */
	}

	int imageId = obj["id"].toInt();
	//this->parent->annotateTab->setAiImageUrl(aiImageUrl);

	QString aiOriginResult = obj["aiOriginResult"].toString();

	qDebug() << "imageId:" << imageId;
	qDebug() << "aiOriginResult:" << aiOriginResult;

	// hard code version
	//if (aiOriginResult == "") {
	//	switch (COLOR_IMAGE_WIDTH) {
	//	case 1920:
	//		//aiOriginResult = "[[960.0, 300.0], [1050.0, 450.0], [870.0, 450.0], [1050.0, 750.0], [870.0, 750.0], [960.0, 900.0]]";
	//		// Cropped version. 800 Width
	//		aiOriginResult = "[[400.0, 300.0], [500.0, 450.0], [300.0, 450.0], [500.0, 750.0], [300.0, 750.0], [400.0, 900.0]]";
	//		break;
	//	case 1280:
	//		aiOriginResult = "[[640.0, 200.0], [700.0, 300.0], [580.0, 300.0], [700.0, 500.0], [580.0, 500.0], [640.0, 600.0]]";
	//		break;
	//	}
	//}

	if (aiOriginResult == "") {
		int w = this->capturedColorImage.cols;
		int h = this->capturedColorImage.rows;
		qDebug() << w << h;
		std::string PtC = "[" + std::to_string(w / 2) + ", " + std::to_string(h / 5) + "], ";
		std::string PtA2 = "[" + std::to_string(2 * w / 3) + ", " + std::to_string(2 * h / 5) + "], ";
		std::string PtA1 = "[" + std::to_string(w / 3) + ", " + std::to_string(2 * h / 5) + "], ";
		std::string PtB2 = "[" + std::to_string(2 * w / 3) + ", " + std::to_string(3 * h / 5) + "], ";
		std::string PtB1 = "[" + std::to_string(w / 3) + ", " + std::to_string(3 * h / 5) + "], ";
		std::string PtD = "[" + std::to_string(w / 2) + ", " + std::to_string(4 * h / 5) + "], ";
		std::string complete = "[" + PtC + PtA2 + PtA1 + PtB2 + PtB1 + PtD + "]";
		aiOriginResult = QString::fromStdString(complete);
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
		//dataModel->setData(index, QColor(Qt::green), Qt::BackgroundRole);
	}
	/** Select image table view update UI to green background, showing successful image analysis END */

	this->enableButtonsForUploading();
	this->isUploading = false;

	// Move to annotate tab which index is 4
	this->parent->annotateTab->reloadCurrentImage(getQColorImage(), getCapturedDepthToColorImage());
	//this->parent->ui.tabWidget->setCurrentIndex(TabIndex::ANNOTATETAB);
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

cv::Mat CaptureTab::getPointCloudImage()
{
	return this->PointCloudPNG;
}

cv::Mat CaptureTab::getFourChannelPNG()
{
	return this->FourChannelPNG;
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


int CaptureTab::getCaptureCount() { return this->captureCount; }

void CaptureTab::setCaptureCount(int newCaptureCount) { this->captureCount = newCaptureCount; }

Recorder* CaptureTab::getRecorder() { return this->recorder; }

QString CaptureTab::getCaptureFilepath() { return this->captureFilepath; }
void CaptureTab::setCaptureFilepath(QString captureFilepath) { this->captureFilepath = captureFilepath; }

cv::Mat CaptureTab::computePointCloudFromDepth() {

	cv::Mat temp;
	RealsenseEngine::getInstance().readPointCloudImage(temp);
	temp.convertTo(temp, CV_16UC4);
	return temp; // For Realsense, return early

	//KinectEngine::getInstance().readPointCloudImage(temp);

	/** Convert 16UC3 to 16SC4 with alpha=1 */
	cv::Mat pointCloudImage16UC4 = cv::Mat::ones(temp.rows, temp.cols, CV_16UC4);
	std::vector<cv::Mat>channels16C4(4);
	std::vector<cv::Mat>channels16C3(3);
	cv::split(pointCloudImage16UC4, channels16C4);
	cv::split(temp, channels16C3);
	channels16C4[0] = channels16C3[0];
	channels16C4[1] = channels16C3[1];
	channels16C4[2] = channels16C3[2];

	cv::merge(channels16C4, pointCloudImage16UC4);
	temp = pointCloudImage16UC4;

	//this->pointCloudImage = pointCloudImage16SC4
	/** Convert 16UC3 to 16UC4 with alpha=1 END */

	/** Save point cloud image for testing */
	/*QString dateTimeString = Helper::getCurrentDateTimeString();
	QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);

	QString	chosenFolder = QFileDialog::getExistingDirectory(this, tr("Select Output Folder"),
		visitFolderPath,
		QFileDialog::ShowDirsOnly);
	QString pointCloudSavePath = QDir(chosenFolder).filePath(QString::fromStdString(dateTimeString.toStdString() + "_point_cloud.png"));
	bool pointCloudWriteSuccess = false;

	QImageWriter writer(pointCloudSavePath);

	QImage img((uchar*)pointCloudImage16SC4.data,
		pointCloudImage16SC4.cols,
		pointCloudImage16SC4.rows,
		pointCloudImage16SC4.step,
		QImage::Format_RGBA64);
		pointCloudWriteSuccess = writer.write(img);*/
	/** Save point cloud image for testing END */

	return temp;
}

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
		/*qDebug() << "Equation of plane is " << a << " x + " << b
			<< " y + " << c << " z + " << d << " = 0.";*/
	
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
		//qDebug() << "Inliers: " << inlierCount;
	
	}
	//qDebug() << "Max Inliers: " << MaxInlierCount;

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
	this->max_clip_width = imageScaled.width();
	this->max_clip_height = imageScaled.height();
	this->clip_rect = QRect(0, 0, max_clip_width, max_clip_height);

	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->parent->ui.graphicsViewImage->scene()) {
		delete this->parent->ui.graphicsViewImage->scene();
	}

	QGraphicsPixmapItem* pixmapItem = new ClipGraphicsPixmapItem(QPixmap::fromImage(imageScaled), this);
	QGraphicsScene* scene = new ClipGraphicsScene(this, pixmapItem);

	this->parent->ui.graphicsViewImage->setScene(scene);
	this->parent->ui.graphicsViewImage->show();
}

int CaptureTab::getImageTypeFromDescription(QString description)
{
	int imageType = 7;
	if (description == backAnalysisString) {
		imageType = 7;
	}
	else if (description == leftSideString) {
		imageType = 8;
	}
	else if (description == rightSideString) {
		imageType = 9;
	}
	else if (description == otherString) {
		imageType = 10;
	}
	return imageType;
}

void CaptureTab::onEnterOfflineMode()
{
	qDebug() << "CaptureTab::onEnterOfflineMode()";
	this->parent->ui.importButton->setEnabled(false);
	this->parent->ui.captureButton->setEnabled(false);
	this->parent->ui.saveVideoButton->setEnabled(false);
	this->parent->ui.saveButtonCaptureTab->setEnabled(false);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(false);
	this->parent->ui.radioButton->setEnabled(false);
	this->parent->ui.radioButton2->setEnabled(false);
	this->parent->ui.radioButton3->setEnabled(false);
	this->parent->ui.radioButton4->setEnabled(false);

	this->parent->ui.imageTypeComboBox->setEnabled(false);

	this->parent->ui.enablePointCloudcheckBox->setEnabled(false);

	// Disable log out button in offline mode
	this->parent->ui.logOutButton3->setEnabled(false);
	////

	/** Disable table view row selection */
	tableView->setSelectionMode(QAbstractItemView::NoSelection);
	/** Disable table view row selection END */
}

void CaptureTab::onExitOfflineMode()
{
	qDebug() << "CaptureTab::onExitOfflineMode()";
	this->parent->ui.importButton->setEnabled(true);
	this->parent->ui.captureButton->setEnabled(true);
	this->parent->ui.saveVideoButton->setEnabled(true);
	this->parent->ui.saveButtonCaptureTab->setEnabled(true);
	this->parent->ui.annotateButtonCaptureTab->setEnabled(true);
	this->parent->ui.radioButton->setEnabled(true);
	this->parent->ui.radioButton2->setEnabled(true);
	this->parent->ui.radioButton3->setEnabled(true);
	this->parent->ui.radioButton4->setEnabled(true);

	this->parent->ui.imageTypeComboBox->setEnabled(true);

	this->parent->ui.enablePointCloudcheckBox->setEnabled(true);

	// Enable log out button in offline mode
	this->parent->ui.logOutButton3->setEnabled(true);
	////

	/** Enable table view row selection */
	tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	/** Enable table view row selection END */
}

void CaptureTab::onLanguageChanged()
{
	setHeaders();

	backAnalysisString = tr("Back analysis");
	leftSideString = tr("Left side");
	rightSideString = tr("Right side");
	otherString = tr("Other");

	this->parent->ui.patientNameInCapture->setText(tr("Current Patient: ") + this->parent->patientTab->getCurrentPatientName());

	if (this->recorder->getRecordingStatus()) {
		this->parent->ui.saveVideoButton->setText(tr("Stop"));
	}
	else {
		this->parent->ui.saveVideoButton->setText(tr("Record"));
	}
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
	FourChannelPNG = captureHistory.FourChannelPNG;
	PointCloudPNG = captureHistory.PointCloudPNG;
	hasPointCloud = captureHistory.hasPointCloud;
	RANSACImage = captureHistory.RANSACImage;
	qColorImage = captureHistory.qColorImage;
	qDepthImage = captureHistory.qDepthImage;
	qColorToDepthImage = captureHistory.qColorToDepthImage;
	qDepthToColorImage = captureHistory.qDepthToColorImage;
	clip_rect = captureHistory.clip_rect;

	displayCapturedImages();
}

void CaptureTab::setHeaders()
{
	QStringList headerLabels = { "", tr("index"), tr("Image Type"), tr("Has Point Cloud?"), tr("Creation Time") };

	for (int i = 0; i < COLUMN_COUNT; i++)
	{
		QString text = headerLabels.at(i);
		QStandardItem* item = new QStandardItem(text);
		QFont fn = item->font();
		fn.setPixelSize(14);
		item->setFont(fn);

		dataModel->setHorizontalHeaderItem(i, item);
	}
}
