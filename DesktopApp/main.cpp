#include "mainwindow.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include "helper.h"
#include "translationhelper.h"

#include "cameramanager.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    //fprintf(stdout, "%s", msg.toLocal8Bit().constData());

    QString dateString = Helper::getCurrentDateString();
    QString dateTimeString = Helper::getCurrentDateTimeString();

    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("%1 - Debug - %2").arg(dateTimeString).arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("%1 - Warning - %2").arg(dateTimeString).arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("%1 - Critical - %2").arg(dateTimeString).arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("%1 - Fatal - %2").arg(dateTimeString).arg(msg);
        abort();
    }
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    QString logDir = dir.absolutePath() + "/Log";
    if (dir.cd(logDir)) {
    }
    else
    {
        dir.mkdir(logDir);
        dir.cd(logDir);
    }
    QFile outFile(QDir(dir).filePath(dateString));
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageHandler);
    //qInstallMessageHandler(nullptr);

    TranslationHelper::getInstance().useEnglishTranslator();
    //TranslationHelper::getInstance().useSimplifiedChineseTranslator();

    // Test implementing Strategy pattern and multiple inheritance
    //camera::CameraManager::getInstance().setCamera(camera::Model::REALSENSE);
    //camera::CameraManager::getInstance().getCamera()->open();
    //camera::CameraManager::getInstance().getCamera()->startThread();
    //qDebug() << endl << "width is " << camera::CameraManager::getInstance().getConfig()->color_width << endl;
    //camera::CameraManager::getInstance().getCamera()->stopThread();

    //camera::CameraManager::getInstance().setCamera(camera::Model::KINECT);
    //qDebug() << endl << "width is " << camera::CameraManager::getInstance().getConfig()->color_width << endl;
    // Test implementing Strategy pattern and multiple inheritance END

    MainWindow w;
    w.show();
    return a.exec();
}
