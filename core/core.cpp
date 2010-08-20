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

const ProcessId TraceEntry::processId = getCurrentProcessId();

TraceEntry::TraceEntry( const TracePoint *tracePoint_, const char *msg )
    : threadId( getCurrentThreadId() ),
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
    m_configuration( 0 ),
    m_configFileMonitor( 0 )
{
    const string cfgFileName = Configuration::defaultFileName();
    reloadConfiguration( cfgFileName );
    m_configFileMonitor = FileModificationMonitor::create( cfgFileName, this );
}

Trace::~Trace()
{
    {
        MutexLocker serializerLocker( m_serializerMutex );
        delete m_serializer;
    }

    {
        MutexLocker outputLocker( m_outputMutex );
        delete m_output;
    }

    {
        MutexLocker configurationLocker( m_configurationMutex );
        deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
        delete m_configuration;
    }

    delete m_configFileMonitor;
}

void Trace::reloadConfiguration( const string &fileName )
{
    Configuration *cfg = Configuration::fromFile( fileName );
    if ( cfg ) {
        {
            MutexLocker serializerLocker( m_serializerMutex );
            delete m_serializer;
            m_serializer = cfg->configuredSerializer();
        }
        {
            MutexLocker outputLocker( m_outputMutex );
            delete m_output;
            m_output = cfg->configuredOutput();
        }
        {
            MutexLocker configurationLocker( m_configurationMutex );
            deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
            m_tracePointSets = cfg->configuredTracePointSets();
            delete m_configuration;
            m_configuration = cfg;
        }
    } else {
        {
            MutexLocker serializerLocker( m_serializerMutex );
            delete m_serializer;
            m_serializer = 0;
        }
        {
            MutexLocker outputLocker( m_outputMutex );
            delete m_output;
        }
        {
            MutexLocker configurationLocker( m_configurationMutex );
            deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
            delete m_configuration;
            m_configuration = 0;
        }
    }
}

void Trace::configureTracePoint( TracePoint *tracePoint ) const
{
    MutexLocker configurationLocker( m_configurationMutex );
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

    if ( !tracePoint->active || !m_serializer || !m_output ) {
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
    vector<char> data;
    {
        MutexLocker serializerLocker( m_serializerMutex );
        if ( !m_serializer ) {
            return;
        }
        data = m_serializer->serialize( entry );
    }

    if ( !data.empty() ) {
        MutexLocker outputLocker( m_outputMutex );
        if ( !m_output || !m_output->canWrite() ) {
            return;
        }
        m_output->write( data );
    }
}

void Trace::setSerializer( Serializer *serializer )
{
    MutexLocker serializerLocker( m_serializerMutex );
    delete m_serializer;
    m_serializer = serializer;
}

void Trace::setOutput( Output *output )
{
    MutexLocker outputLocker( m_outputMutex );
    delete m_output;
    m_output = output;
}

void Trace::handleFileModification( const std::string &fileName, NotificationReason reason )
{
    reloadConfiguration( fileName );
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

