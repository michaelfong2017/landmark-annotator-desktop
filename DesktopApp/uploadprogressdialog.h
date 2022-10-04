#ifndef UPLOADPROGRESSDIALOG_H
#define UPLOADPROGRESSDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_uploadprogressdialog.h"
#include "stdafx.h"

class uploadrequest;

class UploadProgressDialog : public QDialog
{
	Q_OBJECT

public:
	std::map<int, uploadrequest*> requests;
	UploadProgressDialog();
	Ui::UploadProgressDialogUI ui;
	int latestUploadNumber = 0; // Start from 1 (increment from 0 to 1 when first use)
	void onUploading(QString patientName, int captureNumber);
	void onCompleted(int uploadNumber);
	void onFailed(int uploadNumber);
	void RetryAllFailedAttempts();

private:
	QTableView* tableView;
	QStandardItemModel* dataModel;
	const int COLUMN_COUNT = 4;
};

#endif
