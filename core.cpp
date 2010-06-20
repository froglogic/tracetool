#include "tracelib.h"

using namespace Tracelib;
using namespace std;

Output::Output()
{
}

Output::~Output()
{
}

Serializer::Serializer()
{
}

Serializer::~Serializer()
{
}

Filter::Filter()
{
}

Filter::~Filter()
{
}

Trace::Trace()
    : m_serializer( 0 ),
    m_output( 0 )
{
}

Trace::~Trace()
{
    delete m_serializer;
    delete m_output;
    vector<Filter *>::iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        delete *it;
    }
}

void Trace::addEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    vector<Filter *>::const_iterator it, end = m_filters.end();
    for ( it = m_filters.begin(); it != end; ++it ) {
        if ( !( *it )->acceptsEntry( verbosity, sourceFile, lineno, functionName ) ) {
            return;
        }
    }
    m_output->write( m_serializer->serialize( verbosity, sourceFile, lineno, functionName ) );
}

void Trace::setSerializer( Serializer *serializer )
{
    delete m_serializer;
    m_serializer = serializer;
}

void Trace::setOutput( Output *output )
{
    delete m_output;
    m_output = output;
}

void Trace::addFilter( Filter *filter )
{
    m_filters.push_back( filter );
}

static Trace *g_activeTrace = 0;

namespace Tracelib
{

Trace *getActiveTrace()
{
    return g_activeTrace;
}

void setActiveTrace( Trace *trace )
{
    g_activeTrace = trace;
}

}

