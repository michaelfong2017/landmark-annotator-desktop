#ifndef ALIGNMENTTAB_H 
#define ALIGNMENTTAB_H 

#include "stdafx.h"
#include "desktopapp.h"
#include "helper.h"

class AlignmentTab : public QWidget
{
    Q_OBJECT

public:
    AlignmentTab(DesktopApp* parent);

private:
    DesktopApp* parent;
    QString colorInputFilename;
    QString depthInputFilename;
    cv::VideoCapture* colorVideoCapture;
    cv::VideoCapture* depthVideoCapture;
    cv::VideoWriter* depthToColorVideoWriter;
};

#endif