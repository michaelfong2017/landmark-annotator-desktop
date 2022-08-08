#include "clipgraphicsscene.h"

ClipGraphicsScene::ClipGraphicsScene() {

}

void ClipGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	float x = event->scenePos().x(), y = event->scenePos().y();

	qDebug() << "mousePressEvent (x,y) = (" << x << "," << y << ")";

}

void ClipGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent* event) {
	float x = event->scenePos().x(), y = event->scenePos().y();

	qDebug() << "dropEvent (x,y) = (" << x << "," << y << ")";

	event->acceptProposedAction();
}

void ClipGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event) {
	event->acceptProposedAction();
}

void ClipGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event) {
	event->acceptProposedAction();
}
