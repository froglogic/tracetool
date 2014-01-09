/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "errorlog.h"
#include "timehelper.h" // for now and timeToString

#ifdef _WIN32
#  include <windows.h>
#else
#  include <stdio.h>
#endif

#include <ostream>

#include <ctime>

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
#else
    fprintf( stderr, "%s\n", msg.c_str() );
#endif
}

StreamErrorLog::StreamErrorLog( ostream *stream )
    : m_stream( stream )
{
}

StreamErrorLog::~StreamErrorLog()
{
    delete m_stream;
}

void StreamErrorLog::write( const std::string &msg )
{
    ( *m_stream ) << "[" << timeToString( *m_stream, now() ) << "] " << msg << endl;
}

TRACELIB_NAMESPACE_END

