#include "crashhandler.h"

#include <windows.h>

static LPTOP_LEVEL_EXCEPTION_FILTER g_prevExceptionFilter = 0;

TRACELIB_NAMESPACE_BEGIN

static CrashHandler g_handler;

static LONG WINAPI tracelibExceptionFilterProc( LPEXCEPTION_POINTERS ex )
{
    (*g_handler)();
    if ( g_prevExceptionFilter ) {
        return g_prevExceptionFilter( ex );
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void installCrashHandler( CrashHandler handler )
{
    static bool crashHandlerInstalled = false;
    if ( !crashHandlerInstalled ) {
        crashHandlerInstalled = true;
        g_handler = handler;
        g_prevExceptionFilter = ::SetUnhandledExceptionFilter( tracelibExceptionFilterProc );
    }
}

TRACELIB_NAMESPACE_END
