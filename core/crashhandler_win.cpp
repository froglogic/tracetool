#include "crashhandler.h"
#include "backtrace.h"
#include "tracelib.h"

#include <string>

#include <windows.h>

static LPTOP_LEVEL_EXCEPTION_FILTER g_prevExceptionFilter = 0;

TRACELIB_NAMESPACE_BEGIN

static LONG WINAPI tracelibExceptionFilterProc( LPEXCEPTION_POINTERS ex )
{
    BacktraceGenerator backtraceGenerator;
    Backtrace *bt = new Backtrace( backtraceGenerator.generate( 8 ) );

    const StackFrame &f = bt->frame( 0 );

    static TracePoint tp( TracePoint::ErrorPoint, 0,
                          f.sourceFile.c_str(), f.lineNumber, f.function.c_str() );
    TraceEntry te( &tp, "The application crashed at this point!" );
    te.backtrace = bt;
    getActiveTrace()->addEntry( te );

    if ( g_prevExceptionFilter ) {
        return g_prevExceptionFilter( ex );
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void installCrashHandler()
{
    static bool crashHandlerInstalled = false;
    if ( !crashHandlerInstalled ) {
        crashHandlerInstalled = true;
        g_prevExceptionFilter = ::SetUnhandledExceptionFilter( tracelibExceptionFilterProc );
    }
}

TRACELIB_NAMESPACE_END
