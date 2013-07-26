/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "output.h"
#include "errorlog.h"

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
}

FileOutput::FileOutput( ErrorLog *errorLog, const string& filename )
    : m_filename( filename ), m_file( 0 ), m_errorLog( errorLog )
{
}

FileOutput::~FileOutput()
{
    if( m_file ) {
        fclose(m_file);
    }
    m_file = 0;
    m_filename = "";
    m_errorLog = 0;
}

bool FileOutput::canWrite() const
{
    return m_file != 0;
}

bool FileOutput::open()
{
    m_file = fopen( m_filename.c_str(), "w" );
    if( !m_file ) {
        m_errorLog->write( "Failed to open file!: %s", strerror( errno ) );
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

