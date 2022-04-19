#include "annotatetab.h"
#include "draganddropgraphicsscene.h"
#include "kinectengine.h"

QPointF getRandomPoint(int maxWidth, int maxHeight) {
	int randX = rand() % (maxWidth + 1);
	int randY = rand() % (maxHeight + 1);

	return QPointF((float) randX, (float) randY);
}

AnnotateTab::AnnotateTab(DesktopApp* parent) {
	this->parent = parent;

	this->colorScene = new DragAndDropGraphicsScene(this, ImageType::Color);
	this->depthToColorScene = new DragAndDropGraphicsScene(this, ImageType::DepthToColor);

	this->parent->ui.graphicsViewAnnotation->setScene(this->colorScene);
	this->parent->ui.graphicsViewAnnotation->show();

	this->parent->ui.graphicsViewAnnotation2->setScene(this->depthToColorScene);
	this->parent->ui.graphicsViewAnnotation2->show();

	QObject::connect(this->parent->ui.annotateButtonAnnotateTab, &QPushButton::clicked, [this]() {
		this->depthToColorImage = this->parent->captureTab->getCapturedDepthToColorImage().clone();
		this->qColorImage = this->parent->captureTab->getQColorImage().copy();
		this->qDepthToColorColorizedImage = this->parent->captureTab->getQDepthToColorColorizedImage().copy();

		int width = this->parent->ui.graphicsViewAnnotation->width();
		int height = this->parent->ui.graphicsViewAnnotation->height();
		this->annotatedColorImage = this->qColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

		width = this->parent->ui.graphicsViewAnnotation2->width();
		height = this->parent->ui.graphicsViewAnnotation2->height();
		this->annotatedDepthToColorColorizedImage = this->qDepthToColorColorizedImage.copy().scaled(width, height, Qt::KeepAspectRatio);


		// Reset annotations
		this->annotations.clear();

		this->annotations.insert({"C", QPointF(200.0f, 45.0f)});
		this->annotations.insert({"A1", QPointF(170.0f, 90.0f)});
		this->annotations.insert({"A2", QPointF(230.0f, 90.0f)});
		this->annotations.insert({"B1", QPointF(170.0f, 135.0f)});
		this->annotations.insert({"B2", QPointF(230.0f, 135.0f)});
		this->annotations.insert({"D", QPointF(200.0f, 180.0f)});

		int x, y;
		this->scalingFactor = std::min(this->qDepthToColorColorizedImage.width() / this->annotatedDepthToColorColorizedImage.width(), this->qDepthToColorColorizedImage.height() / this->annotatedDepthToColorColorizedImage.height());

		for (auto it : this->annotations) {
			x = it.second.x();
			y = it.second.y();
			x *= this->scalingFactor;
			y *= this->scalingFactor;
			QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->depthToColorImage);
			
			if (this->annotations3D.find(it.first) == this->annotations3D.end()) {
				this->annotations3D.insert({ it.first, vector3D });
			} else {
				this->annotations3D[it.first].setX(vector3D.x());
				this->annotations3D[it.first].setY(vector3D.y());
				this->annotations3D[it.first].setZ(vector3D.z());
			}
		}

		this->drawAnnotations();
		this->computeMetrics();
		this->setAnnotationsText();
	});

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
}

void AnnotateTab::reloadCurrentImage() {
	// Remove existing annotations in annotations member variable
	for (auto it : this->annotations) this->annotations[it.first] = QPointF();

	this->depthToColorImage = this->parent->captureTab->getCapturedDepthToColorImage().clone();
	this->qColorImage = this->parent->captureTab->getQColorImage().copy();
	this->qDepthToColorColorizedImage = this->parent->captureTab->getQDepthToColorColorizedImage().copy();

	int width = this->parent->ui.graphicsViewAnnotation2->width();
	int height = this->parent->ui.graphicsViewAnnotation2->height();

	// Deallocate heap memory used by previous GGraphicsScene object
    if (this->colorScene) delete this->colorScene;
	if (this->depthToColorScene) delete this->depthToColorScene;

	this->colorScene = new DragAndDropGraphicsScene(this, ImageType::Color);
	this->depthToColorScene = new DragAndDropGraphicsScene(this, ImageType::DepthToColor);

	this->parent->ui.graphicsViewAnnotation->setScene(this->colorScene);
	this->parent->ui.graphicsViewAnnotation->show();

	this->parent->ui.graphicsViewAnnotation2->setScene(this->depthToColorScene);
	this->parent->ui.graphicsViewAnnotation2->show();

    this->parent->ui.annotateButtonAnnotateTab->click();
}

cv::Mat AnnotateTab::getDepthToColorImage()
{
	return this->depthToColorImage;
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
	for(auto it: this->annotations3D) {
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

	if (!this->annotations["a"].isNull()) {
		QJsonObject coordinates;

		for(auto it: this->annotations) {
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

int* AnnotateTab::getScalingFactor() {
	return &this->scalingFactor;
}

std::map<std::string, QVector3D>* AnnotateTab::getAnnotations3D() {
	return &this->annotations3D;
}

void AnnotateTab::computeMetrics() {
	const float PI = 3.14159265;
	this->distance1 = (this->annotations3D["D"].x() - this->annotations3D["C"].x())/10;
	
	//Angle between b1-b2 line and xy-plane
	float yDiff = this->annotations3D["A1"].y() - this->annotations3D["A2"].y();
	//float xyDistance = std::sqrt(std::pow(this->annotations3D["b1"].x() - this->annotations3D["b2"].x(), 2) + std::pow(this->annotations3D["b1"].y() - this->annotations3D["b2"].y(), 2));
	float xDistance = this->annotations3D["A2"].x() - this->annotations3D["A1"].x();
	this->angle1 = std::atan(yDiff/xDistance) * 180 / PI;

	//Angle between c1-c2 line and xy-plane
	yDiff = this->annotations3D["B1"].y() - this->annotations3D["B2"].y();
	//xyDistance = std::sqrt(std::pow(this->annotations3D["c1"].x() - this->annotations3D["c2"].x(), 2) + std::pow(this->annotations3D["c1"].y() - this->annotations3D["c2"].y(), 2));
	xDistance = this->annotations3D["B2"].x() - this->annotations3D["B1"].x();
	this->angle2 = std::atan(yDiff/xDistance) * 180 / PI;
}
