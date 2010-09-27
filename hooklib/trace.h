/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_TRACE_H
#define TRACELIB_TRACE_H

#include "tracelib_config.h"
#include "backtrace.h"
#include "filemodificationmonitor.h"
#include "getcurrentthreadid.h"
#include "mutex.h"
#include "shutdownnotifier.h"
#include "variabledumping.h"

#include <vector>

TRACELIB_NAMESPACE_BEGIN

class Configuration;
class Filter;
class Output;
class Serializer;
struct TracePoint;

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

struct TracedProcess
{
    ProcessId id;
    time_t startTime;
};

struct TraceEntry
{
    TraceEntry( const TracePoint *tracePoint_, const char *msg = 0 );
    ~TraceEntry();

    static const TracedProcess process;
    const ThreadId threadId;
    const time_t timeStamp;
    const TracePoint *tracePoint;
    VariableSnapshot *variables;
    Backtrace *backtrace;
    const char * const message;
    const size_t stackPosition;
};

struct ProcessShutdownEvent
{
    ProcessShutdownEvent();

    const TracedProcess * const process;
    const time_t shutdownTime;
};


class Trace : public FileModificationMonitorObserver, public ShutdownNotifierObserver
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

    virtual void handleFileModification( const std::string &fileName, NotificationReason reason );

    virtual void handleProcessShutdown();

private:
    Trace( const Trace &trace );
    void operator=( const Trace &trace );

    void reloadConfiguration( const std::string &fileName );

    Serializer *m_serializer;
    Mutex m_serializerMutex;
    Output *m_output;
    Mutex m_outputMutex;
    std::vector<TracePointSet *> m_tracePointSets;
    Configuration *m_configuration;
    mutable Mutex m_configurationMutex;
    BacktraceGenerator m_backtraceGenerator;
    FileModificationMonitor *m_configFileMonitor;
};

Trace *getActiveTrace();
void setActiveTrace( Trace *trace );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
