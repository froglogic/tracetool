/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2013-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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
