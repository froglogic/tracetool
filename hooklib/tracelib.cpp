#include "tracelib.h"
#include "trace.h"

TRACELIB_NAMESPACE_BEGIN

bool advanceVisit( TracePoint *tracePoint )
{
    return getActiveTrace()->advanceVisit( tracePoint );
}

void visitTracePoint( const TracePoint *tracePoint,
                      const char *msg,
                      VariableSnapshot *variables )
{
    getActiveTrace()->visitTracePoint( tracePoint, msg, variables );
}

TRACELIB_NAMESPACE_END

