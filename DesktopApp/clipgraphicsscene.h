#ifndef CLIPGRAPHICSSCENE_H
#define CLIPGRAPHICSSCENE_H

#include "stdafx.h"

class ClipGraphicsScene : public QGraphicsScene {
public:
	ClipGraphicsScene();
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void dropEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

};

#endif