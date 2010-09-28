/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "trace.h"
#include "configuration.h"
#include "crashhandler.h"
#include "filter.h"
#include "output.h"
#include "serializer.h"
#include "tracepoint.h"
#include "errorlog.h"

#include <ctime>
#include <fstream>

using namespace std;

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

TRACELIB_NAMESPACE_BEGIN

static void recordCrashInTrace()
{
    string sourceFile = "<unknown file>";
    size_t lineNumber = 0;
    string functionName = "<unknown function>";

    BacktraceGenerator backtraceGenerator;
    Backtrace *bt = new Backtrace( backtraceGenerator.generate( 8 ) );
    if ( bt->depth() > 0 ) {
        const StackFrame &f = bt->frame( 0 );
        sourceFile = f.sourceFile;
        lineNumber = f.lineNumber;
        functionName = f.function;
    }

    static TracePoint tp( TracePointType::Error, 0,
                          sourceFile.c_str(), lineNumber, functionName.c_str(), 0 );
    TraceEntry te( &tp, "The application crashed at this point!" );
    te.backtrace = bt;
    getActiveTrace()->addEntry( te );

}

const struct CrashHandlerInstaller {
    CrashHandlerInstaller() { installCrashHandler( recordCrashInTrace ); }
} g_crashHandlerInstaller;

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

const TracedProcess TraceEntry::process = {
    getCurrentProcessId(),
    getCurrentProcessStartTime()
};

ProcessShutdownEvent::ProcessShutdownEvent()
    : process( &TraceEntry::process ),
    shutdownTime( std::time( NULL ) )
{
}

TraceEntry::TraceEntry( const TracePoint *tracePoint_, const char *msg )
    : threadId( getCurrentThreadId() ),
    timeStamp( std::time( NULL ) ),
    tracePoint( tracePoint_ ),
    variables( 0 ),
    backtrace( 0 ),
    message( msg ),
    stackPosition( reinterpret_cast<size_t>( &stackPosition ) )
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
    m_configFileMonitor( 0 ),
    m_errorLog( 0 )
{
    if ( getenv( "TRACELIB_DEBUG_LOG" ) ) {
        ofstream *stream = new ofstream( getenv( "TRACELIB_DEBUG_LOG" ), ios_base::out | ios_base::trunc );
        if ( stream->is_open() ) {
            m_errorLog = new StreamErrorLog( stream );
        } else {
            delete stream;
        }
    }

    if ( !m_errorLog ) {
        m_errorLog = new DebugViewErrorLog;
    }

    const string cfgFileName = Configuration::defaultFileName();
    reloadConfiguration( cfgFileName );
    m_configFileMonitor = FileModificationMonitor::create( cfgFileName, this );
    m_configFileMonitor->start();
    ShutdownNotifier::self().addObserver( this );
}

Trace::~Trace()
{
    ShutdownNotifier::self().removeObserver( this );

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
    delete m_errorLog;
}

void Trace::reloadConfiguration( const string &fileName )
{
    Configuration *cfg = Configuration::fromFile( fileName, m_errorLog );
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
        const unsigned int action = ( *it )->actionForTracePoint( tracePoint );
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
        if ( !m_output || ( !m_output->canWrite() && !m_output->open() ) ) {
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
        if ( !m_output || ( !m_output->canWrite() && !m_output->open() ) ) {
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

void Trace::handleProcessShutdown()
{
    ProcessShutdownEvent ev;

    vector<char> data;
    {
        MutexLocker serializerLocker( m_serializerMutex );
        if ( !m_serializer ) {
            return;
        }
        data = m_serializer->serialize( ev );
    }

    if ( !data.empty() ) {
        MutexLocker outputLocker( m_outputMutex );
        if ( !m_output || ( !m_output->canWrite() && !m_output->open() ) ) {
            return;
        }
        m_output->write( data );

        /* Delete the output object to make sure it flushes any data which
         * it might have buffered. We most likely don't need the object anymore
         * anyway - after all, the process is shutting down.
         */
        delete m_output;
        m_output = 0;
    }
}

static Trace *g_activeTrace = 0;

Trace *getActiveTrace()
{
    if ( !g_activeTrace ) {
        setActiveTrace( new Trace );
    }
    return g_activeTrace;
}

void setActiveTrace( Trace *trace )
{
    g_activeTrace = trace;
}

TRACELIB_NAMESPACE_END

