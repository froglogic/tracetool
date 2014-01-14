/**********************************************************************
** Copyright (C) 2013 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "timehelper.h"

#include <time.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#define snprintf _snprintf
#  include <sys/types.h> // for _ftime
#  include <sys/timeb.h> // for struct timeb
#else
#  include <sys/time.h> // for gettimeofday
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

std::string timeToString( uint64_t t )
{
    time_t secondsSinceEpoch = t / 1000;
    int mseconds = t % 1000;
    char timestamp[23] = { '\0' };
    strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S", localtime(&secondsSinceEpoch));
    snprintf(&timestamp[18], 5, ":%03d", mseconds);
    return std::string( timestamp );
}

uint64_t now()
{
#ifdef _WIN32
    struct _timeb timebuffer;
    _ftime( &timebuffer );
    return timebuffer.time * 1000 + timebuffer.millitm;
#else
    timeval tv;
    gettimeofday(&tv, 0);
    return ((uint64_t)tv.tv_sec) * 1000 + ((uint64_t)tv.tv_usec) / 1000;
#endif
}

TRACELIB_NAMESPACE_END
