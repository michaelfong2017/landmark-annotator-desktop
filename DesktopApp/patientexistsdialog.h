#ifndef PATIENTEXISTSDIALOG_H
#define PATIENTEXISTSDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_patientexistsdialog.h"
#include "stdafx.h"

class PatientExistsDialog : public QDialog
{
	Q_OBJECT

public:
	PatientExistsDialog();
	Ui::PatientExistsDialogUI ui;

private:
};

#endif
