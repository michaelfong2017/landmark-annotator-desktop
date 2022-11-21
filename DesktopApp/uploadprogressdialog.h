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
	// https://stackoverflow.com/questions/35681215/avoid-memory-leakage-when-removing-object-pointer-from-the-map
	// If you really do need to dynamically allocate the Request, for whatever reason, 
	// instead of storing a raw pointer, you could store a smart pointer,
	// which will automatically delete the pointed-to object when it is destroyed. 
	// The default choice for this would be std::unique_ptr.
	std::map<int, std::unique_ptr<uploadrequest>> requests;
	// Then you again only need to do myMap.erase(it_req);; no manual delete to worry about.
	UploadProgressDialog();
	Ui::UploadProgressDialogUI ui;
	int latestUploadNumber = 0; // Start from 1 (increment from 0 to 1 when first use)
	void onUploading(int uploadNumber);
	void onCompleted(int uploadNumber);
	void onFailed(int uploadNumber);
	void updateRowStatus(int uploadNumber, QString newStatus, QColor color);
	void addRow(int uploadNumber, QString patientName, int captureNumber);
	std::vector<int> completedUploadNumbers;

private:
	QTableView* tableView;
	QStandardItemModel* dataModel;
	const int COLUMN_COUNT = 4;

private slots:
	void onSlotRowDoubleClicked(const QModelIndex& index);
};

#endif
