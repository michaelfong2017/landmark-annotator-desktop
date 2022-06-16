#include "draganddropgraphicsscene.h"
#include "annotatetab.h"
#include "kinectengine.h"

DragAndDropGraphicsScene::DragAndDropGraphicsScene( AnnotateTab* annotateTab, ImageType imageType) {

	this->annotateTab = annotateTab;
	this->imageType = imageType;

	qDebug() << "DragAndDropGraphicsScene";

	this->addPixmap(QPixmap::fromImage(this->imageType == ImageType::Color ? *this->annotateTab->getAnnotatedColorImage() : *this->annotateTab->getAnnotatedDepthToColorColorizedImage()));

	/** Human cut shape */
	int width, height;
	if (this->imageType == ImageType::Color) {
		width = this->annotateTab->getParent()->ui.graphicsViewAnnotation->width();
		height = this->annotateTab->getParent()->ui.graphicsViewAnnotation->height();
	}
	else if (this->imageType == ImageType::DepthToColor) {
		width = this->annotateTab->getParent()->ui.graphicsViewAnnotation2->width();
		height = this->annotateTab->getParent()->ui.graphicsViewAnnotation2->height();
	}
	else {
		qWarning() << "draganddropgraphicsscene ImageType is neither Color nor DepthToColor!";
	}

	QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape.png");
	QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);
	//this->addPixmap(humanPixmapScaled);
	/** Human cut shape END */

	// Draw annotations if any
	//QPainter painter(this->imageType == ImageType::Color ? this->annotateTab->getAnnotatedColorImage() : this->annotateTab->getAnnotatedDepthToColorColorizedImage());
	QPainter painter(this->imageType == ImageType::Color ? &humanPixmapScaled : &humanPixmapScaled);

	painter.setPen(QPen(Qt::magenta, 8, Qt::SolidLine, Qt::RoundCap));
	for (auto it : *this->annotateTab->getAnnotations()) {
		if (!it.second.isNull()) painter.drawPoint(it.second.x(), it.second.y());
	}

	painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
	for (auto it : *this->annotateTab->getAnnotations()) {
		if (!it.second.isNull()) painter.drawText(it.second.x(), it.second.y(), QString::fromStdString(it.first));
	}

	painter.setPen(QPen(Qt::white, 0.5, Qt::DashLine, Qt::RoundCap));
	painter.drawLine(
		(*this->annotateTab->getAnnotations())["A1"].x(), (*this->annotateTab->getAnnotations())["A1"].y(),
		(*this->annotateTab->getAnnotations())["A2"].x(), (*this->annotateTab->getAnnotations())["A2"].y()
	);

	painter.drawLine(
		(*this->annotateTab->getAnnotations())["B1"].x(), (*this->annotateTab->getAnnotations())["B1"].y(),
		(*this->annotateTab->getAnnotations())["B2"].x(), (*this->annotateTab->getAnnotations())["B2"].y()
	);

	painter.end();

	this->addPixmap(humanPixmapScaled);
	this->annotateTab->computeMetrics();
	this->annotateTab->setAnnotationsText();
}

void DragAndDropGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	float x = event->scenePos().x(), y = event->scenePos().y();
	this->isPoint = false;
	this->pointKey = "";

	for(auto it: *this->annotateTab->getAnnotations()) {
		if (abs(it.second.x() - x) <= 5 && abs(it.second.y() - y) <= 5) {
			this->isPoint = true;
			this->pointKey = it.first;
		}
	}

	if (event->button() == Qt::LeftButton && this->isPoint) {
		QDrag* drag = new QDrag(this);
		QPixmap* point = new QPixmap(5, 5);
		point->fill(Qt::red);

		QMimeData* mimeData = new QMimeData;
		mimeData->setText("annotation");

		drag->setMimeData(mimeData);
		drag->setPixmap(*point);

		Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	}
}

void DragAndDropGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent* event) {
	if (this->isPoint) {

		qDebug() << "Drop Event";

		this->annotateTab->recopyAnnotatedImage();

		float x = event->scenePos().x(), y = event->scenePos().y();
		(*this->annotateTab->getAnnotations())[this->pointKey].setX(x);
		(*this->annotateTab->getAnnotations())[this->pointKey].setY(y);

		x *= *this->annotateTab->getScalingFactor();
		y *= *this->annotateTab->getScalingFactor();
		QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->annotateTab->getDepthToColorImage());

		(*this->annotateTab->getAnnotations3D())[this->pointKey].setX(vector3D.x());
		(*this->annotateTab->getAnnotations3D())[this->pointKey].setY(vector3D.y());
		(*this->annotateTab->getAnnotations3D())[this->pointKey].setZ(vector3D.z());

		if (this->imageType == ImageType::Color) {
			this->annotateTab->getColorScene()->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedColorImage()));
			this->annotateTab->getDepthToColorScene()->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedDepthToColorColorizedImage()));
		}

		if (this->imageType == ImageType::DepthToColor) {
			this->annotateTab->getColorScene()->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedColorImage()));
			this->annotateTab->getDepthToColorScene()->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedDepthToColorColorizedImage()));
		}

		/** Human cut shape */
		int width = this->annotateTab->getParent()->ui.graphicsViewAnnotation->width();
		int height = this->annotateTab->getParent()->ui.graphicsViewAnnotation->height();
		QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape.png");
		QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);
		/** Human cut shape END */

		QPainter painter(&humanPixmapScaled);

		painter.setPen(QPen(Qt::magenta, 8, Qt::SolidLine, Qt::RoundCap));
		for(auto it: *this->annotateTab->getAnnotations()) {
			painter.drawPoint(it.second.x(), it.second.y());
		}
		painter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
		for(auto it: *this->annotateTab->getAnnotations()) {
			painter.drawText(it.second.x(), it.second.y(), QString::fromStdString(it.first));

		}
		painter.setPen(QPen(Qt::white , 0.5, Qt::DashLine, Qt::RoundCap));
		painter.drawLine(
			(*this->annotateTab->getAnnotations())["A1"].x(), (*this->annotateTab->getAnnotations())["A1"].y(),
			(*this->annotateTab->getAnnotations())["A2"].x(), (*this->annotateTab->getAnnotations())["A2"].y()
		);
		painter.drawLine(
			(*this->annotateTab->getAnnotations())["B1"].x(), (*this->annotateTab->getAnnotations())["B1"].y(),
			(*this->annotateTab->getAnnotations())["B2"].x(), (*this->annotateTab->getAnnotations())["B2"].y()
		);
		painter.end();

		
		if (this->imageType == ImageType::Color) {
			//this->addPixmap(humanPixmapScaled);
			//this->annotateTab->getDepthToColorScene()->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedDepthToColorColorizedImage()));
		}

		if (this->imageType == ImageType::DepthToColor) {
			//this->addPixmap(humanPixmapScaled2);
			//this->annotateTab->getColorScene()->addPixmap(humanPixmapScaled2);
			//this->addPixmap(QPixmap::fromImage(*this->annotateTab->getAnnotatedDepthToColorColorizedImage()));
		}

		this->annotateTab->getColorScene()->addPixmap(humanPixmapScaled);
		this->annotateTab->getDepthToColorScene()->addPixmap(humanPixmapScaled);

		this->annotateTab->computeMetrics();
		this->annotateTab->setAnnotationsText();
	}
	
	event->acceptProposedAction();
}

void DragAndDropGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
	event->acceptProposedAction();
}

void DragAndDropGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
	event->acceptProposedAction();
}
