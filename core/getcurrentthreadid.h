#ifndef TRACELIB_GETCURRENTTHREADID_H
#define TRACELIB_GETCURRENTTHREADID_H

#include "tracelib_config.h"

#include <time.h>

TRACELIB_NAMESPACE_BEGIN

typedef unsigned long ProcessId;
typedef unsigned long ThreadId;

time_t getCurrentProcessStartTime();
ProcessId getCurrentProcessId();
ThreadId getCurrentThreadId();

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_GETCURRENTTHREADID_H)

