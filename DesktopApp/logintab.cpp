#include "logintab.h"
#include "librealsense2/rs.hpp"
#include <QtUiTools/quiloader.h>
#include <cameramanager.h>
#include <realsensecamera.h>

LoginTab::LoginTab(DesktopApp* parent)
{
	this->parent = parent;
	QSettings settings("Wukong", "Wukong");

	/** init email login */
	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setEchoMode(QLineEdit::Password);

	QString username = settings.value("login/username").toString();
	QString password = settings.value("login/password").toString();

	if (username != "" && password != "") {
		this->parent->ui.rememberCheckBox->setChecked(true);
	}

	this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->setText(username);
	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setText(password);

	QObject::connect(parent->ui.switchLoginButton, &QPushButton::clicked, [this]() {
		this->parent->ui.tabWidget->insertTab(TabIndex::LOGINTAB, this->parent->loginTabPhone, tr("Phone Login"));
		this->parent->ui.tabWidget->setCurrentIndex(TabIndex::LOGINTAB);
		this->parent->ui.tabWidget->removeTab(TabIndex::LOGINTAB + 1);
		});

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

		});

	QObject::connect(parent->ui.offlineModeButton, &QPushButton::clicked, [this]() {
		startOfflineMode();
		});

	/** init email login END */

	/** init phone login */
	this->parent->ui.passwordLineEdit_2->setEchoMode(QLineEdit::Password);

	// Full phone number is stored and retrieved
	QString phoneFull = settings.value("login/phoneFull").toString();
	QString passwordPhone = settings.value("login/passwordPhone").toString();

	if (phoneFull != "" && passwordPhone != "") {
		this->parent->ui.rememberCheckBox_2->setChecked(true);
	}

	if (phoneFull.split("-").size() == 2) {
		QString areaCode = "+" + phoneFull.split("-")[0];
		QString number = phoneFull.split("-")[1];

		QComboBox* comboBox = this->parent->ui.comboBox;
		int index = comboBox->findText(areaCode);
		if (index != -1) { // -1 for not found
			comboBox->setCurrentIndex(index);
		}
		this->parent->ui.usernameLineEdit_2->setText(number);
		this->parent->ui.passwordLineEdit_2->setText(passwordPhone);
	}


	QObject::connect(parent->ui.switchLoginButton_2, &QPushButton::clicked, [this]() {
		this->parent->ui.tabWidget->insertTab(TabIndex::LOGINTAB, this->parent->loginTabEmail, tr("Email Login"));
		this->parent->ui.tabWidget->setCurrentIndex(TabIndex::LOGINTAB);
		this->parent->ui.tabWidget->removeTab(TabIndex::LOGINTAB + 1);
		});

	QObject::connect(parent->ui.loginButton_2, &QPushButton::clicked, [this]() {
		QComboBox* comboBox = this->parent->ui.comboBox;
		QString areaCode = comboBox->currentText();
		QString number = this->parent->ui.usernameLineEdit_2->text();
		QString fullPhone = areaCode.remove("+") + "-" + number;
		QString password = this->parent->ui.passwordLineEdit_2->text();

		qDebug() << fullPhone << password;

		if (this->parent->ui.rememberCheckBox_2->isChecked()) {
			QSettings settings("Wukong", "Wukong");
			settings.setValue("login/phoneFull", fullPhone);
			settings.setValue("login/passwordPhone", password);
		}
		else {
			QSettings settings("Wukong", "Wukong");
			settings.setValue("login/phoneFull", "");
			settings.setValue("login/passwordPhone", "");
		}

		QNetworkClient::getInstance().login(this->parent->ui.tabWidget, fullPhone, password);
		});


	QObject::connect(parent->ui.offlineModeButton_2, &QPushButton::clicked, [this]() {
		startOfflineMode();
		});
	/** init phone login END */

}

DesktopApp* LoginTab::getParent()
{
	return this->parent;
}

void LoginTab::onLanguageChanged()
{
	/** Remembered username and password */
	QSettings settings("Wukong", "Wukong");


	QString username = settings.value("login/username").toString();
	QString password = settings.value("login/password").toString();

	if (username != "" && password != "") {
		this->parent->ui.rememberCheckBox->setChecked(true);
	}

	this->parent->ui.loginTab->findChild<QLineEdit*>("usernameLineEdit")->setText(username);
	this->parent->ui.loginTab->findChild<QLineEdit*>("passwordLineEdit")->setText(password);



	// Full phone number is stored and retrieved
	QString phoneFull = settings.value("login/phoneFull").toString();
	QString passwordPhone = settings.value("login/passwordPhone").toString();

	if (phoneFull != "" && passwordPhone != "") {
		this->parent->ui.rememberCheckBox_2->setChecked(true);
	}

	if (phoneFull.split("-").size() == 2) {
		QString areaCode = "+" + phoneFull.split("-")[0];
		QString number = phoneFull.split("-")[1];

		QComboBox* comboBox = this->parent->ui.comboBox;
		int index = comboBox->findText(areaCode);
		if (index != -1) { // -1 for not found
			comboBox->setCurrentIndex(index);
		}
		this->parent->ui.usernameLineEdit_2->setText(number);
		this->parent->ui.passwordLineEdit_2->setText(passwordPhone);
	}
	/** Remembered username and password END */
}

bool LoginTab::isEmailLogin(QString account)
{
	if (account.contains("@")) {
		return true;
	}
	return false;
}

void LoginTab::startOfflineMode() {
	qDebug() << "offlineModeButton clicked";

	//QUiLoader loader;
	//QFile file(":/DesktopApp/desktopapp.ui");
	//file.open(QFile::ReadOnly);
	//QWidget* myWidget = loader.load(&file, this);
	//file.close();

	//this->parent->ui.tabWidget->removeTab(0);

	//QWidget* newLoginTab = myWidget->findChild<QWidget*>("loginTab");
	//this->parent->ui.tabWidget->insertTab(0, newLoginTab, "New Login");
	//this->parent->ui.tabWidget->setCurrentIndex(TabIndex::LOGINTAB);
	//myWidget->show();

	//return;

	// Test realsense2
//	rs2::context ctx;
//	auto list = ctx.query_devices(); // Get a snapshot of currently connected devices
//	int device_count = list.size();

//	rs2::device front, back;

//	if (device_count == 0)
//		throw std::runtime_error("No device detected. Is it plugged in?");
//	else if (device_count == 1)
//		front = list.front();
//	else if (device_count == 2) {
//		front = list.front();
//		back = list.back();
//	}


//	// 建構一個RealSense抽象設備的管道以容納擷取到的影像

//	rs2::pipeline p;

//	// 創建自定義參數以配置管道

//	rs2::config cfg;

//	// 設定影像尺寸(寬w，高h)

//	const int w = 1280;

//	const int h = 720;

//	// 設定欲顯示的影像流(可依需求啟動不一定要全設)

//	cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGRA8, 30); // 8-bit blue, green and red channels + constant alpha channel equal to FF 30fps

//	cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, 30); // 16 bit格式灰階深度影像 30fps

//	cfg.enable_stream(RS2_STREAM_INFRARED, 1, w, h, RS2_FORMAT_Y8, 30); // 8 bit格式左紅外線影像 30fps

//	cfg.enable_stream(RS2_STREAM_INFRARED, 2, w, h, RS2_FORMAT_Y8, 30); // 8 bit格式右紅外線影像 30fps

//	// 根據設定值啟動指定串流影像

//	p.start(cfg);

//	// Find first depth sensor (devices can have zero or more then one)
////    auto sensor = selection.get_device().first<rs2::depth_sensor>();
////    auto scale =  sensor.get_depth_scale();

//	// Block program until frames arrive
//	rs2::frameset frames = p.wait_for_frames();

//	rs2::frame color = frames.get_color_frame();            // Find the color data

//	// Declare depth colorizer for enhanced color visualization of depth data
//	rs2::colorizer color_map;
//	rs2::frame depth = color_map.process(frames.get_depth_frame()); // Find and colorize the depth data

//	// Query frame size (width and height)
//	int width, height;

//	width = color.as<rs2::video_frame>().get_width();
//	height = color.as<rs2::video_frame>().get_height();
//	cv::Mat colorCVImage(height, width, CV_8UC4, (void*)color.get_data(), cv::Mat::AUTO_STEP);


//	width = depth.as<rs2::video_frame>().get_width();
//	height = depth.as<rs2::video_frame>().get_height();
//	cv::Mat depthCVImage(height, width, CV_8UC3, (void*)depth.get_data(), cv::Mat::AUTO_STEP);
//	cv::cvtColor(depthCVImage, depthCVImage, cv::COLOR_RGB2BGRA);


//	bool success;
//	success = Helper::saveCVImage(colorCVImage, "d455_color_test.png", QImage::Format_RGB32);


//	success = Helper::saveCVImage(depthCVImage, "d455_depth_test.png", QImage::Format_RGB32);



	//// Get the depth frame's dimensions
	//float width = depth.get_width();
	//float height = depth.get_height();

	//// Query the distance from the camera to the object in the center of the image
	//float dist_to_center = depth.get_distance(width / 2, height / 2);

	//// Print the distance
	//qDebug() << "Michael here";
	//qDebug() << "The camera is facing an object " << dist_to_center << " meters away \r";
	// Test realsense2 END

	//if (!RealsenseEngine::getInstance().isDeviceConnected()) {
	//	TwoLinesDialog dialog;
	//	dialog.setLine1("Kinect device cannot be opened!");
	//	dialog.setLine2("Please check it and try again.");
	//	dialog.exec();
	//	return;
	//}

	//if (!RealsenseEngine::getInstance().isDeviceOpened()) {
	//	RealsenseEngine::getInstance().configDevice();
	//	bool isSuccess = RealsenseEngine::getInstance().openDevice();

	//	if (!isSuccess) {
	//		TwoLinesDialog dialog;
	//		dialog.setLine1("Kinect device cannot be opened!");
	//		dialog.setLine2("Please check it and try again.");
	//		dialog.exec();
	//		return;
	//	}
	//}


	//if (!KinectEngine::getInstance().isDeviceConnected()) {
	//	TwoLinesDialog dialog;
	//	dialog.setLine1("Kinect device cannot be opened!");
	//	dialog.setLine2("Please check it and try again.");
	//	dialog.exec();
	//	return;
	//}

	//if (!KinectEngine::getInstance().isDeviceOpened()) {
	//	KinectEngine::getInstance().configDevice();
	//	bool isSuccess = KinectEngine::getInstance().openDevice();

	//	if (!isSuccess) {
	//		TwoLinesDialog dialog;
	//		dialog.setLine1("Kinect device cannot be opened!");
	//		dialog.setLine2("Please check it and try again.");
	//		dialog.exec();
	//		return;
	//	}
	//}

	// NO NEED to clear capture histories
	////

	camera::CameraManager::getInstance().autoSelectAndOpenCamera();

	// Disable all tabs except capture tab in offline mode
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::CAPTURETAB, true);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTLISTTAB, false);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::PATIENTTAB, false);
	this->parent->ui.tabWidget->setTabEnabled(TabIndex::ANNOTATETAB, false);
	this->parent->ui.tabWidget->setCurrentIndex(TabIndex::CAPTURETAB);
	this->parent->isOfflineMode = true;
	this->parent->captureTab->onEnterOfflineMode();
}
