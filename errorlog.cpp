#include "errorlog.h"

#ifdef _WIN32
#  include <windows.h>
#endif

using namespace std;

TRACELIB_NAMESPACE_BEGIN

ErrorLog::ErrorLog()
{
}

ErrorLog::~ErrorLog()
{
}

void DebugViewErrorLog::write( const string &msg )
{
    // XXX Consider encoding issues (msg is UTF-8).
#ifdef _WIN32
    OutputDebugStringA( msg.c_str() );
#endif
}

TRACELIB_NAMESPACE_END

