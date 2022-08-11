#include "annotatetab.h"
#include "draganddropgraphicsscene.h"
#include "kinectengine.h"

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

	/*QObject::connect(this->parent->ui.saveButtonAnnotateTab, &QPushButton::clicked, [this]() {
		QString dateTimeString = Helper::getCurrentDateTimeString();
		QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
		QString colorSavePath = visitFolderPath + "/landmarks_color_" + dateTimeString + ".png";
		QString depthToColorSavePath = visitFolderPath + "/landmarks_rgbd_" + dateTimeString + ".png";
		QString landmarksSavePath = visitFolderPath + "/landmarks_" + dateTimeString + ".json";

		// Save color image
		QImageWriter colorWriter(colorSavePath);
		if (!colorWriter.write(this->annotatedColorImage)) {
			qDebug() << colorWriter.errorString();
			this->parent->ui.saveInfoAnnotateTab->setText("Something went wrong, cannot save images.");
			return;
		}

		// Save RGBD image
		QImageWriter depthToColorWriter(depthToColorSavePath);
		if (!depthToColorWriter.write(this->annotatedDepthToColorColorizedImage)) {
			qDebug() << depthToColorWriter.errorString();
			this->parent->ui.saveInfoAnnotateTab->setText("Something went wrong, cannot save images.");
			return;
		}

		// Save landmarks in json
		QFile jsonFile(landmarksSavePath);
		jsonFile.open(QFile::WriteOnly);
		QJsonDocument document = this->getAnnotationsJson();
		jsonFile.write(document.toJson());

		this->parent->ui.saveInfoAnnotateTab->setText("Images saved as " + colorSavePath + " and " + depthToColorSavePath);
		});*/

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
}

void AnnotateTab::reloadCurrentImage() {
	// Remove existing annotations in annotations member variable
	for (auto it : this->annotationsOnRight) this->annotationsOnRight[it.first] = QPointF();
	// Remove existing annotations in annotations member variable END

	this->depthToColorImage = this->parent->captureTab->getCapturedDepthToColorImage().clone();

	if (depthToColorImage.empty()) {
		qCritical() << "Cannot proceed since no successful depthToColorImage has been captured.";
		return;
	}

	/** Display ai image from url */
	//QNetworkClient::getInstance().downloadImage(this->aiImageUrl, this, SLOT(onDownloadImage(QNetworkReply*)));
	/** Display ai image from url END */

	this->qColorImage = this->parent->captureTab->getQColorImage().copy();
	this->qDepthToColorColorizedImage = this->parent->captureTab->getQDepthToColorColorizedImage().copy();

	// scale both images according to displaying window size
	int width = this->parent->ui.graphicsViewAnnotation->width();
	int height = this->parent->ui.graphicsViewAnnotation->height();
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
		QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->depthToColorImage);

		if (this->annotations3D.find(it.first) == this->annotations3D.end()) {
			this->annotations3D.insert({ it.first, vector3D });
		}
		else {
			this->annotations3D[it.first].setX(vector3D.x());
			this->annotations3D[it.first].setY(vector3D.y());
			this->annotations3D[it.first].setZ(vector3D.z());
		}
	}

	this->drawAnnotations();
	this->computeMetrics();
	this->setAnnotationsText();
}

cv::Mat AnnotateTab::getDepthToColorImage()
{
	return this->depthToColorImage;
}

void AnnotateTab::setAiImageUrl(QString aiImageUrl)
{
	this->aiImageUrl = aiImageUrl;
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
	QString text = "";
	QString text2 = "";
	for (auto it : this->annotations3D) {
		std::string key = it.first;
		int x = this->annotationsOnRight[key].x() * this->scalingFactorForRight, y = this->annotationsOnRight[key].y() * this->scalingFactorForRight, z = it.second.z();
		//int x = it.second.z(), y = it.second.y(), z = it.second.z();

		std::string PointName = "";
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

		std::string plain_s = PointName + ": (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")\n";
		QString str = QString::fromUtf8(plain_s.c_str());
		text2.append(str);
	}

	text.append(QString::fromStdString("Distance - Central Shift: " + std::to_string(this->distance1) + " mm\n"));
	text.append(QString::fromStdString("Imbalance - Pelvic: " + std::to_string(this->angle1) + " degree\n"));
	text.append(QString::fromStdString("Imbalance - Scapular: " + std::to_string(this->angle2) + " degree\n"));
	text.append(QString::fromStdString("Distance - Trunk Rotation: " + std::to_string(this->trunkRotation) + " mm\n"));

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
	if (this->annotations3D["C"] == QVector3D(0, 0, 0) || this->annotations3D["D"] == QVector3D(0, 0, 0)) {
		this->distance1 = 0;
	}
	else {
		this->distance1 = - (this->annotations3D["D"].x() * (this->annotations3D["C"].z() / this->annotations3D["D"].z()) 
			- this->annotations3D["C"].x());
		//this->distance1 = (this->annotations3D["C"].x() - this->annotations3D["D"].x());
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

	//Angle between x-diff and z-diff of A1 and A2
	//float xDistance = this->annotations3D["A2"].x() - this->annotations3D["A1"].x();
	float zDiff = this->annotations3D["A1"].z() - this->annotations3D["A2"].z();
	//this->trunkRotation = std::atan(zDiff / xDistance) * 180 / PI;
	this->trunkRotation = zDiff;
}

void AnnotateTab::onConfirmLandmarks(QNetworkReply* reply) {
	qDebug() << "onConfirmLandmarks";

	QByteArray response_data = reply->readAll();
	reply->deleteLater();

	QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

	qDebug() << jsonResponse;

	QJsonObject obj = jsonResponse.object();
	QString aiImageUrl = obj["aiImageUrl"].toString();

	if (!aiImageUrl.isEmpty()) {
		// Success
		TwoLinesDialog dialog;
		dialog.setLine1("Landmarks confirmed!");
		dialog.exec();
	}
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
