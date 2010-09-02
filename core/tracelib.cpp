#include "tracelib.h"
#include "trace.h"

TRACELIB_NAMESPACE_BEGIN

void visitTracePoint( TracePoint *tracePoint,
                      const char *msg,
                      VariableSnapshot *variables )
{
    getActiveTrace()->visitTracePoint( tracePoint, msg, variables );
}

TRACELIB_NAMESPACE_END

