#include "getcurrentthreadid.h"

#include <windows.h>

TRACELIB_NAMESPACE_BEGIN

ProcessId getCurrentProcessId()
{
    return (ProcessId)::GetCurrentProcessId();
}

ThreadId getCurrentThreadId()
{
    return (ThreadId)::GetCurrentThreadId();
}

TRACELIB_NAMESPACE_END

