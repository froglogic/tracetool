/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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

MultiplexLogOutput::~MultiplexLogOutput()
{
    for( std::set<LogOutput*>::const_iterator it = m_multiplexedOutputs.begin(); it != m_multiplexedOutputs.end(); ++it ) {
        delete *it;
    }
    m_multiplexedOutputs.clear();
}

void MultiplexLogOutput::addOutput( LogOutput *output )
{
    m_multiplexedOutputs.insert( output );
}

void MultiplexLogOutput::write( const std::string &msg )
{
    for( std::set<LogOutput*>::const_iterator it = m_multiplexedOutputs.begin(); it != m_multiplexedOutputs.end(); ++it ) {
        (*it)->write( msg );
    }
}

TRACELIB_NAMESPACE_END

