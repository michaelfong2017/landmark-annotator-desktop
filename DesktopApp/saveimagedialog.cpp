#include "saveimagedialog.h"

SaveImageDialog::SaveImageDialog(CaptureTab* parent, bool autoSave)
	: QDialog(parent)
{
	ui.setupUi(this);

	this->parent = parent;

	/** UI init */
	ui.warningLabel->setText(QString());

	QString visitFolderPath = Helper::getVisitFolderPath(this->parent->getParent()->savePath);
	ui.imageSavePath->setText("<html><head/><body><p word-wrap=break-word align=\"center\">Images will be saved under<br>" + visitFolderPath + "</p><br/></body></html>");
	ui.imageSavePath->setWordWrap(true);
	/** UI init END */

	if (autoSave) {
		onAutoSave();
		QDialog::accept();
	}

	QObject::connect(ui.buttonBoxCancel, &QDialogButtonBox::rejected, [this]() {
		QDialog::reject();
	});


	QObject::connect(ui.buttonBoxSave, &QDialogButtonBox::accepted, [this]() {
		if (!ui.checkBoxColor->isChecked() && !ui.checkBoxDepth->isChecked() && !ui.checkBoxColorToDepth->isChecked() && !ui.checkBoxDepthToColor->isChecked() && !ui.checkBoxFourChannel->isChecked()) {
			ui.warningLabel->setText("<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; color:#ff0000; \">Please tick at least one checkbox, or press Cancel.</span></p></body></html>");
			return;
		}

		onManualSave();
		QDialog::accept();
	});
}

void SaveImageDialog::onManualSave() {
	QString dateTimeString = Helper::getCurrentDateTimeString();
	QString visitFolderPath = Helper::getVisitFolderPath(this->parent->getParent()->savePath);
	QString colorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_color.png"));
	QString depthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_depth.png"));
	QString colorToDepthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_color_aligned.png"));
	QString depthToColorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_depth_aligned.png"));
	QString fourChannelPNGSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_four_channel.png"));

	QString chosenColorSavePath = colorSavePath;
	QString chosenDepthSavePath = depthSavePath;
	QString chosenColorToDepthSavePath = colorToDepthSavePath;
	QString chosenDepthToColorSavePath = depthToColorSavePath;
	QString chosenFourChannelPNGSavePath = fourChannelPNGSavePath;

	bool colorWriteSuccess = false;
	bool depthWriteSuccess = false;
	bool colorToDepthWriteSuccess = false;
	bool depthToColorWriteSuccess = false;
	bool fourChannelPNGWriteSuccess = false;

	if (ui.checkBoxColor->isChecked()) {
		chosenColorSavePath = QFileDialog::getSaveFileName(this, tr("Save Color Image"),
			colorSavePath,
			tr("Images (*.png *.jpg)"));
		QImageWriter writer1(chosenColorSavePath);

		//cv::Mat mat = this->parent->getCapturedColorImage();
		//colorWriteSuccess = cv::imwrite(colorSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedColorImage().data,
			this->parent->getCapturedColorImage().cols,
			this->parent->getCapturedColorImage().rows,
			this->parent->getCapturedColorImage().step,
			QImage::Format_RGB32);
		colorWriteSuccess = writer1.write(img);
	}
	if (ui.checkBoxDepth->isChecked()) {
		chosenDepthSavePath = QFileDialog::getSaveFileName(this, tr("Save Depth Image"),
			depthSavePath,
			tr("Images (*.png *.jpg)"));
		QImageWriter writer2(chosenDepthSavePath);

		//cv::Mat i = this->parent->getCapturedDepthImage();
		//depthWriteSuccess = cv::imwrite(depthSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedDepthImage().data,
			this->parent->getCapturedDepthImage().cols,
			this->parent->getCapturedDepthImage().rows,
			QImage::Format_Grayscale16);
		depthWriteSuccess = writer2.write(img);
	}
	if (ui.checkBoxColorToDepth->isChecked()) {
		chosenColorToDepthSavePath = QFileDialog::getSaveFileName(this, tr("Save Color To Depth Image"),
			colorToDepthSavePath,
			tr("Images (*.png *.jpg)"));
		QImageWriter writer3(chosenColorToDepthSavePath);

		//cv::Mat i = this->parent->getCapturedColorToDepthImage();
		//cv::Mat temp;
		//cvtColor(i, temp, cv::COLOR_BGRA2BGR);
		//colorToDepthWriteSuccess = cv::imwrite(colorToDepthSavePath.toStdString(), temp);
		QImage img((uchar*)this->parent->getCapturedColorToDepthImage().data,
			this->parent->getCapturedColorToDepthImage().cols,
			this->parent->getCapturedColorToDepthImage().rows,
			QImage::Format_RGB32);
		colorToDepthWriteSuccess = writer3.write(img);
	}

	if (ui.checkBoxDepthToColor->isChecked()) {
		chosenDepthToColorSavePath = QFileDialog::getSaveFileName(this, tr("Save Depth To Color Image"),
			depthToColorSavePath,
			tr("Images (*.png *.jpg)"));
		QImageWriter writer4(chosenDepthToColorSavePath);

		//cv::Mat i = this->parent->getCapturedDepthToColorImage();
		//depthToColorWriteSuccess = cv::imwrite(depthToColorSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedDepthToColorImage().data,
			this->parent->getCapturedDepthToColorImage().cols,
			this->parent->getCapturedDepthToColorImage().rows,
			this->parent->getCapturedDepthToColorImage().step,
			QImage::Format_Grayscale16);
		depthToColorWriteSuccess = writer4.write(img);
	}

	if (ui.checkBoxFourChannel->isChecked()) {
		chosenFourChannelPNGSavePath = QFileDialog::getSaveFileName(this, tr("Save Four Channel Image"),
			fourChannelPNGSavePath,
			tr("Images (*.png *.jpg)"));
		QImageWriter writer5(chosenFourChannelPNGSavePath);

		QImage img((uchar*)this->parent->getFourChannelPNG().data,
			this->parent->getFourChannelPNG().cols,
			this->parent->getFourChannelPNG().rows,
			this->parent->getFourChannelPNG().step,
			QImage::Format_RGBA64);
		fourChannelPNGWriteSuccess = writer5.write(img);
	}

	/** "Images saved under" */
	if (!colorWriteSuccess && !depthWriteSuccess && !colorToDepthWriteSuccess && !depthToColorWriteSuccess && !fourChannelPNGWriteSuccess) {
		if (this->parent->getCaptureFilepath() == QString()) {
			qDebug() << "no capture filepath exists";
			this->parent->getParent()->ui.saveInfoCaptureTab->setText("Something went wrong, cannot save images.");
		}
		else qDebug() << "has capture filepath before";
		QDialog::reject();
		return;
	}

	/** "Images saved under" END */

	/** Show In Explorer */
	if (colorWriteSuccess) this->parent->setCaptureFilepath(chosenColorSavePath);
	else if (depthWriteSuccess) this->parent->setCaptureFilepath(chosenDepthSavePath);
	else if (colorToDepthWriteSuccess) this->parent->setCaptureFilepath(chosenColorToDepthSavePath);
	else if (depthToColorWriteSuccess) this->parent->setCaptureFilepath(chosenDepthToColorSavePath);
	else this->parent->setCaptureFilepath(chosenFourChannelPNGSavePath);

	this->parent->getParent()->ui.saveInfoCaptureTab->setText("Images are saved under\n" + visitFolderPath + "\nat " + dateTimeString);
	this->parent->getParent()->ui.showInExplorer->show();
	/** Show In Explorer END */
}

void SaveImageDialog::onAutoSave() {
	QString dateTimeString = Helper::getCurrentDateTimeString();
	QString visitFolderPath = Helper::getVisitFolderPath(this->parent->getParent()->savePath);
	QString colorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_color.png"));
	QString depthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_depth.png"));
	QString colorToDepthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_color_aligned.png"));
	QString depthToColorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_depth_aligned.png"));
	QString fourChannelPNGSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(dateTimeString.toStdString() + "_four_channel.png"));

	qDebug() << "colorSavePath" << colorSavePath;

	bool colorWriteSuccess = false;
	bool depthWriteSuccess = false;
	bool colorToDepthWriteSuccess = false;
	bool depthToColorWriteSuccess = false;
	bool fourChannelPNGWriteSuccess = false;

	QImageWriter writer1(colorSavePath);
	QImageWriter writer2(depthSavePath);
	QImageWriter writer3(colorToDepthSavePath);
	QImageWriter writer4(depthToColorSavePath);
	QImageWriter writer5(fourChannelPNGSavePath);

	if (ui.checkBoxColor->isChecked()) {
		//cv::Mat mat = this->parent->getCapturedColorImage();
		//colorWriteSuccess = cv::imwrite(colorSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedColorImage().data,
			this->parent->getCapturedColorImage().cols,
			this->parent->getCapturedColorImage().rows,
			this->parent->getCapturedColorImage().step,
			QImage::Format_RGB32);
		colorWriteSuccess = writer1.write(img);
	}
	if (ui.checkBoxDepth->isChecked()) {
		//cv::Mat i = this->parent->getCapturedDepthImage();
		//depthWriteSuccess = cv::imwrite(depthSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedDepthImage().data,
			this->parent->getCapturedDepthImage().cols,
			this->parent->getCapturedDepthImage().rows,
			QImage::Format_Grayscale16);
		depthWriteSuccess = writer2.write(img);
	}
	if (ui.checkBoxColorToDepth->isChecked()) {
		//cv::Mat i = this->parent->getCapturedColorToDepthImage();
		//cv::Mat temp;
		//cvtColor(i, temp, cv::COLOR_BGRA2BGR);
		//colorToDepthWriteSuccess = cv::imwrite(colorToDepthSavePath.toStdString(), temp);
		QImage img((uchar*)this->parent->getCapturedColorToDepthImage().data,
			this->parent->getCapturedColorToDepthImage().cols,
			this->parent->getCapturedColorToDepthImage().rows,
			QImage::Format_RGB32);
		colorToDepthWriteSuccess = writer3.write(img);
	}

	if (ui.checkBoxDepthToColor->isChecked()) {
		//cv::Mat i = this->parent->getCapturedDepthToColorImage();
		//depthToColorWriteSuccess = cv::imwrite(depthToColorSavePath.toStdString(), i);
		QImage img((uchar*)this->parent->getCapturedDepthToColorImage().data,
			this->parent->getCapturedDepthToColorImage().cols,
			this->parent->getCapturedDepthToColorImage().rows,
			this->parent->getCapturedDepthToColorImage().step,
			QImage::Format_Grayscale16);
		depthToColorWriteSuccess = writer4.write(img);
	}

	if (ui.checkBoxFourChannel->isChecked()) {
		QImage img((uchar*)this->parent->getFourChannelPNG().data,
			this->parent->getFourChannelPNG().cols,
			this->parent->getFourChannelPNG().rows,
			this->parent->getFourChannelPNG().step,
			QImage::Format_RGBA64);
		fourChannelPNGWriteSuccess = writer5.write(img);
	}

	/** "Images saved under" */
	if (!colorWriteSuccess && !depthWriteSuccess && !colorToDepthWriteSuccess && !depthToColorWriteSuccess && !fourChannelPNGWriteSuccess) {
		if (this->parent->getCaptureFilepath() == QString()) {
			qDebug() << "no capture filepath exists";
			this->parent->getParent()->ui.saveInfoCaptureTab->setText("Something went wrong, cannot save images.");
		}
		else qDebug() << "has capture filepath before";
		QDialog::reject();
		return;
	}

	/** "Images saved under" END */

	/** Show In Explorer */
	if (colorWriteSuccess) this->parent->setCaptureFilepath(colorSavePath);
	else if (depthWriteSuccess) this->parent->setCaptureFilepath(depthSavePath);
	else if (colorToDepthWriteSuccess) this->parent->setCaptureFilepath(colorToDepthSavePath);
	else if (depthToColorWriteSuccess) this->parent->setCaptureFilepath(depthToColorSavePath);
	else this->parent->setCaptureFilepath(fourChannelPNGSavePath);

	this->parent->getParent()->ui.saveInfoCaptureTab->setText("Images are saved under\n" + visitFolderPath + "\nat " + dateTimeString);
	this->parent->getParent()->ui.showInExplorer->show();
	/** Show In Explorer END */
}