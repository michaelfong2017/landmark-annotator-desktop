#ifndef DESKTOPAPP_H
#define DESKTOPAPP_H

#define MAX_IMAGE_QUEUE_SIZE 10
// #define KINECT_CAMERA_FPS 30

#include <QtWidgets/QWidget>
#include "ui_desktopapp.h"
#include <k4a/k4a.hpp>
#include "stdafx.h"
#include "patient.h"

// Forward declaration to break circular dependency
// Since DesktopApp have member variables of type <PatientData | View | Capture | Annotate | Alignment>Tab
// And each tab classes has member variable of type DesktopApp
class LoginTab;
class PatientListTab;
class PatientTab;
class CaptureTab;
class AnnotateTab;

enum TabIndex { LOGINTAB = 0, PATIENTLISTTAB = 1, PATIENTTAB = 2, CAPTURETAB = 3, ANNOTATETAB = 4 };

class DesktopApp : public QWidget
{
    Q_OBJECT

public:
    DesktopApp(QWidget *parent = Q_NULLPTR);

    Ui::DesktopAppClass ui;
    
    LoginTab* loginTab;
    PatientListTab* patientListTab;
    PatientTab* patientTab;
    CaptureTab* captureTab;
    AnnotateTab* annotateTab;

    std::queue<k4a_image_t> irImageQueue;

    QDir savePath;
    /** Handle Chinese name when saving video */
    QDir tempVideoSavePath;
    /** Handle Chinese name when saving video END */

    bool isOfflineMode = false;
    void logOut();

    void setTextOnGraphicsViews(std::string text);
    QImage getQIRImage();

    QWidget* loginTabEmail;
    QWidget* loginTabPhone;

protected:
    void resizeEvent(QResizeEvent* event) override;
};

#endif
