#ifndef CREATENEWPATIENTDIALOG_H
#define CREATENEWPATIENTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_createnewpatientdialog.h"
#include "stdafx.h"
#include "patientlisttab.h"
#include "patient.h"
#include "qnetworkclient.h"

class CreateNewPatientDialog : public QDialog
{
	Q_OBJECT

public:
	CreateNewPatientDialog(PatientListTab* parent = nullptr);
	Ui::CreateNewPatientDialog ui;

private:
	PatientListTab* parent;
	Patient patient;

private slots:
	void onUploadNewPatient(QNetworkReply* reply);
};

#endif
