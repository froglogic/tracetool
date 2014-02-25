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

LogOutput::LogOutput()
{
}

LogOutput::~LogOutput()
{
}

Log::Log( LogOutput* statusOutput, LogOutput *errorOutput )
    : m_statusOutput( statusOutput )
    , m_errorOutput( errorOutput )
{
}

void Log::writeStatus( const string &msg )
{
    m_statusOutput->write( msg );
}

void Log::writeError( const string &msg )
{
    m_errorOutput->write( msg );
}

Log::~Log()
{
}

#ifdef _WIN32
void DebugViewLogOutput::write( const string &msg )
{
    // XXX Consider encoding issues (msg is UTF-8).
    OutputDebugStringA( msg.c_str() );
}
#endif

void NullLogOutput::write( const string & /*msg*/ )
{
    // Intentionally left blank.
}

StreamLogOutput::StreamLogOutput( ostream *stream )
    : m_stream( stream )
{
}

StreamLogOutput::~StreamLogOutput()
{
    delete m_stream;
}

void StreamLogOutput::write( const std::string &msg )
{
    ( *m_stream ) << "[" << timeToString( now() ) << "] " << msg << endl;
}

TRACELIB_NAMESPACE_END

