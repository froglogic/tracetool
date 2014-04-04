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

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

/* Compiler-specific macros for determining the current file name, line number,
 * and function name.
 */
#ifdef _MSC_VER
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  if _MSC_VER <= 1200
#    define TRACELIB_CURRENT_FUNCTION_NAME "unknown"
#  else
#    define TRACELIB_CURRENT_FUNCTION_NAME __FUNCSIG__
#  endif
#elif __GNUC__
#  define TRACELIB_CURRENT_FILE_NAME __FILE__
#  define TRACELIB_CURRENT_LINE_NUMBER __LINE__
#  define TRACELIB_CURRENT_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#  error "Unsupported compiler!"
#endif

#define TRACELIB_TOKEN_GLUE_(x, y) x ## y
#define TRACELIB_TOKEN_GLUE(x, y) TRACELIB_TOKEN_GLUE_(x, y)


/* The three core macros which actually expand to C++ code; all other macros
 * simply call these. They are no-ops in case TRACELIB_DISABLE_TRACE_CODE is
 * defined.
 */
#ifndef TRACELIB_DISABLE_TRACE_CODE
// Helper macros to avoid duplicating the VISIT_TRACEPOINT* ones, depending on
// wether there is an actual msg or not we need different code as some compilers
// will not accept an anonymous StringBuilder object and require an actual variable
#  define TRACELIB_CREATE_MESSAGE_VAR(msg) \
        TRACELIB_NAMESPACE_IDENT(StringBuilder) msgBuilder; \
        msgBuilder << msg;
#  define TRACELIB_CREATE_NULL_VAR \
        const char *msgBuilder = 0;
#  define TRACELIB_VISIT_TRACEPOINT_VARS(key, vars, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    if ( TRACELIB_NAMESPACE_IDENT(advanceVisit)( &tracePoint ) ) { \
        TRACELIB_NAMESPACE_IDENT(VariableSnapshot) *variableSnapshot = new TRACELIB_NAMESPACE_IDENT(VariableSnapshot); \
        (*variableSnapshot) << vars; \
        msg \
        TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, msgBuilder, variableSnapshot ); \
        for ( size_t i = 0; i < variableSnapshot->size(); ++i ) delete (*variableSnapshot)[i]; \
        delete variableSnapshot; \
    } \
}
#  define TRACELIB_VISIT_TRACEPOINT(type, key, msg) \
{ \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) tracePoint(type, TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, key); \
    if ( TRACELIB_NAMESPACE_IDENT(advanceVisit)( &tracePoint ) ) { \
        msg \
        TRACELIB_NAMESPACE_IDENT(visitTracePoint)( &tracePoint, msgBuilder ); \
    } \
}
#  define TRACELIB_VISIT_TRACEPOINT_STREAM(VisitorType, type, key) \
    static TRACELIB_NAMESPACE_IDENT(TracePoint) TRACELIB_TOKEN_GLUE(tracePoint, TRACELIB_CURRENT_LINE_NUMBER)( (type), TRACELIB_CURRENT_FILE_NAME, TRACELIB_CURRENT_LINE_NUMBER, TRACELIB_CURRENT_FUNCTION_NAME, (key) ); TRACELIB_NAMESPACE_IDENT(VisitorType) TRACELIB_TOKEN_GLUE(tracePointVisitor, TRACELIB_CURRENT_LINE_NUMBER)( &TRACELIB_TOKEN_GLUE(tracePoint, TRACELIB_CURRENT_LINE_NUMBER) ); if ( TRACELIB_NAMESPACE_IDENT(advanceVisit)( &TRACELIB_TOKEN_GLUE(tracePoint, TRACELIB_CURRENT_LINE_NUMBER) ) ) (TRACELIB_TOKEN_GLUE(tracePointVisitor, TRACELIB_CURRENT_LINE_NUMBER))
#  define TRACELIB_VAR_IMPL(v) TRACELIB_NAMESPACE_IDENT(makeConverter)(#v, v)
#else
#  define TRACELIB_VISIT_TRACEPOINT_VARS(key, vars, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT_VARS(key, vars) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT(type, key) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT(type, key, msg) (void)0;
#  define TRACELIB_VISIT_TRACEPOINT_STREAM(VisitorType, type, key) if (false) (TRACELIB_NAMESPACE_IDENT(VisitorType)( NULL ))
#  define TRACELIB_VAR_IMPL(v) NULL
#endif

/* All the _IMPL macros which are referenced from the public macros listed
 * in tracelib_config.h; these macros are just convenience wrappers around
 * the above core macros.
 */
#define TRACELIB_DEBUG_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 0, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_DEBUG_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, 0, TRACELIB_CREATE_MESSAGE_VAR(msg))
#define TRACELIB_DEBUG_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, key, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_DEBUG_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, key, TRACELIB_CREATE_MESSAGE_VAR(msg))

#define TRACELIB_ERROR_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 0, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_ERROR_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, 0, TRACELIB_CREATE_MESSAGE_VAR(msg))
#define TRACELIB_ERROR_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, key, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_ERROR_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, key, TRACELIB_CREATE_MESSAGE_VAR(msg))

#define TRACELIB_TRACE_IMPL                   TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 0, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_TRACE_MSG_IMPL(msg)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, 0, TRACELIB_CREATE_MESSAGE_VAR(msg))
#define TRACELIB_TRACE_KEY_IMPL(key)          TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, key, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_TRACE_KEY_MSG_IMPL(key, msg) TRACELIB_VISIT_TRACEPOINT(TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, key, TRACELIB_CREATE_MESSAGE_VAR(msg))

#define TRACELIB_WATCH_IMPL(vars)                   TRACELIB_VISIT_TRACEPOINT_VARS(0, vars, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_WATCH_MSG_IMPL(msg, vars)          TRACELIB_VISIT_TRACEPOINT_VARS(0, vars, TRACELIB_CREATE_MESSAGE_VAR(msg))
#define TRACELIB_WATCH_KEY_IMPL(key, vars)          TRACELIB_VISIT_TRACEPOINT_VARS(key, vars, TRACELIB_CREATE_NULL_VAR)
#define TRACELIB_WATCH_KEY_MSG_IMPL(key, msg, vars) TRACELIB_VISIT_TRACEPOINT_VARS(key, vars, TRACELIB_CREATE_MESSAGE_VAR(msg))

#define TRACELIB_VALUE_IMPL(v) #v << "=" << v

#define TRACELIB_STREAM_END_IMPL TRACELIB_NAMESPACE_IDENT(StreamEnd())

#define TRACELIB_DEBUG_STREAM_IMPL(key) TRACELIB_VISIT_TRACEPOINT_STREAM(TracePointVisitor, TRACELIB_NAMESPACE_IDENT(TracePointType)::Debug, (key))
#define TRACELIB_ERROR_STREAM_IMPL(key) TRACELIB_VISIT_TRACEPOINT_STREAM(TracePointVisitor, TRACELIB_NAMESPACE_IDENT(TracePointType)::Error, (key))
#define TRACELIB_TRACE_STREAM_IMPL(key) TRACELIB_VISIT_TRACEPOINT_STREAM(TracePointVisitor, TRACELIB_NAMESPACE_IDENT(TracePointType)::Log, (key))
#define TRACELIB_WATCH_STREAM_IMPL(key) TRACELIB_VISIT_TRACEPOINT_STREAM(TracePointVisitor, TRACELIB_NAMESPACE_IDENT(TracePointType)::Watch, (key))

TRACELIB_NAMESPACE_BEGIN

template <class Iterator>
void deleteRange( Iterator begin, Iterator end )
{
    while ( begin != end ) delete *begin++;
}

inline std::string variableValueAsString( const VariableValue &v )
{
    const size_t bufsize = VariableValue::convertToString( v, NULL, 0 );
    std::vector<char> buf( bufsize );
    VariableValue::convertToString( v, &buf[0], buf.size() );
    return &buf[0];
}

class StringBuilder
{
public:
    StringBuilder() { }

    inline operator const char *() {
        m_s = m_stream.str();
        return m_s.c_str();
    }

    StringBuilder &operator<<( const VariableValue &v ) {
        m_stream << variableValueAsString( v );
        return *this;
    }

private:
    StringBuilder( const StringBuilder &other );
    void operator=( const StringBuilder &rhs );

    std::string m_s;
    std::ostringstream m_stream;
};

template <class T>
inline StringBuilder &operator<<( StringBuilder &lhs, const T &rhs ) {
    return lhs << convertVariable( rhs );
}

TRACELIB_EXPORT bool advanceVisit( TracePoint *tracePoint );

TRACELIB_EXPORT void visitTracePoint( const TracePoint *tracePoint,
                      const char *msg = 0,
                      VariableSnapshot *variables = 0 );

struct StreamEnd {
};

class TracePointVisitor {
public:
    inline TracePointVisitor( TracePoint *tracePoint )
        : m_tracePoint( tracePoint )
        , m_variables( 0 )
    { }
    inline ~TracePointVisitor() {
        if( m_variables ) {
            for ( size_t i = 0; i < m_variables->size(); ++i ) delete (*m_variables)[i]; \
            delete m_variables;
        }
    }

    inline TracePointVisitor &operator<<( const VariableValue &v ) {
        m_stream << variableValueAsString( v );
        return *this;
    }

    inline TracePointVisitor &addVariable( AbstractVariable *v ) {
        if ( !m_variables )
            m_variables = new VariableSnapshot;
        (*m_variables) << v;
        return *this;
    }

    void flush() {
        if( m_tracePoint->active ) {
            visitTracePoint( m_tracePoint, m_stream.str().c_str(), m_variables );
        }
    }

private:
    TracePointVisitor( const TracePointVisitor &other );
    void operator=( const TracePointVisitor &rhs );

    TracePoint *m_tracePoint;
    std::ostringstream m_stream;
    VariableSnapshot *m_variables;
};

// Keep these before the template functions below otherwise the compiler will try to put 'AbstractVariable *'
// into the template. I don't understand template logic enough to explain this, so just use the order as a
// workaround for now.
inline TracePointVisitor &operator<<( TracePointVisitor &lhs, AbstractVariable *rhs ) {
    return lhs.addVariable( rhs );
}

inline void operator<<( TracePointVisitor &lhs, const StreamEnd &rhs ) {
    lhs.flush();
}

template <class T>
inline TracePointVisitor &operator<<( TracePointVisitor &lhs, const T &rhs ) {
    return lhs << convertVariable( rhs );
}

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_H)
