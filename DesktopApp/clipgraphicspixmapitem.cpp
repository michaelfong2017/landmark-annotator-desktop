#include "clipgraphicspixmapitem.h"

ClipGraphicsPixmapItem::ClipGraphicsPixmapItem(const QPixmap& pixmap, CaptureTab* captureTab)
{
	setPixmap(pixmap);
	this->captureTab = captureTab;
}

void ClipGraphicsPixmapItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	QPixmap pixmap = this->pixmap();
	QRect rect = pixmap.rect();

	painter->drawPixmap(rect, pixmap);

	/** Disable cropping if selected display image is either qDepthImage or qColorToDepthImage */
	if (this->captureTab->getParent()->ui.radioButton2->isChecked() || this->captureTab->getParent()->ui.radioButton3->isChecked()) {
		return;
	}
	/** Disable cropping if selected display image is either qDepthImage or qColorToDepthImage END */

	//painter->setPen(QPen(QBrush(Qt::red), 4, Qt::DashLine));
	//painter->drawRect(this->captureTab->clip_rect);

	//painter->setPen(QPen(QBrush(Qt::blue), 4, Qt::SolidLine));
	////painter->drawRect(this->captureTab->clip_rect);
	//for (int i = 0; i < 4; i++) {
	//	painter->drawRect(this->captureTab->corner(i));
	//}

	//painter->setClipRect(this->captureTab->clip_rect);
}
