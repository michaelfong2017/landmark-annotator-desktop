#pragma once

#include <QtWidgets/QDialog>
#include "ui_loadingdialog.h"
#include "stdafx.h"
#include "qnetworkclient.h"

class LoadingDialog : public QDialog
{
	Q_OBJECT

public:
	LoadingDialog(QWidget *parent = Q_NULLPTR);
	void SetBarValue(int i);

private:
	Ui::LoadingDialog ui;
};
