#include "logintab.h"

LoginTab::LoginTab(DesktopApp* parent)
{
	this->parent = parent;

	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setEchoMode(QLineEdit::Password);

	QObject::connect(parent->ui.loginTab->findChild<QPushButton*>("loginButton"), &QPushButton::clicked, [this]() {
		QString username = this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->text();
		QString password = this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->text();

		qDebug() << username << password;
	});
}

DesktopApp* LoginTab::getParent()
{
	return this->parent;
}
