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
class PatientDataTab;
class ViewTab;
class CaptureTab;
class AnnotateTab;
class AlignmentTab;

class DesktopApp : public QWidget
{
    Q_OBJECT

public:
    DesktopApp(QWidget *parent = Q_NULLPTR);

    Ui::DesktopAppClass ui;
    
    PatientDataTab* patientDataTab;
    ViewTab* viewTab;
    CaptureTab* captureTab;
    AnnotateTab* annotateTab;
    AlignmentTab* alignmentTab;

    std::queue<k4a_image_t> irImageQueue;

    Patient patient;
    QDir savePath;

    void setTextOnGraphicsViews(std::string text);
    QImage getQIRImage();
};

#endif
