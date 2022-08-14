#include "logintab.h"
#include "qnetworkclient.h"

LoginTab::LoginTab(DesktopApp* parent)
{
	this->parent = parent;

	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setEchoMode(QLineEdit::Password);

	QObject::connect(parent->ui.loginTab->findChild<QPushButton*>("loginButton"), &QPushButton::clicked, [this]() {
		QString username = this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->text();
		QString password = this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->text();

		//qDebug() << username << password;

		QNetworkClient::getInstance().login(this->parent->ui.tabWidget, username, password);

        QWidget* w = this->parent->ui.loginTab;
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
	});
}

DesktopApp* LoginTab::getParent()
{
	return this->parent;
}
