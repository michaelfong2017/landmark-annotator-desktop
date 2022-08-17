#include "reportdialog.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>

ReportDialog::ReportDialog(AnnotateTab* parent): QDialog(parent)
{
	ui.setupUi(this);
	
	this->parent = parent;

	this->setFixedSize(this->width(), this->height());

	
	ui.variable1Labe2->setText(QString::fromStdString(std::to_string(this->parent->distance1)));
	if (this->parent->distance1 > 0) {
		ui.rightCheckBox1->setChecked(true);
	}
	else if (this->parent->distance1 < 0) {
		ui.leftCheckBox1->setChecked(true);
	}
	else {

	}
	
	ui.variable1Label4->setText(QString::fromStdString(std::to_string(this->parent->angle2)));
	if (this->parent->angle2 > 0) {
		ui.rightCheckBox2->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox2->setChecked(true);
	}
	else {

	}

	ui.variable1Label6->setText(QString::fromStdString(std::to_string(this->parent->angle1)));
	if (this->parent->angle1 > 0) {
		ui.rightCheckBox3->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox3->setChecked(true);
	}
	else {

	}

	ui.variable1Label8->setText(QString::fromStdString(std::to_string(this->parent->trunkRotation)));
	if (this->parent->trunkRotation > 0) {
		ui.rightCheckBox4->setChecked(true);
	}
	else if (this->parent->angle2 < 0) {
		ui.leftCheckBox4->setChecked(true);
	}
	else {

	}


	ui.NameLabel->setText(this->parent->getParent()->patientTab->getCurrentPatientName());
	ui.PhoneLabel->setText(this->parent->getParent()->patientTab->getPhoneNumber());
	ui.GenerLabel->setText(this->parent->getParent()->patientTab->getSex());
	ui.BirthLabel->setText(this->parent->getParent()->patientTab->getDOB());
	ui.AgeLabel->setText(this->parent->getParent()->patientTab->getAge());
	ui.WeightLabel->setText(this->parent->getParent()->patientTab->getWeight());
	ui.HeightLabel->setText(this->parent->getParent()->patientTab->getHeight());

	int reportWidth = this->ui.graphicsView->width();
	int reportHeight = this->ui.graphicsView->height();

	int annotateWidth = this->parent->getParent()->ui.graphicsViewAnnotation2->width();
	int annotateHeight = this->parent->getParent()->ui.graphicsViewAnnotation2->height();

	float scale = std::min((float)reportWidth / annotateWidth, (float)reportHeight / annotateHeight);

	qDebug() << (float)reportWidth / annotateWidth;
	qDebug() << (float)reportHeight / annotateHeight;

	scale -= 0.05;

	ui.graphicsView->setScene(this->parent->getDepthToColorScene());
	ui.graphicsView->scale(scale, scale);
	ui.graphicsView->show();

	/*QObject::connect(ui.pdfButton, &QPushButton::clicked, [this]() {

		QWidget* w = ui.scrollAreaWidgetContents_3;

		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName("report.pdf");
		printer.setPageMargins(5, 10, 8, 16, QPrinter::Millimeter);
		printer.setFullPage(false);

		QPainter painter(&printer);

		double xscale = printer.pageRect().width() / double(w->width());
		double yscale = printer.pageRect().height() / double(w->height());
		double scale = qMin(xscale, yscale);
		painter.translate(double(printer.paperRect().x() + printer.pageRect().width() / 2), double(printer.paperRect().y() + printer.pageRect().height() / 2));
		painter.scale(scale, scale);
		painter.translate(-w->width() / 2, -w->height() / 2);
		w->render(&painter);

		QDialog::accept();
	});*/

	QObject::connect(ui.printButton, &QPushButton::clicked, [this]() {
		printWithPreview();
	});
	
	QObject::connect(ui.clearButton, &QPushButton::clicked, [this]() {
		ui.signView->clearImage();
	});
}

//void ReportDialog::resizeEvent(QResizeEvent* event)
//{
//	event->accept();
//
//	if (event->size().width() > event->size().height()) {
//		QWidget::resize(event->size().height(), event->size().height());
//	}
//	else {
//		QWidget::resize(event->size().width(), event->size().width());
//	}
//}



void ReportDialog::printWithPreview()
{

	QPrinter printer(QPrinter::HighResolution);
	printer.setPageSize(QPrinter::A4);
	printer.setOrientation(QPrinter::Portrait); //打印方向 Portrait 纵向，Landscape：横向
//    printer.setOutputFormat(QPrinter::NativeFormat);

	QPrintPreviewDialog preview(&printer);
	connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printDocument(QPrinter*)));
	preview.setWindowTitle("Preview Dialog");
	preview.setWindowState(Qt::WindowMaximized);
	preview.exec();
}

void ReportDialog::printDocument(QPrinter* printer)
{

	QWidget* w = ui.scrollAreaWidgetContents_3;
	QPainter painter(printer);

	painter.begin(printer);

	double xscale = printer->pageRect().width() / double(w->width());
	double yscale = printer->pageRect().height() / double(w->height());
	double scale = qMin(xscale, yscale);
	painter.translate(double(printer->paperRect().x() + printer->pageRect().width() / 2), 
		double(printer->paperRect().y() + printer->pageRect().height() / 2));
	painter.scale(scale, scale);
	painter.translate(-w->width() / 2, -w->height() / 2);
	w->render(&painter);

	painter.end();
}