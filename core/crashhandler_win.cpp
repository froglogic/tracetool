#include "crashhandler.h"

#include <windows.h>

static LPTOP_LEVEL_EXCEPTION_FILTER g_prevExceptionFilter = 0;

TRACELIB_NAMESPACE_BEGIN

extern void recordCrashInTrace();

static LONG WINAPI tracelibExceptionFilterProc( LPEXCEPTION_POINTERS ex )
{
    recordCrashInTrace();
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
