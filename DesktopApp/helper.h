#ifndef HELPER
#define HELPER 

#include "stdafx.h"

class Helper {
public:
	static QString getCurrentDateTimeString();
	static QString getCurrentDateString();
	static QString getVisitFolderPath(QDir);
	static QString convertFetchedDateTime(QString);
	static QString dateTimeFilepathToDisplay(QString);
};

#endif
