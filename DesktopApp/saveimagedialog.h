#ifndef SAVEIMAGEDIALOG_H
#define SAVEIMAGEDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_saveimagedialog.h"
#include "stdafx.h"
#include "capturetab.h"

class SaveImageDialog : public QDialog
{
	Q_OBJECT

public:
	SaveImageDialog(CaptureTab *parent = nullptr, bool autoSave = false, bool savePointCloud = false);
	Ui::SaveImageDialogUI ui;
	void SaveImageDialog::onManualSave();
	void SaveImageDialog::onAutoSave();

private:
	CaptureTab *parent;
	bool savePointCloud = false;
};

#endif
