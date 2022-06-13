#ifndef LANDMARKSCONFIRMEDDIALOG_H
#define LANDMARKSCONFIRMEDDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_landmarksconfirmeddialog.h"
#include "stdafx.h"

class LandmarksConfirmedDialog: public QDialog
{
	Q_OBJECT

public:
	LandmarksConfirmedDialog();
	Ui::LandmarksConfirmedDialogUI ui;

private:
};

#endif
