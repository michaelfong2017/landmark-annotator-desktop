#ifndef CLIPGRAPHICSPIXMAPITEM_H
#define CLIPGRAPHICSPIXMAPITEM_H

#include "stdafx.h"
#include "capturetab.h"

class CaptureTab;

class ClipGraphicsPixmapItem : public QGraphicsPixmapItem {
public:
	ClipGraphicsPixmapItem(const QPixmap& pixmap, CaptureTab* captureTab);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
	CaptureTab* captureTab;

};

#endif