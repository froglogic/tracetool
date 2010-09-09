/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "output.h"

#include <stdio.h>

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

