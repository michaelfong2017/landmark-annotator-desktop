#ifndef PATIENTLISTTAB_H
#define PATIENTLISTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"

class PatientListTab : public QWidget
{
    Q_OBJECT

public:
    PatientListTab(DesktopApp* parent);
    DesktopApp* getParent();

private:
    DesktopApp* parent;

};

#endif
