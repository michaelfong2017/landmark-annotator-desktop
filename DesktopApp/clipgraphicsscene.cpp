#include "clipgraphicsscene.h"
#include <QTouchEvent>

ClipGraphicsScene::ClipGraphicsScene(CaptureTab* captureTab, QGraphicsPixmapItem* pixmapItem) {
	this->captureTab = captureTab;

	this->addItem(pixmapItem);

	qDebug() << "ClipGraphicsScene()";

}

void ClipGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	float x = event->scenePos().x(), y = event->scenePos().y();

	this->pointKey = -1;

	qint32 p1x;
	qint32 p1y;

	qint32 p2x;
	qint32 p2y;

	this->captureTab->clip_rect.getCoords(&p1x, &p1y, &p2x, &p2y);
	
	this->landmarksOnScreen[0] = QPointF(p1x, p1y); // top left
	this->landmarksOnScreen[1] = QPointF(p2x, p1y); // top right
	this->landmarksOnScreen[2] = QPointF(p2x, p2y); // bottom right
	this->landmarksOnScreen[3] = QPointF(p1x, p2y); // bottom left
	
	//qDebug() << "mousePressEvent (P1) = (" << p1x << "," << p1y << ")";
	//qDebug() << "mousePressEvent (P2) = (" << p2x << "," << p2y << ")";

	std::map<int, QPointF>::iterator it;

	for (it = this->landmarksOnScreen.begin(); it != this->landmarksOnScreen.end(); it++)
	{
		if (abs(it->second.x() - x) <= 10 && abs(it->second.y() - y) <= 10) {
			this->pointKey = it->first;
			break;
		}
	}

	//qDebug() << "1:" << this->landmarksOnScreen[1].x() << " " << this->landmarksOnScreen[1].y();

	//qDebug() << "mousePressEvent (x,y) = (" << x << "," << y << ")";

}

void ClipGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	float x = event->scenePos().x(), y = event->scenePos().y();

	//qDebug() << "mouseMoveEvent (x,y) = (" << x << "," << y << ")";

	switch (this->pointKey) {
		case 0:
			this->captureTab->clip_rect.setTopLeft(QPoint(x, y));
			break;
		case 1:
			this->captureTab->clip_rect.setTopRight(QPoint(x, y));
			break;
		case 2:
			this->captureTab->clip_rect.setBottomRight(QPoint(x, y));
			break;
		case 3:
			this->captureTab->clip_rect.setBottomLeft(QPoint(x, y));
			break;
		default:
			break;
	}

	update();
}

void ClipGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	float x = event->scenePos().x(), y = event->scenePos().y();

	//qDebug() << "mouseReleaseEvent (x,y) = (" << x << "," << y << ")";

	if (this->pointKey != -1) {

		qint32 p1x;
		qint32 p1y;

		qint32 p2x;
		qint32 p2y;

		this->captureTab->clip_rect.getCoords(&p1x, &p1y, &p2x, &p2y);

		this->landmarksOnScreen[0] = QPointF(p1x, p1y); // top left
		this->landmarksOnScreen[1] = QPointF(p2x, p1y); // top right
		this->landmarksOnScreen[2] = QPointF(p2x, p2y); // bottom right
		this->landmarksOnScreen[3] = QPointF(p1x, p2y); // bottom left
	}

	this->pointKey = -1;

	update();
}

bool ClipGraphicsScene::event(QEvent* e){
	switch (e->type()) {
		case QEvent::TouchBegin:
			qDebug() << "touch!";
			return true;
		default:
			// call base implementation
			return QGraphicsScene::event(e);
	}
}