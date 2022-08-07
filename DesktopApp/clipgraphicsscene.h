#ifndef CLIPGRAPHICSSCENE_H
#define CLIPGRAPHICSSCENE_H

#include "stdafx.h"
#include "capturetab.h"

class CaptureTab;

class ClipGraphicsScene : public QGraphicsScene {
public:
	ClipGraphicsScene(CaptureTab* captureTab, QGraphicsPixmapItem* pixmapItem);
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void dropEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

private:
	CaptureTab* captureTab;
};

#endif