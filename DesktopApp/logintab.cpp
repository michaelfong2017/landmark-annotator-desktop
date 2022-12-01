﻿#include "logintab.h"
#include "librealsense2/rs.hpp"
#include <QtUiTools/quiloader.h>

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

		QUiLoader loader;
		QFile file(":/DesktopApp/twolinesdialog.ui");
		file.open(QFile::ReadOnly);
		QDialog* myWidget = (QDialog*)loader.load(&file, this);
		file.close();

		//QVBoxLayout* layout = new QVBoxLayout;
		//layout->addWidget(myWidget);
		//setLayout(layout);
		//QMainWindow mw;
		//mw.setCentralWidget(myWidget);
		//mw.show();
		this->parent->hide();
		QDialogButtonBox* buttonBox = myWidget->findChild<QDialogButtonBox*>("buttonBox");
		QObject::connect(buttonBox, &QDialogButtonBox::accepted, [myWidget]() {
			qDebug() << "buttonBox pressed";
			myWidget->accept();
			});

		myWidget->show();

		//return;

		// Test realsense2
		rs2::context ctx;
		auto list = ctx.query_devices(); // Get a snapshot of currently connected devices
		int device_count = list.size();

		rs2::device front, back;

		if (device_count == 0)
			throw std::runtime_error("No device detected. Is it plugged in?");
		else if (device_count == 1)
			front = list.front();
		else if (device_count == 2) {
			front = list.front();
			back = list.back();
		}


		// 建構一個RealSense抽象設備的管道以容納擷取到的影像

		rs2::pipeline p;

		// 創建自定義參數以配置管道

		rs2::config cfg;

		// 設定影像尺寸(寬w，高h)

		const int w = 1280;

		const int h = 720;

		// 設定欲顯示的影像流(可依需求啟動不一定要全設)

		cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_BGRA8, 30); // 8-bit blue, green and red channels + constant alpha channel equal to FF 30fps

		cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, 30); // 16 bit格式灰階深度影像 30fps

		cfg.enable_stream(RS2_STREAM_INFRARED, 1, w, h, RS2_FORMAT_Y8, 30); // 8 bit格式左紅外線影像 30fps

		cfg.enable_stream(RS2_STREAM_INFRARED, 2, w, h, RS2_FORMAT_Y8, 30); // 8 bit格式右紅外線影像 30fps

		// 根據設定值啟動指定串流影像

		p.start(cfg);

		// Find first depth sensor (devices can have zero or more then one)
	//    auto sensor = selection.get_device().first<rs2::depth_sensor>();
	//    auto scale =  sensor.get_depth_scale();

		// Block program until frames arrive
		rs2::frameset frames = p.wait_for_frames();

		rs2::frame color = frames.get_color_frame();            // Find the color data

		// Declare depth colorizer for enhanced color visualization of depth data
		rs2::colorizer color_map;
		rs2::frame depth = color_map.process(frames.get_depth_frame()); // Find and colorize the depth data

		// Query frame size (width and height)
		int width, height;

		width = color.as<rs2::video_frame>().get_width();
		height = color.as<rs2::video_frame>().get_height();
		cv::Mat colorCVImage(height, width, CV_8UC4, (void*)color.get_data(), cv::Mat::AUTO_STEP);


		width = depth.as<rs2::video_frame>().get_width();
		height = depth.as<rs2::video_frame>().get_height();
		cv::Mat depthCVImage(height, width, CV_8UC3, (void*)depth.get_data(), cv::Mat::AUTO_STEP);
		cv::cvtColor(depthCVImage, depthCVImage, cv::COLOR_RGB2BGRA);


		bool success;
		success = Helper::saveCVImage(colorCVImage, "d455_color_test.png", QImage::Format_RGB32);


		success = Helper::saveCVImage(depthCVImage, "d455_depth_test.png", QImage::Format_RGB32);

		//// Get the depth frame's dimensions
		//float width = depth.get_width();
		//float height = depth.get_height();

		//// Query the distance from the camera to the object in the center of the image
		//float dist_to_center = depth.get_distance(width / 2, height / 2);

		//// Print the distance
		//qDebug() << "Michael here";
		//qDebug() << "The camera is facing an object " << dist_to_center << " meters away \r";
		// Test realsense2 END

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
