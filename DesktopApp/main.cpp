#include "mainwindow.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include "helper.h"
#include "translationhelper.h"

#include "cameramanager.h"

#include "window.h"
#include "libobsensor/hpp/Pipeline.hpp"
#include "libobsensor/hpp/Error.hpp"

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

void orbbecTest() {
    ob::Pipeline pipe;
    std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();

    std::shared_ptr<ob::VideoStreamProfile> colorProfile = nullptr;
    try {
        auto profiles = pipe.getStreamProfileList(OB_SENSOR_COLOR);
        try {
            colorProfile = profiles->getVideoStreamProfile(1280, 0, OB_FORMAT_RGB, 30);
        }
        catch (ob::Error& e) {
            colorProfile = std::const_pointer_cast<ob::StreamProfile>(profiles->getProfile(0))->as<ob::VideoStreamProfile>();
        }
        config->enableStream(colorProfile);
    }
    catch (ob::Error& e) {
        std::cerr << "Current device is not support color sensor!" << std::endl;
        exit(EXIT_FAILURE);
    }

    pipe.start(config);

    Window app("ColorViewer", colorProfile->width(), colorProfile->height());

    try {
        if (pipe.getDevice()->isPropertySupported(OB_PROP_COLOR_MIRROR_BOOL, OB_PERMISSION_WRITE)) {
            pipe.getDevice()->setBoolProperty(OB_PROP_COLOR_MIRROR_BOOL, true);
        }
    }
    catch (const ob::Error& e) {
        std::cerr << "Failed to set mirror property: " << e.getMessage() << std::endl;
    }

    while (app) {
        auto frameSet = pipe.waitForFrames(100);
        if (frameSet == nullptr) {
            continue;
        }

        app.render({ frameSet->colorFrame() }, RENDER_SINGLE);
    }

    pipe.stop();
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

    //orbbecTest();

    MainWindow w;
    w.show();
    return a.exec();
}
