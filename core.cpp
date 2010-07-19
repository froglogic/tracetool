#include "tracelib.h"
#include "configuration.h"

using namespace Tracelib;
using namespace std;

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

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

vector<AbstractVariableConverter *> &Tracelib::operator<<( vector<AbstractVariableConverter *> &v,
                                                 AbstractVariableConverter *c )
{
    v.push_back( c );
    return v;
}

TracePointSet::TracePointSet( Filter *filter, unsigned int actions )
    : m_filter( filter ),
    m_actions( actions )
{
}

TracePointSet::~TracePointSet()
{
    delete m_filter;
}

unsigned int TracePointSet::considerTracePoint( const TracePoint *tracePoint )
{
    if ( m_filter && m_filter->acceptsTracePoint( tracePoint ) ) {
        return m_actions;
    }
    return IgnoreTracePoint;
}

Trace::Trace()
    : m_serializer( 0 ),
    m_output( 0 ),
    m_configuration( new Configuration )
{
    m_serializer = m_configuration->configuredSerializer();
    m_output = m_configuration->configuredOutput();
    m_tracePointSets = m_configuration->configuredTracePointSets();
}

Trace::~Trace()
{
    delete m_serializer;
    delete m_output;
    deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
    delete m_configuration;
}

void Trace::reconsiderTracePoint( TracePoint *tracePoint ) const
{
    tracePoint->lastUsedConfiguration = m_configuration;

    if ( m_tracePointSets.empty() ) {
        tracePoint->active = true;
        return;
    }

    tracePoint->active = false;

    vector<TracePointSet *>::const_iterator it, end = m_tracePointSets.end();
    for ( it = m_tracePointSets.begin(); it != end; ++it ) {
        const int action = ( *it )->considerTracePoint( tracePoint );
        if ( action == TracePointSet::IgnoreTracePoint ) {
            continue;
        }

        tracePoint->active = true;
        tracePoint->backtracesEnabled = ( action & TracePointSet::YieldBacktrace ) == TracePointSet::YieldBacktrace;
        tracePoint->variableSnapshotEnabled = ( action & TracePointSet::YieldVariables ) == TracePointSet::YieldVariables;
        return;
    }
}

void Trace::addEntry( const TraceEntry &entry )
{
    if ( !m_serializer || !m_output ) {
        return;
    }

    if ( !m_output->canWrite() ) {
        return;
    }

    vector<char> data = m_serializer->serialize( entry );
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

