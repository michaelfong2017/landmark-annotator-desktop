#include "helper.h"

QString Helper::getCurrentDateTimeString() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString currentDateTimeString = dateTime.toString("yyyy-MM-dd_HHmmss");

    return currentDateTimeString;
}

QString Helper::getCurrentDateString() {
    QDateTime dateTime = dateTime.currentDateTime();
    QString currentDateString = dateTime.toString("yyyy-MM-dd");

    return currentDateString;
}

QString Helper::getVisitFolderPath(QDir parentDir) {
    /** Create directory if not exists */
    QDir dir(parentDir);
    if (!dir.exists())
        dir.mkpath(".");
    /** Create directory if not exists END */

    QString visitFolder = Helper::getCurrentDateString();

    if (parentDir.cd(visitFolder)) {
        return parentDir.absolutePath();
    }
    else 
    {
        parentDir.mkdir(visitFolder);
        parentDir.cd(visitFolder);
        return parentDir.absolutePath();
    }
}
