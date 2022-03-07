#include "annotatetab.h"
#include "draganddropgraphicsscene.h"

QPointF getRandomPoint(int maxWidth, int maxHeight) {
	int randX = rand() % (maxWidth + 1);
	int randY = rand() % (maxHeight + 1);

	return QPointF((float) randX, (float) randY);
}

AnnotateTab::AnnotateTab(DesktopApp* parent) {
	this->parent = parent;

	this->colorImage = this->parent->captureTab->getQCapturedColorImage().copy();
	this->depthToColorImage = this->parent->captureTab->getQCapturedDepthToColorImage().copy();

	int width = this->parent->ui.graphicsViewAnnotation->width(), height = this->parent->ui.graphicsViewAnnotation->height();
	this->annotatedColorImage = this->colorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width();  height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorImage = this->depthToColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	this->colorScene = new DragAndDropGraphicsScene(this, ImageType::Color);
	this->depthToColorScene = new DragAndDropGraphicsScene(this, ImageType::DepthToColor);

	this->parent->ui.graphicsViewAnnotation->setScene(this->colorScene);
	this->parent->ui.graphicsViewAnnotation->show();

	this->parent->ui.graphicsViewAnnotation2->setScene(this->depthToColorScene);
	this->parent->ui.graphicsViewAnnotation2->show();

	QObject::connect(this->parent->ui.annotateButtonAnnotateTab, &QPushButton::clicked, [this]() {
		int width = this->annotatedColorImage.width(), height = this->annotatedColorImage.height();

		// Reset annotations
		this->annotations.clear();

		this->annotations.insert({"a", getRandomPoint(width, height)});
		this->annotations.insert({"b1", getRandomPoint(width, height)});
		this->annotations.insert({"b2", getRandomPoint(width, height)});
		this->annotations.insert({"c1", getRandomPoint(width, height)});
		this->annotations.insert({"c2", getRandomPoint(width, height)});
		this->annotations.insert({"d", getRandomPoint(width, height)});

		int x, y;
		this->scalingFactor = std::min(this->depthToColorImage.width() / this->annotatedDepthToColorImage.width(), this->depthToColorImage.height() / this->annotatedDepthToColorImage.height());

		for (auto it : this->annotations) {
			x = it.second.x();
			y = it.second.y();
			x *= this->scalingFactor;
			y *= this->scalingFactor;
			QVector3D vector3D = this->parent->captureTab->query3DPoint(x, y);
			
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
		if (!depthToColorWriter.write(this->annotatedDepthToColorImage)) {
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

	this->colorImage = this->parent->captureTab->getQCapturedColorImage().copy();
	this->depthToColorImage = this->parent->captureTab->getQCapturedDepthToColorImage().copy();

	int width = this->parent->ui.graphicsViewAnnotation->width(), height = this->parent->ui.graphicsViewAnnotation->height();
	this->annotatedColorImage = this->colorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width(); height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorImage = this->depthToColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

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

QImage* AnnotateTab::getImage() {
	return &this->colorImage;
}

QImage* AnnotateTab::getAnnotatedColorImage() {
	return &this->annotatedColorImage;
}

QImage* AnnotateTab::getAnnotatedDepthToColorImage() {
	return &this->annotatedDepthToColorImage;
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

	text.append(QString::fromStdString("Distance 1 ( b1 - b2 ): " + std::to_string(this->distance1) + " cm\n"));
	text.append(QString::fromStdString("Distance 2 ( c1 - c2 ): " + std::to_string(this->distance2) + " cm\n"));
	text.append(QString::fromStdString("Alpha: " + std::to_string(this->angle1) + " degree\n"));
	text.append(QString::fromStdString("Beta: " + std::to_string(this->angle2) + " degree\n"));

	this->parent->ui.annotationsText->setText(text);
}

void AnnotateTab::recopyAnnotatedImage() {
	int width = this->parent->ui.graphicsViewAnnotation->width(), height = this->parent->ui.graphicsViewAnnotation->height();
	this->annotatedColorImage = this->colorImage.copy().scaled(width, height, Qt::KeepAspectRatio);

	width = this->parent->ui.graphicsViewAnnotation2->width();  height = this->parent->ui.graphicsViewAnnotation2->height();
	this->annotatedDepthToColorImage = this->depthToColorImage.copy().scaled(width, height, Qt::KeepAspectRatio);
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

QVector3D AnnotateTab::query3DPoint(int x, int y) {
	return this->parent->captureTab->query3DPoint(x, y);
}

void AnnotateTab::computeMetrics() {
	const float PI = 3.14159265;
	this->distance1 = this->annotations3D["b1"].distanceToPoint(this->annotations3D["b2"])/10;
	this->distance2 = this->annotations3D["c1"].distanceToPoint(this->annotations3D["c2"])/10;
	
	//Angle between b1-b2 line and xy-plane
	float zDiff = this->annotations3D["b1"].z() - this->annotations3D["b2"].z();
	float xyDistance = std::sqrt(std::pow(this->annotations3D["b1"].x() - this->annotations3D["b2"].x(), 2) + std::pow(this->annotations3D["b1"].y() - this->annotations3D["b2"].y(), 2));
	this->angle1 = std::atan(zDiff/xyDistance) * 180 / PI;

	//Angle between c1-c2 line and xy-plane
	zDiff = this->annotations3D["c1"].z() - this->annotations3D["c2"].z();
	xyDistance = std::sqrt(std::pow(this->annotations3D["c1"].x() - this->annotations3D["c2"].x(), 2) + std::pow(this->annotations3D["c1"].y() - this->annotations3D["c2"].y(), 2));
	this->angle2 = std::atan(zDiff/xyDistance) * 180 / PI;
}
