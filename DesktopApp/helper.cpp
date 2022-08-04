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

QString Helper::convertFetchedDateTime(QString text)
{
    if (text == "") {
        return "ERROR";
    }

    QString dateStr = text.split("T")[0];
    QString timeStr = text.split("T")[1].split(".")[0];

    QDate date(dateStr.split("-")[0].toInt(), dateStr.split("-")[1].toInt(), dateStr.split("-")[2].toInt());
    QTime time(timeStr.split(":")[0].toInt(), timeStr.split(":")[1].toInt(), timeStr.split(":")[2].toInt());
    QDateTime localTime = QDateTime(date, time, Qt::UTC).toLocalTime();

    return localTime.toString("yyyy-MM-dd HH:mm:ss");
}
