#ifndef DEVICEMOVINGDIALOG_H
#define DEVICEMOVINGDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_devicemovingdialog.h"
#include "stdafx.h"
#include "capturetab.h"

class DeviceMovingDialog : public QDialog
{
	Q_OBJECT

public:
	DeviceMovingDialog(CaptureTab* parent = nullptr);
	Ui::DeviceMovingDialogUI ui;

private:
	CaptureTab* parent;
};

#endif
