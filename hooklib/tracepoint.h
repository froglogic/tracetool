#ifndef TRACELIB_TRACEPOINT_H
#define TRACELIB_TRACEPOINT_H

#include "tracelib_config.h"
#include "dlldefs.h"

TRACELIB_NAMESPACE_BEGIN

struct TracePointType {
    enum TRACELIB_EXPORT Value {
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

class Configuration;

struct TracePoint {
    TRACELIB_EXPORT TracePoint( TracePointType::Value type_, unsigned short verbosity_, const char *sourceFile_, unsigned int lineno_, const char *functionName_, const char *groupName_ )
        : type( type_ ),
        verbosity( verbosity_ ),
        sourceFile( sourceFile_ ),
        lineno( lineno_ ),
        functionName( functionName_ ),
        groupName( groupName_ ),
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
    const char * const groupName;
    const Configuration *lastUsedConfiguration;
    bool active;
    bool backtracesEnabled;
    bool variableSnapshotEnabled;
};

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_TRACEPOINT_H)
