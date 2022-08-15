#include "reportdialog.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>

ReportDialog::ReportDialog()
	: QDialog()
{
	ui.setupUi(this);

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

}

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