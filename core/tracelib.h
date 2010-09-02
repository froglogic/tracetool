/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_H
#define TRACELIB_H

#include "tracelib_config.h"

#include "variabledumping.h"

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

#ifndef NDEBUG
#  define TRACELIB_VARIABLE_SNAPSHOT_MSG(verbosity, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
    (*variableSnapshot) << vars; \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, (msg), variableSnapshot ); \
}

#  define TRACELIB_VISIT_TRACEPOINT_MSG(type, verbosity, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME); \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, msg ); \
}
#  define TRACELIB_VAR(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)
#else
#  define TRACELIB_VARIABLE_SNAPSHOT_MSG(verbosity, vars, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT_MSG(type, verbosity, msg) (void)0;
#  define TRACELIB_VAR(v) (void)0;
#endif

TRACELIB_NAMESPACE_BEGIN

class Configuration;

struct TracePointType {
    enum Value {
        None = 0
#define TRACELIB_TRACEPOINTTYPE(name) ,name
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
    };

    static const int *values() {
        static const int a[] = {
            None,
#define TRACELIB_TRACEPOINTTYPE(name) name,
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
            -1
        };
        return a;
    }

    static const char *valueAsString( Value v ) {
#define TRACELIB_TRACEPOINTTYPE(name) static const char str_##name[] = #name;
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
        switch ( v ) {
            case None: return "None";
#define TRACELIB_TRACEPOINTTYPE(name) case name: return str_##name;
#include "tracepointtypes.def"
#undef TRACELIB_TRACEPOINTTYPE
        }
        return 0;
    }
};

struct TracePoint {
    TracePoint( TracePointType::Value type_, unsigned short verbosity_, const char *sourceFile_, unsigned int lineno_, const char *functionName_ )
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

    const TracePointType::Value type;
    const unsigned short verbosity;
    const char * const sourceFile;
    const unsigned int lineno;
    const char * const functionName;
    const Configuration *lastUsedConfiguration;
    bool active;
    bool backtracesEnabled;
    bool variableSnapshotEnabled;
};

void visitTracePoint( TracePoint *tracePoint,
                      const char *msg = 0,
                      VariableSnapshot *variables = 0 );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
