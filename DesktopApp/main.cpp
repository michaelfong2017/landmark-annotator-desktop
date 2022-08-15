#include "desktopapp.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DesktopApp w;
    w.show();
    w.setAttribute(Qt::WA_AcceptTouchEvents);
    return a.exec();
}
