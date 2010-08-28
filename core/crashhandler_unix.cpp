#include "crashhandler.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

static CrashHandler g_handler;

void installCrashHandler( CrashHandler handler )
{
    static bool crashHandlerInstalled = false;
    if ( !crashHandlerInstalled ) {
        crashHandlerInstalled = true;
        g_handler = handler;
        // XXX Implement me
        assert( !"installCrashHandler not implemented on Unix!" );
    }
}

TRACELIB_NAMESPACE_END
