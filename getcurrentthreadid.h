#ifndef TRACELIB_GETCURRENTTHREADID_H
#define TRACELIB_GETCURRENTTHREADID_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

typedef unsigned long ProcessId;
typedef unsigned long ThreadId;

ProcessId getCurrentProcessId();
ThreadId getCurrentThreadId();

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_GETCURRENTTHREADID_H)

