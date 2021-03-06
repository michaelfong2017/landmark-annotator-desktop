#include "saveimagedialog.h"

SaveImageDialog::SaveImageDialog(CaptureTab* parent)
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

	QObject::connect(ui.buttonBoxCancel, &QDialogButtonBox::rejected, [this]() {
		QDialog::reject();
		});

	QObject::connect(ui.buttonBoxSave, &QDialogButtonBox::accepted, [this]() {
		if (!ui.checkBoxColor->isChecked() && !ui.checkBoxDepth->isChecked() && !ui.checkBoxColorToDepth->isChecked() && !ui.checkBoxDepthToColor->isChecked()) {
			ui.warningLabel->setText("<html><head/><body><p align=\"center\"><span style=\" font-size:12pt; color:#ff0000; \">Please tick at least one checkbox, or press Cancel.</span></p></body></html>");
			return;
		}

		QString dateTimeString = Helper::getCurrentDateTimeString();
		QString visitFolderPath = Helper::getVisitFolderPath(this->parent->getParent()->savePath);
		Patient patient = this->parent->getParent()->patient;
		QString colorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(patient.getSubjectNumber() + "_" + patient.getName() + "_" + dateTimeString.toStdString() + "_color.png"));
		QString depthToColorSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(patient.getSubjectNumber() + "_" + patient.getName() + "_" + dateTimeString.toStdString() + "_depth_aligned.png"));
		QString depthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(patient.getSubjectNumber() + "_" + patient.getName() + "_" + dateTimeString.toStdString() + "_depth.png"));
		QString colorToDepthSavePath = QDir(visitFolderPath).filePath(QString::fromStdString(patient.getSubjectNumber() + "_" + patient.getName() + "_" + dateTimeString.toStdString() + "_color_aligned.png"));

		bool colorWriteSuccess = false;
		bool depthWriteSuccess = false;
		bool colorToDepthWriteSuccess = false;
		bool depthToColorWriteSuccess = false;

		if (ui.checkBoxColor->isChecked()) {
			cv::Mat i = this->parent->getCapturedColorImage();
			colorWriteSuccess = cv::imwrite(colorSavePath.toStdString(), i);
		}
		if (ui.checkBoxDepth->isChecked()) {
			cv::Mat i = this->parent->getCapturedDepthImage();
			cv::imwrite(depthSavePath.toStdString(), i);

			depthWriteSuccess = true;
		}
		if (ui.checkBoxColorToDepth->isChecked()) {
			cv::Mat i = this->parent->getCapturedColorToDepthImage();
			cv::Mat temp;
			cvtColor(i, temp, cv::COLOR_BGRA2BGR);
			colorToDepthWriteSuccess = cv::imwrite(colorToDepthSavePath.toStdString(), temp);
		}

		if (ui.checkBoxDepthToColor->isChecked()) {
			cv::Mat i = this->parent->getCapturedDepthToColorImage();
			depthToColorWriteSuccess = cv::imwrite(depthToColorSavePath.toStdString(), i);
		}

		/** "Images saved under" */
		if (!colorWriteSuccess && !depthWriteSuccess && !colorToDepthWriteSuccess && !depthToColorWriteSuccess) {
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
		else this->parent->setCaptureFilepath(depthToColorSavePath);

		this->parent->getParent()->ui.saveInfoCaptureTab->setText("Images are saved under\n" + visitFolderPath + "\nat " + dateTimeString);
		this->parent->getParent()->ui.showInExplorer->show();
		/** Show In Explorer END */

		QDialog::accept();
		});
}
