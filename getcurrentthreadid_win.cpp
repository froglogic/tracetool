#include "getcurrentthreadid.h"

#include <windows.h>

TRACELIB_NAMESPACE_BEGIN

ThreadId getCurrentThreadId()
{
    return (ThreadId)::GetCurrentThreadId();
}

TRACELIB_NAMESPACE_END

