#include "getcurrentthreadid.h"

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

TRACELIB_NAMESPACE_BEGIN

ProcessId getCurrentProcessId()
{
    return (ProcessId)::getpid();
}

ThreadId getCurrentThreadId()
{
    return (ThreadId)::pthread_self();
}

TRACELIB_NAMESPACE_END

