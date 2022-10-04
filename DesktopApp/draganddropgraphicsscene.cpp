#include "draganddropgraphicsscene.h"
#include "annotatetab.h"
#include "kinectengine.h"

DragAndDropGraphicsScene::DragAndDropGraphicsScene( AnnotateTab* annotateTab, ImageType imageType) {

	this->annotateTab = annotateTab;
	this->imageType = imageType;

	qDebug() << "DragAndDropGraphicsScene";

	QPixmap qPixmap = QPixmap::fromImage(this->imageType == ImageType::Color ? *this->annotateTab->getAnnotatedColorImage() : *this->annotateTab->getAnnotatedDepthToColorColorizedImage());

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

	/** Human cut shape Start */
	//QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape.png");
	//QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);
	/** Human cut shape END */

	// Draw annotations if any
	QPainter painter(&qPixmap);

	if (this->imageType == ImageType::Color)
	{
		painter.setPen(QPen(Qt::magenta, 10, Qt::SolidLine, Qt::RoundCap));
		for (auto it : *this->annotateTab->getAnnotations()) {
			if (!it.second.isNull()) painter.drawPoint(it.second.x() * this->annotateTab->scalingFactorFromRightToLeft, it.second.y() * this->annotateTab->scalingFactorFromRightToLeft);
		}

		painter.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
		for (auto it : *this->annotateTab->getAnnotations()) {
			if (!it.second.isNull()) painter.drawText(it.second.x() * this->annotateTab->scalingFactorFromRightToLeft, it.second.y() * this->annotateTab->scalingFactorFromRightToLeft, QString::fromStdString(it.first));
		}

		painter.setPen(QPen(Qt::white, 0.5, Qt::DashLine, Qt::RoundCap));
		painter.drawLine(
			(*this->annotateTab->getAnnotations())["A1"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["A1"].y() * this->annotateTab->scalingFactorFromRightToLeft,
			(*this->annotateTab->getAnnotations())["A2"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["A2"].y() * this->annotateTab->scalingFactorFromRightToLeft
		);

		painter.drawLine(
			(*this->annotateTab->getAnnotations())["B1"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["B1"].y() * this->annotateTab->scalingFactorFromRightToLeft,
			(*this->annotateTab->getAnnotations())["B2"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["B2"].y() * this->annotateTab->scalingFactorFromRightToLeft
		);
	}
	else if (this->imageType == ImageType::DepthToColor) 
	{
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
	}

	painter.end();

	this->addPixmap(qPixmap);
	this->annotateTab->computeMetrics();
	this->annotateTab->setAnnotationsText();
}

void DragAndDropGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
	float x = event->scenePos().x(), y = event->scenePos().y();

	if (this->imageType == ImageType::Color) {
		y = y / this->annotateTab->scalingFactorFromRightToLeft;
		x = x / this->annotateTab->scalingFactorFromRightToLeft;
	}

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

		this->annotateTab->recopyAnnotatedImage();

		float x = event->scenePos().x(), y = event->scenePos().y();

		if (this->imageType == ImageType::Color) {
			// convert back to color window coordinates
			y = y / this->annotateTab->scalingFactorFromRightToLeft;
			x = x / this->annotateTab->scalingFactorFromRightToLeft;
			qDebug() << "Drop Event : Color Drop: " << x << ", " << y;
		}else if (this->imageType == ImageType::DepthToColor) {
			qDebug() << "Drop Event : Depth Drop: " << x << ", " << y;
		}

		if (x < 0 || y < 0) {
			return;
		}

		if (x > this->annotateTab->getParent()->ui.graphicsViewAnnotation2->width() || y > this->annotateTab->getParent()->ui.graphicsViewAnnotation2->height()) {
			return;
		}

		(*this->annotateTab->getAnnotations())[this->pointKey].setX(x);
		(*this->annotateTab->getAnnotations())[this->pointKey].setY(y);

		x *= *this->annotateTab->getScalingFactor();
		y *= *this->annotateTab->getScalingFactor();
		qDebug() << "Check: " << x << ". " << y;

		QVector3D vector3D = KinectEngine::getInstance().query3DPoint(x, y, this->annotateTab->getDepthToColorImage());
		
		(*this->annotateTab->getAnnotations3D())[this->pointKey].setX(vector3D.x());
		(*this->annotateTab->getAnnotations3D())[this->pointKey].setY(vector3D.y());
		(*this->annotateTab->getAnnotations3D())[this->pointKey].setZ(vector3D.z());

		QPixmap colorQPixmap, depthQPixmap;
		colorQPixmap = QPixmap::fromImage(*this->annotateTab->getAnnotatedColorImage());
		depthQPixmap = QPixmap::fromImage(*this->annotateTab->getAnnotatedDepthToColorColorizedImage());

		/** Human cut shape */
		//int width = this->annotateTab->getParent()->ui.graphicsViewAnnotation->width();
		//int height = this->annotateTab->getParent()->ui.graphicsViewAnnotation->height();
		//QPixmap humanPixmap(":/DesktopApp/resources/HumanCutShape.png");
		//QPixmap humanPixmapScaled = humanPixmap.scaled(width, height, Qt::KeepAspectRatio);
		/** Human cut shape END */

		QPainter colorPainter(&colorQPixmap);
		QPainter depthPainter(&depthQPixmap);

		// color
		colorPainter.setPen(QPen(Qt::magenta, 10, Qt::SolidLine, Qt::RoundCap));
		for(auto it: *this->annotateTab->getAnnotations()) {
			colorPainter.drawPoint(it.second.x() * this->annotateTab->scalingFactorFromRightToLeft, it.second.y() * this->annotateTab->scalingFactorFromRightToLeft);
		}
		colorPainter.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
		for(auto it: *this->annotateTab->getAnnotations()) {
			colorPainter.drawText(it.second.x() * this->annotateTab->scalingFactorFromRightToLeft, it.second.y() * this->annotateTab->scalingFactorFromRightToLeft, QString::fromStdString(it.first));

		}
		colorPainter.setPen(QPen(Qt::white , 0.5, Qt::DashLine, Qt::RoundCap));
		colorPainter.drawLine(
			(*this->annotateTab->getAnnotations())["A1"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["A1"].y() * this->annotateTab->scalingFactorFromRightToLeft,
			(*this->annotateTab->getAnnotations())["A2"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["A2"].y() * this->annotateTab->scalingFactorFromRightToLeft
		);
		colorPainter.drawLine(
			(*this->annotateTab->getAnnotations())["B1"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["B1"].y() * this->annotateTab->scalingFactorFromRightToLeft,
			(*this->annotateTab->getAnnotations())["B2"].x() * this->annotateTab->scalingFactorFromRightToLeft, (*this->annotateTab->getAnnotations())["B2"].y() * this->annotateTab->scalingFactorFromRightToLeft
		);
		colorPainter.end();
		// color END

		// depthToColor
		depthPainter.setPen(QPen(Qt::magenta, 8, Qt::SolidLine, Qt::RoundCap));
		for (auto it : *this->annotateTab->getAnnotations()) {
			depthPainter.drawPoint(it.second.x(), it.second.y());
		}
		depthPainter.setPen(QPen(Qt::white, 2, Qt::SolidLine, Qt::RoundCap));
		for (auto it : *this->annotateTab->getAnnotations()) {
			depthPainter.drawText(it.second.x(), it.second.y(), QString::fromStdString(it.first));

		}
		depthPainter.setPen(QPen(Qt::white, 0.5, Qt::DashLine, Qt::RoundCap));
		depthPainter.drawLine(
			(*this->annotateTab->getAnnotations())["A1"].x(), (*this->annotateTab->getAnnotations())["A1"].y(),
			(*this->annotateTab->getAnnotations())["A2"].x(), (*this->annotateTab->getAnnotations())["A2"].y()
		);
		depthPainter.drawLine(
			(*this->annotateTab->getAnnotations())["B1"].x(), (*this->annotateTab->getAnnotations())["B1"].y(),
			(*this->annotateTab->getAnnotations())["B2"].x(), (*this->annotateTab->getAnnotations())["B2"].y()
		);
		depthPainter.end();
		// depthToColor END


		this->annotateTab->getColorScene()->addPixmap(colorQPixmap);
		this->annotateTab->getDepthToColorScene()->addPixmap(depthQPixmap);

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
