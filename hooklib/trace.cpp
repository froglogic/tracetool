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
#include "log.h"
#include "tracelib.h" // for deleteRange
#include "timehelper.h" // for now

#include <cstdlib>
#include <ctime>

using namespace std;

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

    static TracePoint tp( TracePointType::Error, sourceFile.c_str(), lineNumber,
                          functionName.c_str(), 0 );
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

TracedProcess TraceEntry::process = {
    getCurrentProcessId(),
    getCurrentProcessStartTime(),
    vector<TraceKey>()
};

ProcessShutdownEvent::ProcessShutdownEvent()
    : process( &TraceEntry::process ),
    shutdownTime( now() )
{
}

TraceEntry::TraceEntry( const TracePoint *tracePoint_, const char *msg )
    : threadId( getCurrentThreadId() ),
    timeStamp( now() ),
    tracePoint( tracePoint_ ),
    variables( 0 ),
    backtrace( 0 ),
    message( msg ),
    stackPosition( reinterpret_cast<size_t>( &stackPosition ) )
{
}

TraceEntry::~TraceEntry()
{
    // variables are deleted on the caller side of the macros so the delete happens with the
    // same C runtime as the allocation
    delete backtrace;
}

static LogOutput* checkForLogFileEnvVar( const char* envVar )
{
    if ( getenv( envVar ) ) {
        FileLogOutput *logOutput = new FileLogOutput( getenv( envVar ) );
        if( logOutput->isOpen() ) {
            return logOutput;
        }
        delete logOutput;
    }
    return 0;
}

Trace::Trace()
    : m_serializer( 0 ),
    m_output( 0 ),
    m_configuration( 0 ),
    m_configFileMonitor( 0 ),
    m_log( 0 ),
    m_errorOutput( 0 ),
    m_statusOutput( 0 )
{
    m_statusOutput = checkForLogFileEnvVar( "TRACELIB_DEBUG_LOG" );
    m_errorOutput = checkForLogFileEnvVar( "TRACELIB_ERROR_LOG" );

    if ( !m_statusOutput ) {
#ifdef _WIN32
        m_statusOutput = new DebugViewLogOutput;
#else
        m_statusOutput = new NullLogOutput;
#endif
    }
    if ( !m_errorOutput ) {
#ifdef _WIN32
        m_errorOutput = new DebugViewLogOutput;
#else
        m_errorOutput = new NullLogOutput;
#endif
    }
    m_log = new Log( m_statusOutput, m_errorOutput );

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
    delete m_log;
    delete m_errorOutput;
    delete m_statusOutput;
}

void Trace::reloadConfiguration( const string &fileName )
{
    m_log->writeStatus( "Trace::reloadConfiguration: reading configuration file from '%s'", fileName.c_str() );
    Configuration *cfg = Configuration::fromFile( fileName, m_log );
    if ( cfg ) {
        setSerializer( cfg->configuredSerializer() );
        setOutput( cfg->configuredOutput() );
        {
            MutexLocker configurationLocker( m_configurationMutex );
            deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
            m_tracePointSets = cfg->configuredTracePointSets();
            delete m_configuration;
            m_configuration = cfg;
        }

        {
            MutexLocker serializerLocker( m_serializerMutex );
            if ( m_serializer ) {
                m_serializer->setStorageConfiguration( cfg->storageConfiguration() );
            }
        }

        /* If any trace keys are given in the XML file, they also implicitely
         * filter out all those trace entries which do not have any of the
         * specified keys. A feature requested by Siemens.
         */
        const vector<TraceKey> traceKeys = cfg->configuredTraceKeys();
        TraceEntry::process.availableTraceKeys = traceKeys;
        if ( !traceKeys.empty() ) {
            vector<TracePointSet *>::iterator setIt, setEnd = m_tracePointSets.end();
            for ( setIt = m_tracePointSets.begin(); setIt != setEnd; ++setIt ) {
                bool haveEnabledTraceKey = false;
                GroupFilter *groupFilter = new GroupFilter;
                groupFilter->setMode( GroupFilter::Whitelist );
                vector<TraceKey>::const_iterator keyIt, keyEnd = traceKeys.end();
                for ( keyIt = traceKeys.begin(); keyIt != keyEnd; ++keyIt ) {
                    if ( keyIt->enabled ) {
                        haveEnabledTraceKey = true;
                        groupFilter->addGroupName( keyIt->name );
                    }
                }

                if ( haveEnabledTraceKey ) {
                    ConjunctionFilter *newFilter = new ConjunctionFilter;
                    newFilter->addFilter( groupFilter );
                    newFilter->addFilter( ( *setIt )->filter() );
                    ( *setIt )->setFilter( newFilter );
                }
            }
        }
    } else {
        setSerializer( 0 );
        setOutput( 0 );
        {
            MutexLocker configurationLocker( m_configurationMutex );
            deleteRange( m_tracePointSets.begin(), m_tracePointSets.end() );
            m_tracePointSets.clear();
            delete m_configuration;
            m_configuration = 0;
        }
        TraceEntry::process.availableTraceKeys.clear();
    }
    if( m_configuration ) {
        m_log->writeStatus( "Trace::reloadConfiguration: configuration updated with serializer: %s and output: %s",
                            (m_serializer ? "yes" : "no"),
                            (m_output ? "yes" : "no") );
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

        m_log->writeStatus( "Trace::configureTracePoint: activating trace point at %s:%d (backtraces=%d, variables=%d)", tracePoint->sourceFile, tracePoint->lineno, tracePoint->backtracesEnabled, tracePoint->variableSnapshotEnabled );

        return;
    }

    m_log->writeStatus( "Trace::configureTracePoint: trace point at %s:%d is not active", tracePoint->sourceFile, tracePoint->lineno );
}

// configures the trace point if necessary and tells us if it's
// supposed to be visited.
bool Trace::advanceVisit( TracePoint *tracePoint ) const
{
    if ( tracePoint->lastUsedConfiguration != m_configuration ) {
        configureTracePoint( tracePoint );
    }

    return tracePoint->active && m_serializer && m_output;
}

void Trace::visitTracePoint( const TracePoint *tracePoint,
                             const char *msg,
                             VariableSnapshot *variables )
{
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
    m_log->writeStatus( "Trace::handleProcessShutdown: detected process shutdown" );

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
        g_activeTrace = new Trace;
    }
    return g_activeTrace;
}

void setActiveTrace( Trace *trace )
{
    g_activeTrace = trace;
}

TRACELIB_NAMESPACE_END

