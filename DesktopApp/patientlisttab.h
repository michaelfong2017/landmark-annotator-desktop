#ifndef PATIENTLISTTAB_H
#define PATIENTLISTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include <QtNetwork>

class PatientListTab : public QWidget
{
    Q_OBJECT

public:
    PatientListTab(DesktopApp* parent);
    DesktopApp* getParent();
    void onEnterTab();

private:
    DesktopApp* parent;

private slots:
    void onFetchPatientList(QNetworkReply* reply);
};

#endif
