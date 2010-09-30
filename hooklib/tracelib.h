/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_H
#define TRACELIB_H

#include "dlldefs.h"
#include "tracelib_config.h"
#include "tracepoint.h"
#include "variabledumping.h"

#include <sstream>
#include <string>

/* Compiler-specific macros for determining the current file name, line number,
 * and function name.
 */
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

/* The three core macros which actually expand to C++ code; all other macros
 * simply call these. They are no-ops in case TRACELIB_DISABLE_TRACE_CODE is
 * defined.
 */
#ifndef TRACELIB_DISABLE_TRACE_CODE
#  define TRACELIB_VISIT_TRACEPOINT_VARS(verbosity, key, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
    (*variableSnapshot) << vars; \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, (msg), variableSnapshot ); \
}
#  define TRACELIB_VISIT_TRACEPOINT(type, verbosity, key, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, (verbosity), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, msg ); \
}
#  define TRACELIB_VAR(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)
#else
#  define TRACELIB_VISIT_TRACEPOINT_VARS(verbosity, key, vars, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT(type, verbosity, key, msg) (void)0;
#  define TRACELIB_VAR(v) (void)0;
#endif

/* All the _IMPL macros which are referenced from the public macros listed
 * in tracelib_config.h; these macros are just convenience wrappers around
 * the above core macros.
 */
#define TRACELIB_DEBUG_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, 0, 0)
#define TRACELIB_DEBUG_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_DEBUG_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, key, 0)
#define TRACELIB_DEBUG_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_ERROR_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, 0, 0)
#define TRACELIB_ERROR_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_ERROR_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, key, 0)
#define TRACELIB_ERROR_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_TRACE_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, 0, 0)
#define TRACELIB_TRACE_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, 0, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_TRACE_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, key, 0)
#define TRACELIB_TRACE_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 1, key, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_WATCH_IMPL(vars)                   TRACELIB_VISIT_TRACEPOINT_VARS(1, 0, vars, 0)
#define TRACELIB_WATCH_MSG_IMPL(msg, vars)          TRACELIB_VISIT_TRACEPOINT_VARS(1, 0, vars, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)
#define TRACELIB_WATCH_KEY_IMPL(key, vars)          TRACELIB_VISIT_TRACEPOINT_VARS(1, key, vars, 0)
#define TRACELIB_WATCH_KEY_MSG_IMPL(key, msg, vars) TRACELIB_VISIT_TRACEPOINT_VARS(1, key, vars, TRACELIB_NAMESPACE_IDENT(StringBuilder)() << msg)

#define TRACELIB_VALUE_IMPL(v) #v << "=" << v

TRACELIB_NAMESPACE_BEGIN

class StringBuilder
{
public:
    StringBuilder() { }

    inline operator const char * const() {
        m_s = m_stream.str();
        return m_s.c_str();
    }

    template <class T>
    StringBuilder &operator<<( const T &v ) {
        m_stream << v;
        return *this;
    }

private:
    StringBuilder( const StringBuilder &other );
    void operator=( const StringBuilder &rhs );

    std::string m_s;
    std::ostringstream m_stream;
};

template <>
inline StringBuilder &StringBuilder::operator<<( const VariableValue &v ) {
    m_stream << VariableValue::convertToString( v );
    return *this;
}

TRACELIB_EXPORT void visitTracePoint( TracePoint *tracePoint,
                      const char *msg = 0,
                      VariableSnapshot *variables = 0 );

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
