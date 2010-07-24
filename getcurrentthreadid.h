#ifndef TRACELIB_GETCURRENTTHREADID_H
#define TRACELIB_GETCURRENTTHREADID_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

typedef unsigned int ProcessId;
typedef unsigned int ThreadId;

ProcessId getCurrentProcessId();
ThreadId getCurrentThreadId();

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_GETCURRENTTHREADID_H)

