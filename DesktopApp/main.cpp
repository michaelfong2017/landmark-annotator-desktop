#include "desktopapp.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include "helper.h"
#include <qtranslator.h>

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

    QTranslator translator;
    QString dir = QString(QCoreApplication::applicationDirPath());
    bool success = translator.load(QString("Translation_zh_CN.qm"), dir);
    if (success) {
        QCoreApplication::installTranslator(&translator);
    }

    QString check = QCoreApplication::translate("DesktopAppClass", "Wukong");
    qDebug() << QCoreApplication::translate("DesktopAppClass", "Wukong");

    DesktopApp w;
    w.show();
    w.setAttribute(Qt::WA_AcceptTouchEvents);
    return a.exec();
}
