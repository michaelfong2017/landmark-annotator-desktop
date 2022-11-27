#include "logintab.h"

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

		if (isEmailLogin(username)) {
			QNetworkClient::getInstance().login(this->parent->ui.tabWidget, username, password);
		}
		else {
			QNetworkClient::getInstance().login(this->parent->ui.tabWidget, username, password);
		}
	});

	QObject::connect(parent->ui.offlineModeButton, &QPushButton::clicked, [this]() {
		qDebug() << "offlineModeButton clicked";

		if (!KinectEngine::getInstance().isDeviceConnected()) {
			TwoLinesDialog dialog;
			dialog.setLine1("Kinect device cannot be opened!");
			dialog.setLine2("Please check it and try again.");
			dialog.exec();
			return;
		}

		if (!KinectEngine::getInstance().isDeviceOpened()) {
			KinectEngine::getInstance().configDevice();
			bool isSuccess = KinectEngine::getInstance().openDevice();

			if (!isSuccess) {
				TwoLinesDialog dialog;
				dialog.setLine1("Kinect device cannot be opened!");
				dialog.setLine2("Please check it and try again.");
				dialog.exec();
				return;
			}
		}

		// NO NEED to clear capture histories
		////

		// Disable all tabs except capture tab in offline mode
		this->parent->ui.tabWidget->setTabEnabled(3, true);
		this->parent->ui.tabWidget->setTabEnabled(1, false);
		this->parent->ui.tabWidget->setTabEnabled(2, false);
		this->parent->ui.tabWidget->setTabEnabled(4, false);
		this->parent->ui.tabWidget->setCurrentIndex(3);
		this->parent->isOfflineMode = true;
		this->parent->captureTab->onEnterOfflineMode();
	});
}

DesktopApp* LoginTab::getParent()
{
	return this->parent;
}

bool LoginTab::isEmailLogin(QString account)
{
	if (account.contains("@")) {
		return true;
	}
	return false;
}
