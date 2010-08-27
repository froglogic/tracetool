#include "crashhandler.h"

#include "backtrace.h"
#include "tracelib.h"

TRACELIB_NAMESPACE_BEGIN

void recordCrashInTrace()
{
    BacktraceGenerator backtraceGenerator;
    Backtrace *bt = new Backtrace( backtraceGenerator.generate( 8 ) );

    const StackFrame &f = bt->frame( 0 );

    static TracePoint tp( TracePoint::ErrorPoint, 0,
                          f.sourceFile.c_str(), f.lineNumber, f.function.c_str() );
    TraceEntry te( &tp, "The application crashed at this point!" );
    te.backtrace = bt;
    getActiveTrace()->addEntry( te );

}

TRACELIB_NAMESPACE_END

