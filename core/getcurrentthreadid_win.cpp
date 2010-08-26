#include "getcurrentthreadid.h"

#include <windows.h>

static time_t filetimeToTimeT( const FILETIME &ft )
{
   ULARGE_INTEGER ull;
   ull.LowPart = ft.dwLowDateTime;
   ull.HighPart = ft.dwHighDateTime;

   return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

TRACELIB_NAMESPACE_BEGIN

time_t getCurrentProcessStartTime()
{
    FILETIME creationTime, dummyTime;
    ::GetProcessTimes( ::GetCurrentProcess(), &creationTime, &dummyTime, &dummyTime, &dummyTime );
    return filetimeToTimeT( creationTime );
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

