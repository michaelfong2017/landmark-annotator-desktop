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

	QObject::connect(this->parent->ui.saveButtonAnnotateTab, &QPushButton::clicked, [this]() {
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
		});

	QObject::connect(this->parent->ui.confirmLandmarksButton, &QPushButton::clicked, [this]() {
		qDebug() << "confirmLandmarksButton clicked";

		QString aiOriginResult = QString("[[");

		float x, y;
		QPointF second;

		// C
		second = this->annotations.at("C");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// C END

		// A1
		second = this->annotations.at("A1");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// A1 END

		// A2
		second = this->annotations.at("A2");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// A2 END

		// B1
		second = this->annotations.at("B1");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// B1 END

		// B2
		second = this->annotations.at("B2");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

		// One decimal place, e.g., 170.0
		aiOriginResult += QString("%1").arg(x, 0, 'f', 1);
		aiOriginResult += ",";
		aiOriginResult += QString("%1").arg(y, 0, 'f', 1);
		aiOriginResult += "],[";
		// B2 END

		// D
		second = this->annotations.at("D");
		x = second.x();
		y = second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;

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
	for (auto it : this->annotations) this->annotations[it.first] = QPointF();
	// Remove existing annotations in annotations member variable END

	this->depthToColorImage = this->parent->captureTab->getCapturedDepthToColorImage().clone();

	if (depthToColorImage.empty()) {
		qCritical() << "Cannot proceed since no successful depthToColorImage has been captured.";
		return;
	}

	/** Display ai image from url */
	QNetworkClient::getInstance().downloadImage(this->aiImageUrl, this, SLOT(onDownloadImage(QNetworkReply*)));
	/** Display ai image from url END */

	///** New plane calculation */
	//float* p;
	//p = KinectEngine::getInstance().findPlaneEquationCoefficients(this->depthToColorImage);
	///** New plane calculation END */

	///** Calculate plane equation and distance to plane of each 3D point */
	///**** Get the 3D coordinates of 3 reference points that are assumed to lie on the plane. */
	//// HARD CODED VALUES
	//float x1_2D = 360.0f;
	//float y1_2D = 360.0f;
	//float x2_2D = 750.0f;
	//float y2_2D = 250.0f;
	//float x3_2D = 750.0f;
	//float y3_2D = 470.0f;
	//// HARD CODED VALUES END

	//QVector3D vector3D_1 = KinectEngine::getInstance().query3DPoint(x1_2D, y1_2D, this->depthToColorImage);
	//QVector3D vector3D_2 = KinectEngine::getInstance().query3DPoint(x2_2D, y2_2D, this->depthToColorImage);
	//QVector3D vector3D_3 = KinectEngine::getInstance().query3DPoint(x3_2D, y3_2D, this->depthToColorImage);

	///**** Get the 3D coordinates of 3 reference points that are assumed to lie on the plane. END */
	//float* abcd;
	//abcd = KinectEngine::getInstance().findPlaneEquationCoefficients(
	//	vector3D_1.x(), vector3D_1.y(), vector3D_1.z(),
	//	vector3D_2.x(), vector3D_2.y(), vector3D_2.z(),
	//	vector3D_3.x(), vector3D_3.y(), vector3D_3.z()
	//);

	//float a = abcd[0];
	//float b = abcd[1];
	//float c = abcd[2];
	//float d = abcd[3];

	//qDebug() << "Equation of plane is " << a << " x + " << b
	//	<< " y + " << c << " z + " << d << " = 0.";

	//cv::Mat out = cv::Mat::zeros(depthToColorImage.rows, depthToColorImage.cols, CV_16UC1);
	//float maxDistance = 0.0f;
	//for (int y = 0; y < depthToColorImage.rows; y++) {
	//	for (int x = 0; x < depthToColorImage.cols; x++) {
	//		QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->depthToColorImage);
	//		
	//		if (vector3D.x() == 0.0f && vector3D.y() == 0.0f && vector3D.z() == 0.0f) {
	//			out.at<uint16_t>(y, x) = 0.0f;
	//			continue;
	//		}

	//		float distance = KinectEngine::getInstance().findDistanceBetween3DPointAndPlane(vector3D.x(), vector3D.y(), vector3D.z(), a, b, c, d);
	//		out.at<uint16_t>(y, x) = distance;
	//		
	//		if (distance > maxDistance) {
	//			maxDistance = distance;
	//		}
	//	}
	//}

	//QString visitFolderPath = Helper::getVisitFolderPath(this->parent->savePath);
	//QString depthSavePath = QDir(visitFolderPath).filePath("out.png");

	//cv::Mat temp;
	//out.convertTo(temp, CV_8U, 255.0 / maxDistance, 0.0);

	//cv::imwrite(depthSavePath.toStdString(), temp);

	///** Calculate plane equation and distance to plane of each 3D point END */

	this->qColorImage = this->parent->captureTab->getQColorImage().copy();
	this->qDepthToColorColorizedImage = this->parent->captureTab->getQDepthToColorColorizedImage().copy();

	int width = this->parent->ui.graphicsViewAnnotation->width();
	int height = this->parent->ui.graphicsViewAnnotation->height();
	this->annotatedColorImage = this->qColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width();
	height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorColorizedImage = this->qDepthToColorColorizedImage.copy().scaled(width, height, Qt::KeepAspectRatio);


	// Reset annotations
	this->scalingFactor = std::min(this->qDepthToColorColorizedImage.width() / (float)this->annotatedDepthToColorColorizedImage.width(), this->qDepthToColorColorizedImage.height() / (float)this->annotatedDepthToColorColorizedImage.height());

	this->annotations.clear();

	//this->annotations.insert({ "C", QPointF(240.0f, 70.0f) });
	//this->annotations.insert({ "A1", QPointF(220.0f, 130.0f) });
	//this->annotations.insert({ "A2", QPointF(260.0f, 130.0f) });
	//this->annotations.insert({ "B1", QPointF(220.0f, 165.0f) });
	//this->annotations.insert({ "B2", QPointF(260.0f, 165.0f) });
	//this->annotations.insert({ "D", QPointF(240.0f, 185.0f) });

	this->annotations.insert({ "C", QPointF(predictedCX / scalingFactor, predictedCY / scalingFactor) });
	this->annotations.insert({ "A1", QPointF(predictedA1X / scalingFactor, predictedA1Y / scalingFactor) });
	this->annotations.insert({ "A2", QPointF(predictedA2X / scalingFactor, predictedA2Y / scalingFactor) });
	this->annotations.insert({ "B1", QPointF(predictedB1X / scalingFactor, predictedB1Y / scalingFactor) });
	this->annotations.insert({ "B2", QPointF(predictedB2X / scalingFactor, predictedB2Y / scalingFactor) });
	this->annotations.insert({ "D", QPointF(predictedDX / scalingFactor, predictedDY / scalingFactor) });

	int x, y;

	//qDebug() << this->qDepthToColorColorizedImage.width();
	//qDebug() << this->annotatedDepthToColorColorizedImage.width();
	//qDebug() << this->qDepthToColorColorizedImage.height();
	//qDebug() << this->annotatedDepthToColorColorizedImage.height();
	//qDebug() << this->qDepthToColorColorizedImage.width() / (float)this->annotatedDepthToColorColorizedImage.width();
	//qDebug() << this->qDepthToColorColorizedImage.height() / (float)this->annotatedDepthToColorColorizedImage.height();

	for (auto it : this->annotations) {
		x = it.second.x();
		y = it.second.y();
		x *= this->scalingFactor;
		y *= this->scalingFactor;
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
	return &this->annotations;
}

void AnnotateTab::setAnnotationsText() {
	QString text = "";
	for (auto it : this->annotations3D) {
		int x = it.second.x(), y = it.second.y(), z = it.second.z();
		std::string plain_s = "Point " + it.first + ": (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")\n";
		QString str = QString::fromUtf8(plain_s.c_str());
		text.append(str);
	}

	text.append(QString::fromStdString("Distance d ( D - C ): " + std::to_string(this->distance1) + " cm\n"));
	text.append(QString::fromStdString("Alpha: " + std::to_string(this->angle1) + " degree\n"));
	text.append(QString::fromStdString("Beta: " + std::to_string(this->angle2) + " degree\n"));

	this->parent->ui.annotationsText->setText(text);
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

	if (!this->annotations["A1"].isNull()) {
		QJsonObject coordinates;

		for (auto it : this->annotations) {
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
	return &this->scalingFactor;
}

std::map<std::string, QVector3D>* AnnotateTab::getAnnotations3D() {
	return &this->annotations3D;
}

void AnnotateTab::computeMetrics() {
	const float PI = 3.14159265;
	this->distance1 = (this->annotations3D["D"].x() - this->annotations3D["C"].x()) / 10;

	//Angle between b1-b2 line and xy-plane
	float yDiff = this->annotations3D["B1"].y() - this->annotations3D["B2"].y();
	//float xyDistance = std::sqrt(std::pow(this->annotations3D["b1"].x() - this->annotations3D["b2"].x(), 2) + std::pow(this->annotations3D["b1"].y() - this->annotations3D["b2"].y(), 2));
	float xDistance = this->annotations3D["B2"].x() - this->annotations3D["B1"].x();
	this->angle1 = std::atan(yDiff / xDistance) * 180 / PI;

	//Angle between c1-c2 line and xy-plane
	yDiff = this->annotations3D["A1"].y() - this->annotations3D["A2"].y();
	//xyDistance = std::sqrt(std::pow(this->annotations3D["c1"].x() - this->annotations3D["c2"].x(), 2) + std::pow(this->annotations3D["c1"].y() - this->annotations3D["c2"].y(), 2));
	xDistance = this->annotations3D["A2"].x() - this->annotations3D["A1"].x();
	this->angle2 = std::atan(yDiff / xDistance) * 180 / PI;
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
		LandmarksConfirmedDialog dialog;
		dialog.exec();
	}
}

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
}
