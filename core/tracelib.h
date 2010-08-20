#ifndef TRACELIB_H
#define TRACELIB_H

#include "tracelib_config.h"

#include "backtrace.h"
#include "filter.h"
#include "getcurrentthreadid.h"
#include "mutex.h"
#include "output.h"
#include "serializer.h"
#include "variabledumping.h"

#include <vector>

#ifdef _MSC_VER
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  define TRACELIB_CURRENT_FUNCTION_NAME __FUNCSIG__
#elif __GNUC__
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  define TRACELIB_CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#  error "Unsupported compiler!"
#endif

#define TRACELIB_VARIABLE_SNAPSHOT_MSG(verbosity, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePoint)::WatchPoint, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
    (*variableSnapshot) << vars; \
    TRACELIB_NAMESPACE_IDENT(getActiveTrace)()->visitTracePoint( &tracePoint, (msg), variableSnapshot ); \
}

#define TRACELIB_VISIT_TRACEPOINT_MSG(type, verbosity, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(getActiveTrace)()->visitTracePoint( &tracePoint, msg ); \
}

#define TRACELIB_VAR(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)

#define TRACELIB_DEBUG TRACELIB_DEBUG_MSG(0)
#define TRACELIB_ERROR TRACELIB_ERROR_MSG(0)
#define TRACELIB_TRACE TRACELIB_TRACE_MSG(0)
#define TRACELIB_WATCH(vars) TRACELIB_WATCH_MSG(0, vars)
#define TRACELIB_DEBUG_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePoint)::DebugPoint, 1, msg)
#define TRACELIB_ERROR_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePoint)::ErrorPoint, 1, msg)
#define TRACELIB_TRACE_MSG(msg) TRACELIB_VISIT_TRACEPOINT_MSG(TRACELIB_NAMESPACE_IDENT(TracePoint)::LogPoint, 1, msg)
#define TRACELIB_WATCH_MSG(msg, vars) TRACELIB_VARIABLE_SNAPSHOT_MSG(1, vars, msg)

TRACELIB_NAMESPACE_BEGIN

class Configuration;

struct TracePoint {
    enum Type {
        ErrorPoint,
        DebugPoint,
        LogPoint,
        WatchPoint
    };

    TracePoint( Type type_, unsigned short verbosity_, const char *sourceFile_, unsigned int lineno_, const char *functionName_ )
        : type( type_ ),
        verbosity( verbosity_ ),
        sourceFile( sourceFile_ ),
        lineno( lineno_ ),
        functionName( functionName_ ),
        lastUsedConfiguration( 0 ),
        active( false ),
        backtracesEnabled( false ),
        variableSnapshotEnabled( false )
    {
    }

    const Type type;
    const unsigned short verbosity;
    const char * const sourceFile;
    const unsigned int lineno;
    const char * const functionName;
    const Configuration *lastUsedConfiguration;
    bool active;
    bool backtracesEnabled;
    bool variableSnapshotEnabled;
};

class TracePointSet
{
public:
    static const unsigned int IgnoreTracePoint = 0x0000;
    static const unsigned int LogTracePoint = 0x0001;
    static const unsigned int YieldBacktrace = LogTracePoint | 0x0100;
    static const unsigned int YieldVariables = LogTracePoint | 0x0200;

    TracePointSet( Filter *filter, unsigned int actions );
    ~TracePointSet();

    unsigned int actionForTracePoint( const TracePoint *tracePoint );

private:
    TracePointSet( const TracePointSet &other );
    void operator=( const TracePointSet &rhs );

    Filter *m_filter;
    const unsigned int m_actions;
};

struct TraceEntry
{
    TraceEntry( const TracePoint *tracePoint_, const char *msg = 0 );
    ~TraceEntry();

    const ProcessId processId;
    const ThreadId threadId;
    const time_t timeStamp;
    const TracePoint *tracePoint;
    VariableSnapshot *variables;
    Backtrace *backtrace;
    const char * const message;
};

class Trace
{
public:
    Trace();
    ~Trace();

    void configureTracePoint( TracePoint *tracePoint ) const;
    void visitTracePoint( TracePoint *tracePoint,
                          const char *msg = 0,
                          VariableSnapshot *variables = 0 );

    void addEntry( const TraceEntry &e );

    void setSerializer( Serializer *serializer );
    void setOutput( Output *output );
    void addTracePointSet( TracePointSet *tracePointSet );

private:
    Trace( const Trace &trace );
    void operator=( const Trace &trace );

    Serializer *m_serializer;
    Output *m_output;
    Mutex m_outputMutex;
    std::vector<TracePointSet *> m_tracePointSets;
    Configuration *m_configuration;
    BacktraceGenerator m_backtraceGenerator;
};

Trace *getActiveTrace();
void setActiveTrace( Trace *trace );
void installCrashHandler();

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
