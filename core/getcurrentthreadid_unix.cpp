#include "getcurrentthreadid.h"

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <assert.h>

TRACELIB_NAMESPACE_BEGIN

time_t getCurrentProcessStartTime()
{
    assert( !"getCurrentProcessStartTime not implemented on Unix!" );
    return -1;
}

ProcessId getCurrentProcessId()
{
    return (ProcessId)::getpid();
}

ThreadId getCurrentThreadId()
{
    return (ThreadId)::pthread_self();
}

TRACELIB_NAMESPACE_END

