#include "clipgraphicspixmapitem.h"

ClipGraphicsPixmapItem::ClipGraphicsPixmapItem(const QPixmap& pixmap) : QGraphicsPixmapItem(pixmap)
{
}

void ClipGraphicsPixmapItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	qDebug() << "paint()";
	QPixmap pixmap = this->pixmap();
	QRect rect = pixmap.rect();

	painter->drawPixmap(rect, pixmap);
}
