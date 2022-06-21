#ifndef TWOLINESDIALOG_H
#define TWOLINESDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_twolinesdialog.h"
#include "stdafx.h"

class TwoLinesDialog: public QDialog
{
	Q_OBJECT

public:
	TwoLinesDialog();
	Ui::TwoLinesDialogUI ui;
	void setLine1(QString text);
	void setLine2(QString text);

private:
};

#endif
