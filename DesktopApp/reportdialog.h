#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_reportdialog.h"
#include "stdafx.h"

class ReportDialog : public QDialog
{
	Q_OBJECT

public:
	ReportDialog();
	Ui::ReportDialogUI ui;

private:
};

#endif
