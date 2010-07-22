#include "output.h"

using namespace std;

TRACELIB_NAMESPACE_BEGIN

void StdoutOutput::write( const vector<char> &data )
{
    fprintf(stdout, "%s\n", &data[0]);
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

