/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "getcurrentthreadid.h"
#include "timehelper.h" // for now()

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <assert.h>

TRACELIB_NAMESPACE_BEGIN

uint64_t getCurrentProcessStartTime()
{
    // ### BUG: starts counting as of first call only
    static uint64_t t0 = now();
    return t0;
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

