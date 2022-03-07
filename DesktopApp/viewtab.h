#ifndef VIEWTAB_H
#define VIEWTAB_H

#include <QtWidgets/QWidget>
#include "stdafx.h"
#include "desktopapp.h"
#include <k4a/k4a.hpp>
#include <opencv2/opencv.hpp>

class ViewTab : public QWidget
{
    Q_OBJECT

public:
    ViewTab(DesktopApp* parent);
    QTimer* timer;

private:
    DesktopApp* parent;
};

#endif
