#ifndef CLIPGRAPHICSPIXMAPITEM_H
#define CLIPGRAPHICSPIXMAPITEM_H

#include "stdafx.h"

class ClipGraphicsPixmapItem : public QGraphicsPixmapItem {
public:
	ClipGraphicsPixmapItem(const QPixmap& pixmap);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

};

#endif