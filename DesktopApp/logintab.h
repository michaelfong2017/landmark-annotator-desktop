#ifndef LOGINTAB_H
#define LOGINTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include "qnetworkclient.h"
#include "kinectengine.h"
#include "twolinesdialog.h"
#include "capturetab.h"

class LoginTab : public QWidget
{
    Q_OBJECT

public:
    LoginTab(DesktopApp* parent);
    DesktopApp* getParent();

private:
    DesktopApp* parent;
    bool isEmailLogin(QString account);
    void startOfflineMode();

};

#endif
