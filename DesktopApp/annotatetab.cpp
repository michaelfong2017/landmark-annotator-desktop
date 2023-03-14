#include "annotatetab.h"
#include "draganddropgraphicsscene.h"
#include "kinectengine.h"
#include "realsenseengine.h"
#include "cameramanager.h"
#include <reportdialog.h>

QPointF getRandomPoint(int maxWidth, int maxHeight) {
	int randX = rand() % (maxWidth + 1);
	int randY = rand() % (maxHeight + 1);

	return QPointF((float)randX, (float)randY);
}

AnnotateTab::AnnotateTab(DesktopApp* parent) {
	this->parent = parent;

	this->colorScene = new DragAndDropGraphicsScene(this, ImageType::Color);
	this->depthToColorScene = new DragAndDropGraphicsScene(this, ImageType::DepthToColor);

	this->parent->ui.graphicsViewAnnotation->setScene(this->colorScene);
	this->parent->ui.graphicsViewAnnotation->show();

	this->parent->ui.graphicsViewAnnotation2->setScene(this->depthToColorScene);
	this->parent->ui.graphicsViewAnnotation2->show();

	QObject::connect(this->parent->ui.confirmLandmarksButton, &QPushButton::clicked, [this]() {
		qDebug() << "confirmLandmarksButton clicked";

		QString aiOriginResult = QString("[[");

		float x, y;
		QPointF second;

		// C
		second = this->annotationsOnRight.at("C");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// C END

		// A1
		second = this->annotationsOnRight.at("A1");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// A1 END

		// A2
		second = this->annotationsOnRight.at("A2");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// A2 END

		// B1
		second = this->annotationsOnRight.at("B1");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// B1 END

		// B2
		second = this->annotationsOnRight.at("B2");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// B2 END

		// D
		second = this->annotationsOnRight.at("D");
		x = second.x();
		y = second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "]]";
		// D END

		qDebug() << aiOriginResult;

		QNetworkClient::getInstance().confirmLandmarks(this->imageId, aiOriginResult, this, SLOT(onConfirmLandmarks(QNetworkReply*)));
		});

		QObject::connect(this->parent->ui.generateReportButton, &QPushButton::clicked, [this]() {
			qDebug() << "generateReportButton clicked";
			ReportDialog dialog(this);
			dialog.exec();
		});

}

// For both images, 800x1080 for Kinect, 534x720 for Realsense
void AnnotateTab::reloadCurrentImage(QImage colorImageLeft, cv::Mat depthMapToColorImage) {

	// Remove existing annotations in annotations member variable
	for (auto it : this->annotationsOnRight) this->annotationsOnRight[it.first] = QPointF();
	// Remove existing annotations in annotations member variable END

	this->depthToColorImage = depthMapToColorImage.clone();

	if (depthToColorImage.empty()) {
		qCritical() << "Cannot proceed since no successful depthToColorImage has been captured.";
		return;
	}

	cv::Mat BlankImage; 
	cv::Mat destRoi;

	//camera::Config* cameraConfig = camera::CameraManager::getInstance().getConfig();
	//int widthOfPatientBack = (int)(cameraConfig->color_height * 20.0 / 27.0 / 2.0 + 0.5) * 2;
	//int widthOffset = (cameraConfig->color_width - widthOfPatientBack) / 2;
	//BlankImage = cv::Mat(cameraConfig->color_height, cameraConfig->color_width, CV_16UC1);
	//destRoi = BlankImage(cv::Rect(widthOffset, 0, widthOfPatientBack, cameraConfig->color_height));
	//query3DOffsetX = widthOffset;

	if (depthMapToColorImage.cols == 800 && depthMapToColorImage.rows == 1080) {
		BlankImage = cv::Mat(1080, 1920, CV_16UC1);
		destRoi = BlankImage(cv::Rect(560, 0, 800, 1080));
		query3DOffsetX = 560;
	}
	else if (depthMapToColorImage.cols == 534 && depthMapToColorImage.rows == 720) {
		BlankImage = cv::Mat(720, 1280, CV_16UC1);
		destRoi = BlankImage(cv::Rect(373, 0, 534, 720));
		query3DOffsetX = 373;
	}
	else {
		qCritical() << "Cropped image must have dimensions either 800x1080 or 534x720, but now is not.";
	}

	RealsenseEngine::getInstance().readIntrinsicsFromFile("intrinsics_realsense_color.txt");

	depthMapToColorImage.copyTo(destRoi);
	this->recalculatedFullResolutionDepthImage = BlankImage;

	/*cv::Mat view;
	this->recalculatedFullResolutionDepthImage.convertTo(view, CV_8U, 255.0 / 5000.0, 0.0);
	cv::imshow("Checking", view);*/

	/** Display ai image from url */
	//QNetworkClient::getInstance().downloadImage(this->aiImageUrl, this, SLOT(onDownloadImage(QNetworkReply*)));
	/** Display ai image from url END */

	this->qColorImage = colorImageLeft.copy();
	this->qDepthToColorColorizedImage = convertDepthToColorCVToColorizedQImageDetailed(depthMapToColorImage);

	/** Find lower and upper depth, and update the upper and lower labels of the gradient color bar */
	float lower = 0.0f;
	float upper = 0.0f;
	if (!depthMapToColorImage.empty()) {
		// per unit is now (5000/255) mm = 19.6 mm
		depthMapToColorImage.convertTo(depthMapToColorImage, CV_8U, 255.0 / 5000.0, 0.0);

		uchar midDepth = findClosestNonZeroDepth(depthMapToColorImage, depthMapToColorImage.cols, depthMapToColorImage.rows);

		float lowerBound = 5.0; // 1 = 20mm, 5 = 1cm
		float upperBound = 7.5; // 15 = 2cm
		float lowerThreshold = midDepth - lowerBound;
		float upperThreshold = midDepth + upperBound;

		lower = lowerThreshold * 5000.0 / 255.0;
		upper = upperThreshold * 5000.0 / 255.0;

		qDebug() << "lower depth label:" << QString::number((int)lower) << "mm, upper depth label:" << QString::number((int)upper) << "mm";
		this->parent->ui.lowerLabel->setText(QString::number((int)lower) + " " + tr("mm"));
		this->parent->ui.upperLabel->setText(QString::number((int)upper) + " " + tr("mm"));
	}
	/** Find lower and upper depth, and update the upper and lower labels of the gradient color bar END */

	/** Cropping part 2 */
	/*cv::Rect cropRect = this->parent->captureTab->cropRect;
	this->qColorImage = this->qColorImage.copy(cropRect.x, cropRect.y, cropRect.width, cropRect.height);
	this->qDepthToColorColorizedImage = this->qDepthToColorColorizedImage.copy(cropRect.x, cropRect.y, cropRect.width, cropRect.height);*/
	/** Cropping part 2 END */

	resizeAndDrawAnnotations();


	this->computeMetrics();
	this->setAnnotationsText();
}

void AnnotateTab::resizeAndDrawAnnotations() {
	// scale both images according to displaying window size
	int width = this->parent->ui.graphicsViewAnnotation->width();
	int height = this->parent->ui.graphicsViewAnnotation->height();

	qDebug() << width << height;

	this->annotatedColorImage = this->qColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width();
	height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorColorizedImage = this->qDepthToColorColorizedImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	// Reset annotations
	this->scalingFactorForRight = std::min(this->qDepthToColorColorizedImage.width() / (float)this->annotatedDepthToColorColorizedImage.width(), this->qDepthToColorColorizedImage.height() / (float)this->annotatedDepthToColorColorizedImage.height());
	this->scalingFactorFromRightToLeft = this->annotatedColorImage.width() / (float)this->annotatedDepthToColorColorizedImage.width();
	qDebug() << "Scale from Depth to Color:" << scalingFactorFromRightToLeft;

	this->annotationsOnRight.clear();

	this->annotationsOnRight.insert({ "C", QPointF(predictedCX / scalingFactorForRight, predictedCY / scalingFactorForRight) });
	this->annotationsOnRight.insert({ "A1", QPointF(predictedA1X / scalingFactorForRight, predictedA1Y / scalingFactorForRight) });
	this->annotationsOnRight.insert({ "A2", QPointF(predictedA2X / scalingFactorForRight, predictedA2Y / scalingFactorForRight) });
	this->annotationsOnRight.insert({ "B1", QPointF(predictedB1X / scalingFactorForRight, predictedB1Y / scalingFactorForRight) });
	this->annotationsOnRight.insert({ "B2", QPointF(predictedB2X / scalingFactorForRight, predictedB2Y / scalingFactorForRight) });
	this->annotationsOnRight.insert({ "D", QPointF(predictedDX / scalingFactorForRight, predictedDY / scalingFactorForRight) });

	int x, y;

	//qDebug() << this->qDepthToColorColorizedImage.width();
	//qDebug() << this->annotatedDepthToColorColorizedImage.width();
	//qDebug() << this->qDepthToColorColorizedImage.height();
	//qDebug() << this->annotatedDepthToColorColorizedImage.height();
	//qDebug() << this->qDepthToColorColorizedImage.width() / (float)this->annotatedDepthToColorColorizedImage.width();
	//qDebug() << this->qDepthToColorColorizedImage.height() / (float)this->annotatedDepthToColorColorizedImage.height();

	for (auto it : this->annotationsOnRight) {
		x = it.second.x();
		y = it.second.y();
		x *= this->scalingFactorForRight;
		y *= this->scalingFactorForRight;
		//QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->depthToColorImage);
		QVector3D vector3D = RealsenseEngine::getInstance().query3DPoint(x + this->query3DOffsetX, y, this->recalculatedFullResolutionDepthImage);


		if (this->annotations3D.find(it.first) == this->annotations3D.end()) {
			this->annotations3D.insert({ it.first, vector3D });
		}
		else {
			this->annotations3D[it.first].setX(vector3D.x());
			this->annotations3D[it.first].setY(vector3D.y());
			this->annotations3D[it.first].setZ(vector3D.z());
		}
	}

	this->parent->ui.patientNameInCapture2->setText(tr("Current Patient: ") + this->parent->patientTab->getCurrentPatientName());

	this->drawAnnotations();
}

cv::Mat AnnotateTab::getDepthToColorImage()
{
	return this->depthToColorImage;
}

void AnnotateTab::setAiImageUrl(QString aiImageUrl)
{
	this->aiImageUrl = aiImageUrl;
}

void AnnotateTab::onLanguageChanged()
{
	this->setAnnotationsText();
	this->parent->ui.patientNameInCapture2->setText(tr("Current Patient: ") + this->parent->patientTab->getCurrentPatientName());
}

void AnnotateTab::drawAnnotations() {
	this->recopyAnnotatedImage();

	// Deallocate heap memory used by previous GGraphicsScene object
	if (this->colorScene) delete this->colorScene;
	if (this->depthToColorScene) delete this->depthToColorScene;

	this->colorScene = new DragAndDropGraphicsScene(this, ImageType::Color);
	this->depthToColorScene = new DragAndDropGraphicsScene(this, ImageType::DepthToColor);

	this->parent->ui.graphicsViewAnnotation->setScene(this->colorScene);
	this->parent->ui.graphicsViewAnnotation->show();

	this->parent->ui.graphicsViewAnnotation2->setScene(this->depthToColorScene);
	this->parent->ui.graphicsViewAnnotation2->show();
}

QImage* AnnotateTab::getQColorImage() {
	return &this->qColorImage;
}

QImage* AnnotateTab::getAnnotatedColorImage() {
	return &this->annotatedColorImage;
}

QImage* AnnotateTab::getAnnotatedDepthToColorColorizedImage() {
	return &this->annotatedDepthToColorColorizedImage;
}

std::map<std::string, QPointF>* AnnotateTab::getAnnotations() {
	return &this->annotationsOnRight;
}

void AnnotateTab::setAnnotationsText() {
	QString text = tr("");
	QString text2 = tr("");
	for (auto it : this->annotations3D) {
		std::string key = it.first;
		int x = this->annotationsOnRight[key].x() * this->scalingFactorForRight, y = this->annotationsOnRight[key].y() * this->scalingFactorForRight, z = it.second.z();
		//int x = it.second.z(), y = it.second.y(), z = it.second.z();

		QString PointName = "";
		if (it.first == "A1") {
			PointName = "Left Inf Scapular Angle (A1)";
		}
		else if ((it.first == "A2")) {
			PointName = "Right Inf Scapular Angle (A2)";
		}
		else if ((it.first == "B1")) {
			PointName = "Left PIIS (B1)";
		}
		else if ((it.first == "B2")) {
			PointName = "Right PIIS (B2)";
		}
		else if ((it.first == "C")) {
			PointName = "C7 (C)";
		}
		else if ((it.first == "D")) {
			PointName = "TOC (D)";
		}

		QString str = PointName + ": (" + QString::number(x) + ", " + QString::number(y) + ", " + QString::number(z) + ")\n";
		text2.append(str);
	}

	/** Round to 2 decimal places */
	QString distance1String = QString::number(this->distance1, 'f', 2);
	QString angle1String = QString::number(this->angle1, 'f', 2);
	QString angle2String = QString::number(this->angle2, 'f', 2);
	QString trunkRotationString = QString::number(this->trunkRotation, 'f', 2);
	/** Round to 2 decimal places END */

	if (this->invalidDistance == 1) {
		text += "Distance - Central Shift: Landmark C invalid \n";
	}
	else if (this->invalidDistance == 2) {
		text += "Distance - Central Shift: Landmark D invalid \n";
	}
	else {
		text += "Distance - Central Shift: " + distance1String + " mm\n";
	}
	
	text += "Imbalance - Scapular: " + angle2String + " degree\n";
	text += "Imbalance - Pelvic: " + angle1String + " degree\n";
	text += "Angle - Trunk Rotation: " + trunkRotationString + " degree\n";

	this->parent->ui.annotationsText->setText(text);
	this->parent->ui.annotationsText2->setText(text2);
}

void AnnotateTab::recopyAnnotatedImage() {
	int width = this->parent->ui.graphicsViewAnnotation->width();
	int height = this->parent->ui.graphicsViewAnnotation->height();
	this->annotatedColorImage = this->qColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width();
	height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorColorizedImage = this->qDepthToColorColorizedImage.copy().scaled(width, height, Qt::KeepAspectRatio);
}

DesktopApp* AnnotateTab::getParent()
{
	return this->parent;
}

QJsonDocument AnnotateTab::getAnnotationsJson() {
	QJsonObject emptyJsonObject{};
	QJsonDocument document;

	if (!this->annotationsOnRight["A1"].isNull()) {
		QJsonObject coordinates;

		for (auto it : this->annotationsOnRight) {
			QJsonObject coordinate;
			coordinate.insert("x", it.second.x());
			coordinate.insert("y", it.second.y());
			coordinates.insert(QString::fromStdString(it.first), coordinate);
		}

		emptyJsonObject.insert("coordinates", coordinates);
	}

	document.setObject(emptyJsonObject);
	return document;
}

DragAndDropGraphicsScene* AnnotateTab::getColorScene() {
	return this->colorScene;
}

DragAndDropGraphicsScene* AnnotateTab::getDepthToColorScene() {
	return this->depthToColorScene;
}

float* AnnotateTab::getScalingFactor() {
	return &this->scalingFactorForRight;
}

std::map<std::string, QVector3D>* AnnotateTab::getAnnotations3D() {
	return &this->annotations3D;
}

void AnnotateTab::computeMetrics() {
	const float PI = 3.14159265;

	// This is compute using 3D coordinates
	//this->distance1 = (this->annotations3D["C"].x() - this->annotations3D["D"].x());

	////Angle between b1-b2 line and xy-plane
	//float yDiff = this->annotations3D["B2"].y() - this->annotations3D["B1"].y();
	////float xyDistance = std::sqrt(std::pow(this->annotations3D["b1"].x() - this->annotations3D["b2"].x(), 2) + std::pow(this->annotations3D["b1"].y() - this->annotations3D["b2"].y(), 2));
	//float xDistance = this->annotations3D["B2"].x() - this->annotations3D["B1"].x();
	//this->angle1 = std::atan(yDiff / xDistance) * 180 / PI;

	////Angle between c1-c2 line and xy-plane
	//yDiff = this->annotations3D["A2"].y() - this->annotations3D["A1"].y();
	////xyDistance = std::sqrt(std::pow(this->annotations3D["c1"].x() - this->annotations3D["c2"].x(), 2) + std::pow(this->annotations3D["c1"].y() - this->annotations3D["c2"].y(), 2));
	//xDistance = this->annotations3D["A2"].x() - this->annotations3D["A1"].x();
	//this->angle2 = std::atan(yDiff / xDistance) * 180 / PI;

	// This is compute using 2D coordinates
	if (this->annotations3D["C"] == QVector3D(0, 0, 0)) {
		this->invalidDistance = 1;
	}
	else if (this->annotations3D["D"] == QVector3D(0, 0, 0)) {
		this->invalidDistance = 2;
	}
	else 
	{
		this->invalidDistance = 0;
		/*this->distance1 = - (this->annotations3D["D"].x() * (this->annotations3D["C"].z() / this->annotations3D["D"].z()) 
			- this->annotations3D["C"].x());*/
		this->distance1 = (this->annotations3D["C"].x() - this->annotations3D["D"].x());
	}
	

	// This is compute using 2D coordinates
	//Angle between b1-b2 line and xy-plane
	float yDiff = this->annotationsOnRight["B2"].y() - this->annotationsOnRight["B1"].y();
	float xDistance = this->annotationsOnRight["B2"].x() - this->annotationsOnRight["B1"].x();
	this->angle1 = std::atan(yDiff / xDistance) * 180 / PI;

	//Angle between c1-c2 line and xy-plane
	yDiff = this->annotationsOnRight["A2"].y() - this->annotationsOnRight["A1"].y();
	xDistance = this->annotationsOnRight["A2"].x() - this->annotationsOnRight["A1"].x();
	this->angle2 = std::atan(yDiff / xDistance) * 180 / PI;

	//Angle between x-diff and z-diff of A1 and A2 and B1 B2
	float AlphaxDiff = this->annotations3D["A2"].x() - this->annotations3D["A1"].x();
	float AlphazDiff = this->annotations3D["A2"].z() - this->annotations3D["A1"].z();

	float BetaxDiff = this->annotations3D["B2"].x() - this->annotations3D["B1"].x();
	float BetazDiff = this->annotations3D["B2"].z() - this->annotations3D["B1"].z();

	float AlphaTrunkRotation = std::atan(AlphazDiff / AlphaxDiff) * 180 / PI;
	float BetaTrunkRotation = std::atan(BetazDiff / BetaxDiff) * 180 / PI;

	/*qDebug() << "AlphaxDiff " << AlphaxDiff;
	qDebug() << "AlphazDiff " << AlphazDiff;
	qDebug() << "BetaxDiff " << BetaxDiff;
	qDebug() << "BetazDiff " << BetazDiff;
	qDebug() << "AlphaTrunkRotation " << AlphaTrunkRotation;
	qDebug() << "BetaTrunkRotation " << BetaTrunkRotation;*/

	this->trunkRotation = BetaTrunkRotation - AlphaTrunkRotation;
}	

void AnnotateTab::onConfirmLandmarks(QNetworkReply* reply) {
	qDebug() << "onConfirmLandmarks";

	QByteArray response_data = reply->readAll();
	reply->deleteLater();

	QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

	//qDebug() << jsonResponse;

	QJsonObject obj = jsonResponse.object();
	//QString aiImageUrl = obj["aiImageUrl"].toString();

	if (jsonResponse.isEmpty()) {
		TwoLinesDialog dialog;
		dialog.setLine1("Error: Unknown");
		dialog.exec();
		return;
	}

	if (obj.contains("error")) {
		QJsonObject child = obj["error"].toObject();
		TwoLinesDialog dialog;
		dialog.setLine1("Error: " + child["message"].toString());
		dialog.exec();
		return;
	}

	// Success
	TwoLinesDialog dialog;
	dialog.setLine1("Landmarks confirmed!");
	dialog.exec();
}


// Useful for fake X-ray image
/*
void AnnotateTab::onDownloadImage(QNetworkReply* reply) {
	qDebug() << "onDownloadImage";

	QByteArray response_data = reply->readAll();
	reply->deleteLater();

	QPixmap pixmap;
	pixmap.loadFromData(response_data);

	int width, height;
	width = this->parent->ui.graphicsViewImageUrl->width();
	height = this->parent->ui.graphicsViewImageUrl->height();

	QPixmap pixmapScaled = pixmap.scaled(width, height, Qt::KeepAspectRatio);
	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addPixmap(pixmapScaled);
	this->parent->ui.graphicsViewImageUrl->setScene(scene);
	this->parent->ui.graphicsViewImageUrl->show();
}*/
