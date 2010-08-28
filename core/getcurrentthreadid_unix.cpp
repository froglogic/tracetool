#include "getcurrentthreadid.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <assert.h>

TRACELIB_NAMESPACE_BEGIN

static time_t secondsSinceEpoch()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

time_t getCurrentProcessStartTime()
{
    // ### BUG: starts counting as of first call only
    static time_t t0 = secondsSinceEpoch();
    return secondsSinceEpoch() - t0;
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

