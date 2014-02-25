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
#include <fstream>

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

StreamLogOutput::StreamLogOutput( ostream &stream )
    : m_stream( stream )
{
}

StreamLogOutput::~StreamLogOutput()
{
}

void StreamLogOutput::write( const std::string &msg )
{
    m_stream << "[" << timeToString( now() ) << "] " << msg << endl;
}

FileLogOutput::FileLogOutput( const string &filename )
    : m_fileStream( new std::ofstream( filename.c_str(), ios_base::out | ios_base::trunc ) )
    , m_internalOutput( new StreamLogOutput( *m_fileStream ) )
{
}

FileLogOutput::~FileLogOutput()
{
    delete m_internalOutput;
    delete m_fileStream;
}

bool FileLogOutput::isOpen() const
{
    return m_fileStream->is_open();
}

void FileLogOutput::write( const string &msg )
{
    m_internalOutput->write( msg );
}

std::ostream &FileLogOutput::stream() const
{
    return *m_fileStream;
}

TRACELIB_NAMESPACE_END

