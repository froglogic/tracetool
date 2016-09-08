/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
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

#include "getcurrentthreadid.h"

#include <windows.h>

static uint64_t filetimeToUInt64( const FILETIME &ft )
{
    // windows epoch starts at 1601-01-01T00:00:00Z
    static const uint64_t MSEC_TO_UNIX_EPOCH = 11644473600000ULL;
    // windows ticks are in 100ns
    static const uint64_t WINDOWS_TICK_MS = 10000ULL;
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    return ull.QuadPart / WINDOWS_TICK_MS - MSEC_TO_UNIX_EPOCH;
}

TRACELIB_NAMESPACE_BEGIN

uint64_t getCurrentProcessStartTime()
{
    FILETIME creationTime, dummyTime;
    ::GetProcessTimes( ::GetCurrentProcess(), &creationTime, &dummyTime, &dummyTime, &dummyTime );
    return filetimeToUInt64( creationTime );
}

ProcessId getCurrentProcessId()
{
    return (ProcessId)::GetCurrentProcessId();
}

ThreadId getCurrentThreadId()
{
    return (ThreadId)::GetCurrentThreadId();
}

TRACELIB_NAMESPACE_END

