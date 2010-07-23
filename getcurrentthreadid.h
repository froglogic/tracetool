#ifndef TRACELIB_GETCURRENTTHREADID_H
#define TRACELIB_GETCURRENTTHREADID_H

#include "tracelib_config.h"

TRACELIB_NAMESPACE_BEGIN

typedef unsigned int ThreadId;

ThreadId getCurrentThreadId();

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_GETCURRENTTHREADID_H)

