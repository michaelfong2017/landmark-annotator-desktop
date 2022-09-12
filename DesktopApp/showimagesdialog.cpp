#include "showimagesdialog.h"

ShowImagesDialog::ShowImagesDialog()
	: QDialog()
{
	ui.setupUi(this);

	QObject::connect(ui.buttonBox, &QDialogButtonBox::accepted, [this]() {
		QDialog::accept();
		});
}

void ShowImagesDialog::setQColorImage(QImage image)
{
	QGraphicsView* graphicsView = this->ui.graphicsView;

	int width = graphicsView->width();
	int height = graphicsView->height();

	qDebug() << "graphicsView width and height: " << width << ", " << height;
	QImage imageScaled = image.scaled(width, height, Qt::KeepAspectRatio);

	// Deallocate heap memory used by previous GGraphicsScene object
	if (graphicsView->scene()) {
		delete graphicsView->scene();
	}

	QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
	QGraphicsScene* scene = new QGraphicsScene;
	scene->addItem(pixmapItem);

	graphicsView->setScene(scene);
	graphicsView->show();
}

void ShowImagesDialog::setQDepthImage1(QImage image)
{
	QGraphicsView* graphicsView = this->ui.graphicsView_2;

	int width = graphicsView->width();
	int height = graphicsView->height();

	QImage imageScaled = image.scaled(width, height, Qt::KeepAspectRatio);

	// Deallocate heap memory used by previous GGraphicsScene object
	if (graphicsView->scene()) {
		delete graphicsView->scene();
	}

	QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
	QGraphicsScene* scene = new QGraphicsScene;
	scene->addItem(pixmapItem);

	graphicsView->setScene(scene);
	graphicsView->show();
}

void ShowImagesDialog::setQDepthImage2(QImage image)
{
	QGraphicsView* graphicsView = this->ui.graphicsView_3;

	int width = graphicsView->width();
	int height = graphicsView->height();

	QImage imageScaled = image.scaled(width, height, Qt::KeepAspectRatio);

	// Deallocate heap memory used by previous GGraphicsScene object
	if (graphicsView->scene()) {
		delete graphicsView->scene();
	}

	QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(imageScaled));
	QGraphicsScene* scene = new QGraphicsScene;
	scene->addItem(pixmapItem);

	graphicsView->setScene(scene);
	graphicsView->show();
}
