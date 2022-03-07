#ifndef DRAGANDDROPGRAPHICSSCENE_H
#define DRAGANDDROPGRAPHICSSCENE_H
#define NUM_ANNOTATIONS 6

#include "stdafx.h"

// Forward Declaration
class AnnotateTab;
enum class ImageType;

class DragAndDropGraphicsScene : public QGraphicsScene {
public:
	DragAndDropGraphicsScene(AnnotateTab* annotateTab, ImageType imageType);
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void dropEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
	void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;

private:
	ImageType imageType;
	AnnotateTab* annotateTab;
	bool isPoint;
	std::string pointKey;
};

#endif