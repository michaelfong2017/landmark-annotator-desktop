#ifndef HELPER
#define HELPER 

#include "stdafx.h"

class Helper {
public:
	static QString getCurrentDateTimeString();
	static QString getCurrentDateString();
	static QString getVisitFolderPath(QDir);
};

#endif
