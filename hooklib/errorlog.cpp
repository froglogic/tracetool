/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "errorlog.h"

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

// XXX duplicated in serializer.cpp
static string timeToString( time_t t )
{
    char timestamp[64] = { '\0' };
    strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S", localtime(&t));
    return string( (const char *)&timestamp );
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
    ( *m_stream ) << "[" << timeToString( std::time( NULL ) ) << "] " << msg << endl;
}

TRACELIB_NAMESPACE_END

