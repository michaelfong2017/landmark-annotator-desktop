#include "logintab.h"
#include "qnetworkclient.h"
#include "reportdialog.h"

LoginTab::LoginTab(DesktopApp* parent)
{
	this->parent = parent;

	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setEchoMode(QLineEdit::Password);

	QSettings settings("Wukong", "Wukong");
	QString username = settings.value("login/username").toString();
	QString password = settings.value("login/password").toString();

	if (username != "" && password != "") {
		this->parent->ui.rememberCheckBox->setChecked(true);
	}

	this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->setText(username);
	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setText(password);

	QObject::connect(parent->ui.loginTab->findChild<QPushButton*>("loginButton"), &QPushButton::clicked, [this]() {
		QString username = this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->text();
		QString password = this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->text();

		//qDebug() << username << password;

		if (this->parent->ui.rememberCheckBox->isChecked()) {
			QSettings settings("Wukong", "Wukong");
			settings.setValue("login/username", username);
			settings.setValue("login/password", password);
		}
		else {
			QSettings settings("Wukong", "Wukong");
			settings.setValue("login/username", "");
			settings.setValue("login/password", "");
		}

		QNetworkClient::getInstance().login(this->parent->ui.tabWidget, username, password);

		//// Testing
		//ReportDialog dialog;
		//dialog.exec();
	});
}

DesktopApp* LoginTab::getParent()
{
	return this->parent;
}
