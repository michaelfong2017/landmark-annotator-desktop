#include "clipgraphicsscene.h"

ClipGraphicsScene::ClipGraphicsScene(CaptureTab* captureTab, QGraphicsPixmapItem* pixmapItem) {
	this->captureTab = captureTab;

	this->addItem(pixmapItem);

	qDebug() << "ClipGraphicsScene()";
}

void ClipGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	float x = event->scenePos().x(), y = event->scenePos().y();

	qDebug() << "mousePressEvent (x,y) = (" << x << "," << y << ")";

}

void ClipGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	float x = event->scenePos().x(), y = event->scenePos().y();

	qDebug() << "mouseMoveEvent (x,y) = (" << x << "," << y << ")";

	this->captureTab->clip_rect.setTopLeft(QPoint(x, y));
}

void ClipGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	float x = event->scenePos().x(), y = event->scenePos().y();

	qDebug() << "mouseReleaseEvent (x,y) = (" << x << "," << y << ")";

	update();
}
