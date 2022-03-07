#ifndef PATIENTDATATAB_H
#define PATIENTDATATAB_H

#define DATE_FORMAT "yyyyMMdd"

#include "desktopapp.h"

class PatientDataTab: public QWidget {
public:
	PatientDataTab(DesktopApp* parent);

private:
	DesktopApp* parent;
	bool savePatientData(void);
};

#endif