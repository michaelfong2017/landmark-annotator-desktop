#ifndef PATIENTTAB_H
#define PATIENTTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"

class PatientTab : public QWidget
{
    Q_OBJECT

public:
    PatientTab(DesktopApp* parent);
    DesktopApp* getParent();

private:
    DesktopApp* parent;

};

#endif
