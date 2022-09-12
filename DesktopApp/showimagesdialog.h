#ifndef SHOWIMAGESDIALOG_H
#define SHOWIMAGESDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_showimagesdialog.h"
#include "stdafx.h"

class ShowImagesDialog : public QDialog
{
	Q_OBJECT

public:
	ShowImagesDialog();
	Ui::ShowImagesDialogUI ui;
	void setQColorImage(QImage image);
	void setQDepthImage1(QImage image);
	void setQDepthImage2(QImage image);

private:
};

#endif
