#include "errorlog.h"

#ifdef _WIN32
#  include <windows.h>
#endif

using namespace std;
using namespace Tracelib;

ErrorLog::ErrorLog()
{
}

ErrorLog::~ErrorLog()
{
}

void DebugViewErrorLog::write( const string &msg )
{
#ifdef _WIN32
    OutputDebugStringA( msg.c_str() );
#endif
}

