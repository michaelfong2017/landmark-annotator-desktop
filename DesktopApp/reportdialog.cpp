#include "reportdialog.h"

ReportDialog::ReportDialog()
	: QDialog()
{
	ui.setupUi(this);

	QObject::connect(ui.okButton, &QPushButton::clicked, [this]() {
		QWidget* w = this;
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName("output.pdf");
		printer.setPageMargins(12, 16, 12, 20, QPrinter::Millimeter);
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
		});
}
