#include "tracelib.h"
#include "configuration.h"

#include <ctime>

using namespace std;

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

TRACELIB_NAMESPACE_BEGIN

TracePointSet::TracePointSet( Filter *filter, unsigned int actions )
    : m_filter( filter ),
    m_actions( actions )
{
}

TracePointSet::~TracePointSet()
{
    delete m_filter;
}

unsigned int TracePointSet::actionForTracePoint( const TracePoint *tracePoint )
{
    if ( m_filter && m_filter->acceptsTracePoint( tracePoint ) ) {
        return m_actions;
    }
    return IgnoreTracePoint;
}

TraceEntry::TraceEntry( const TracePoint *tracePoint_, const char *msg )
    : processId( getCurrentProcessId() ),
    threadId( getCurrentThreadId() ),
    timeStamp( std::time( NULL ) ),
    tracePoint( tracePoint_ ),
    backtrace( 0 ),
    variables( 0 ),
    message( msg )
{
}

TraceEntry::~TraceEntry()
{
    if ( variables ) {
        deleteRange( variables->begin(), variables->end() );
    }
    delete variables;
    delete backtrace;
}

Trace::Trace()
    : m_serializer( 0 ),
    m_output( 0 ),
    m_configuration( Configuration::fromFile( Configuration::defaultFileName() ) )
{
    if ( m_configuration ) {
        m_serializer = m_configuration->configuredSerializer();
        m_output = m_configuration->configuredOutput();
        m_tracePointSets = m_configuration->configuredTracePointSets();
    }
}

Trace::~Trace()
{
    delete m_serializer;
    {
        MutexLocker outputLocker( m_outputMutex );
        delete m_output;
    }
    deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
    delete m_configuration;
}

void Trace::configureTracePoint( TracePoint *tracePoint ) const
{
    tracePoint->lastUsedConfiguration = m_configuration;

    if ( m_tracePointSets.empty() ) {
        tracePoint->active = true;
        return;
    }

    tracePoint->active = false;

    vector<TracePointSet *>::const_iterator it, end = m_tracePointSets.end();
    for ( it = m_tracePointSets.begin(); it != end; ++it ) {
        const int action = ( *it )->actionForTracePoint( tracePoint );
        if ( action == TracePointSet::IgnoreTracePoint ) {
            continue;
        }

        tracePoint->active = true;
        tracePoint->backtracesEnabled = ( action & TracePointSet::YieldBacktrace ) == TracePointSet::YieldBacktrace;
        tracePoint->variableSnapshotEnabled = ( action & TracePointSet::YieldVariables ) == TracePointSet::YieldVariables;
        return;
    }
}

void Trace::visitTracePoint( TracePoint *tracePoint,
                             const char *msg,
                             VariableSnapshot *variables )
{
    if ( tracePoint->lastUsedConfiguration != m_configuration ) {
        configureTracePoint( tracePoint );
    }

    if ( !tracePoint->active || !m_serializer ) {
        return;
    }

    {
        MutexLocker outputLocker( m_outputMutex );
        if ( !m_output || !m_output->canWrite() ) {
            return;
        }
    }

    TraceEntry entry( tracePoint, msg );
    if ( tracePoint->backtracesEnabled ) {
        entry.backtrace = new Backtrace( m_backtraceGenerator.generate( 1 /* omit this function in backtrace */ ) );
    }

    if ( tracePoint->variableSnapshotEnabled ) {
        entry.variables = variables;
    }

    addEntry( entry );
}

void Trace::addEntry( const TraceEntry &entry )
{
    if ( !m_serializer ) {
        return;
    }

    {
        MutexLocker outputLocker( m_outputMutex );
        if ( !m_output || !m_output->canWrite() ) {
            return;
        }
    }

    vector<char> data = m_serializer->serialize( entry );
    if ( !data.empty() ) {
        MutexLocker outputLocker( m_outputMutex );
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
    MutexLocker outputLocker( m_outputMutex );
    delete m_output;
    m_output = output;
}

static Trace *g_activeTrace = 0;

Trace *getActiveTrace()
{
    if ( !g_activeTrace ) {
        static Trace defaultTrace;
        return &defaultTrace;
    }
    return g_activeTrace;
}

void setActiveTrace( Trace *trace )
{
    g_activeTrace = trace;
}

TRACELIB_NAMESPACE_END

