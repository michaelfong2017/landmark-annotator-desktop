#ifndef LOGINTAB_H
#define LOGINTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"

class LoginTab : public QWidget
{
    Q_OBJECT

public:
    LoginTab(DesktopApp* parent);
    DesktopApp* getParent();

private:
    DesktopApp* parent;

};

#endif
