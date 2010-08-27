#include "crashhandler.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

void installCrashHandler()
{
    static bool crashHandlerInstalled = false;
    if ( !crashHandlerInstalled ) {
        crashHandlerInstalled = true;
        // XXX Implement me
        assert( !"installCrashHandler not implemented on Unix!" );
    }
}

TRACELIB_NAMESPACE_END
