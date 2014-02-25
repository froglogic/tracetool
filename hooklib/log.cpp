/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "log.h"
#include "timehelper.h" // for now and timeToString

#ifdef _WIN32
#  include <windows.h>
#endif

#include <ostream>

#include <ctime>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

Log::Log()
{
}

Log::~Log()
{
}

#ifdef _WIN32
void DebugViewLog::write( const string &msg )
{
    // XXX Consider encoding issues (msg is UTF-8).
    OutputDebugStringA( msg.c_str() );
}
#endif

void NullLog::write( const string & /*msg*/ )
{
    // Intentionally left blank.
}

StreamLog::StreamLog( ostream *stream )
    : m_stream( stream )
{
}

StreamLog::~StreamLog()
{
    delete m_stream;
}

void StreamLog::write( const std::string &msg )
{
    ( *m_stream ) << "[" << timeToString( now() ) << "] " << msg << endl;
}

TRACELIB_NAMESPACE_END

