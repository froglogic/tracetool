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

SnapshotCreator::SnapshotCreator( Trace *trace, unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
    : m_trace( trace ),
    m_verbosity( verbosity ),
    m_sourceFile( sourceFile ),
    m_lineno( lineno ),
    m_functionName( functionName )
{
}

SnapshotCreator::~SnapshotCreator()
{
    m_trace->addEntry( m_verbosity, m_sourceFile, m_lineno, m_functionName, m_variables );
    vector<AbstractVariableConverter *>::iterator it, end = m_variables.end();
    for ( it = m_variables.begin(); it != end; ++it ) {
        delete *it;
    }
}

SnapshotCreator &SnapshotCreator::operator<<( AbstractVariableConverter *converter )
{
    m_variables.push_back( converter );
    return *this;
}

Trace::Trace()
    : m_serializer( 0 ),
    m_filter( 0 ),
    m_output( 0 )
{
}

Trace::~Trace()
{
    delete m_serializer;
    delete m_output;
    delete m_filter;
}

void Trace::addEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const vector<AbstractVariableConverter *> &variables )
{
    if ( !m_serializer || !m_output ) {
        return;
    }

    if ( !m_output->canWrite() ) {
        return;
    }

    if ( m_filter && !m_filter->acceptsEntry( verbosity, sourceFile, lineno, functionName ) ) {
        return;
    }

    vector<char> data = m_serializer->serialize( verbosity, sourceFile, lineno, functionName, variables );
    if ( !data.empty() ) {
        m_output->write( data );
    }
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

void Trace::setFilter( Filter *filter )
{
    delete m_filter;
    m_filter = filter;
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

