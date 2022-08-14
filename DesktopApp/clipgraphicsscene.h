#ifndef CLIPGRAPHICSSCENE_H
#define CLIPGRAPHICSSCENE_H

#include "stdafx.h"
#include "capturetab.h"

class CaptureTab;

class ClipGraphicsScene : public QGraphicsScene {
public:
	ClipGraphicsScene(CaptureTab* captureTab, QGraphicsPixmapItem* pixmapItem);
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	std::map<int, QPointF> landmarksOnScreen;

private:
	CaptureTab* captureTab;
	int pointKey;

};

#endif