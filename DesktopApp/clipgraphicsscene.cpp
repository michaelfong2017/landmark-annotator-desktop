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

	qDebug() << "mouseMoveEvent (x,y) = (" << x << "," << y << ")";

	//qDebug() << "scene width, height:" << this->width() << ", " << this->height();

	if (x < 0) x = 0;
	if (x > this->width()) x = this->width();
	if (y < 0) y = 0;
	if (y > this->height()) y = this->height();

	QRect clip_rect = this->captureTab->clip_rect;
	int minL = 30; // min side length

	int selectedImageIndex = this->captureTab->getSelectedImageIndex();
	CaptureHistory captureHistory = this->captureTab->getCaptureHistories()[selectedImageIndex];
	switch (this->pointKey) {
		case 0:
			if (clip_rect.right() - x < minL) {
				x = clip_rect.right() - minL;
			}
			if (clip_rect.bottom() - y < minL) {
				y = clip_rect.bottom() - minL;
			}
			this->captureTab->clip_rect.setTopLeft(QPoint(x, y));
			captureHistory.clip_rect = this->captureTab->clip_rect;
			this->captureTab->setCaptureHistories(selectedImageIndex, captureHistory);
			qDebug() << this->captureTab->getCaptureHistories()[selectedImageIndex].clip_rect;
			break;
		case 1:
			if (x - clip_rect.left() < minL) {
				x = clip_rect.left() + minL;
			}
			if (clip_rect.bottom() - y < minL) {
				y = clip_rect.bottom() - minL;
			}
			this->captureTab->clip_rect.setTopRight(QPoint(x, y));
			captureHistory.clip_rect = this->captureTab->clip_rect;
			this->captureTab->setCaptureHistories(selectedImageIndex, captureHistory);
			break;
		case 2:
			if (x - clip_rect.left() < minL) {
				x = clip_rect.left() + minL;
			}
			if (y - clip_rect.top() < minL) {
				y = clip_rect.top() + minL;
			}
			this->captureTab->clip_rect.setBottomRight(QPoint(x, y));
			captureHistory.clip_rect = this->captureTab->clip_rect;
			this->captureTab->setCaptureHistories(selectedImageIndex, captureHistory);
			break;
		case 3:
			if (clip_rect.right() - x < minL) {
				x = clip_rect.right() - minL;
			}
			if (y - clip_rect.top() < minL) {
				y = clip_rect.top() + minL;
			}
			this->captureTab->clip_rect.setBottomLeft(QPoint(x, y));
			captureHistory.clip_rect = this->captureTab->clip_rect;
			this->captureTab->setCaptureHistories(selectedImageIndex, captureHistory);
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

//bool ClipGraphicsScene::event(QEvent* e){
//	switch (e->type()) {
//		case QEvent::TouchBegin:
//			mouseMoveEvent(e);
//			return true;
//		case QEvent::TouchEnd:
//			mouseReleaseEvent(e);
//			return true;
//		default:
//			 call base implementation
//			return QGraphicsScene::event(e);
//	}
//}