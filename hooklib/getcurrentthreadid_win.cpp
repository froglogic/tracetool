/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

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

