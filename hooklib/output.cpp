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

#include "output.h"
#include "log.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

Output::Output()
{
}

Output::~Output()
{
}

void StdoutOutput::write( const vector<char> &data )
{
    vector<char> nullTerminatedData = data;
    nullTerminatedData.push_back( '\0' );
    fprintf(stdout, "%s\n", &nullTerminatedData[0]);
    fflush(stdout);
}

FileOutput::FileOutput( Log *log, const string& filename )
    : m_filename( filename ), m_file( 0 ), m_log( log )
{
}

FileOutput::~FileOutput()
{
    if( m_file ) {
        fclose(m_file);
    }
    m_file = 0;
    m_filename = "";
    m_log = 0;
}

bool FileOutput::canWrite() const
{
    return m_file != 0;
}

bool FileOutput::open()
{
    m_file = fopen( m_filename.c_str(), "w" );
    if( !m_file ) {
        m_log->writeError( "Failed to open file!: %s", strerror( errno ) );
        return false;
    }
    return true;
}

void FileOutput::write( const vector<char> &data )
{
    if( m_file ) {
        vector<char> nullTerminatedData = data;
        nullTerminatedData.push_back( '\0' );
        fprintf( m_file, "%s\n", &nullTerminatedData[0] );
        fflush( m_file );
    }
}

void MultiplexingOutput::addOutput( Output *output )
{
    m_outputs.push_back( output );
}

void MultiplexingOutput::write( const vector<char> &data )
{
    vector<Output *>::const_iterator it, end = m_outputs.end();
    for ( it = m_outputs.begin(); it != end; ++it ) {
        ( *it )->write( data );
    }
}

MultiplexingOutput::~MultiplexingOutput()
{
    vector<Output *>::const_iterator it, end = m_outputs.end();
    for ( it = m_outputs.begin(); it != end; ++it ) {
        delete *it;
    }
}

TRACELIB_NAMESPACE_END

