#ifndef TRACELIB_H
#define TRACELIB_H

#include "backtrace.h"

#include <vector>

#ifdef _MSC_VER
#  define TRACELIB_BEACON(verbosity) \
{ \
    if (Tracelib::getActiveTrace()) { \
        static Tracelib::TracePoint tracePoint((verbosity), __FILE__, __LINE__, __FUNCSIG__); \
        if ( tracePoint.lastSeenFilter != Tracelib::getActiveTrace()->filter() ) { \
            Tracelib::getActiveTrace()->reconsiderTracePoint( &tracePoint ); \
        } \
        if ( tracePoint.active ) { \
            Tracelib::TraceEntry entry( &tracePoint ); \
            if ( tracePoint.backtracesEnabled ) { \
                entry.backtrace = new Tracelib::Backtrace( Tracelib::Backtrace::generate() ); \
            } \
            Tracelib::getActiveTrace()->addEntry( entry ); \
        } \
    } \
}
#  define TRACELIB_SNAPSHOT(verbosity, vars) \
{ \
    if (Tracelib::getActiveTrace()) { \
        static Tracelib::TracePoint tracePoint((verbosity), __FILE__, __LINE__, __FUNCSIG__); \
        if ( tracePoint.lastSeenFilter != Tracelib::getActiveTrace()->filter() ) { \
            Tracelib::getActiveTrace()->reconsiderTracePoint( &tracePoint ); \
        } \
        if ( tracePoint.active ) { \
            Tracelib::TraceEntry entry( &tracePoint ); \
            if ( tracePoint.backtracesEnabled ) { \
                entry.backtrace = new Tracelib::Backtrace( Tracelib::Backtrace::generate() ); \
            } \
            if ( tracePoint.variableSnapshotEnabled ) { \
                entry.variables = new std::vector<Tracelib::AbstractVariableConverter *>; \
                (*entry.variables) << vars; \
            } \
            Tracelib::getActiveTrace()->addEntry( entry ); \
        } \
    } \
}
#  define TRACELIB_VAR(v) Tracelib::makeConverter(#v, v)
#else
#  error "Unsupported compiler!"
#endif

namespace Tracelib
{

template <typename T>
std::string convertVariable( T o );

class AbstractVariableConverter
{
public:
    virtual const char *name() const = 0;
    virtual std::string toString() const = 0;
};

template <typename T>
class VariableConverter : public AbstractVariableConverter
{
public:
    VariableConverter( const char *name, const T &o ) : m_name( name ), m_o( o ) { }

    const char *name() const { return m_name; }

    virtual std::string toString() const {
        return convertVariable( m_o );
    }

private:
    const char *m_name;
    const T &m_o;
};

template <typename T>
AbstractVariableConverter *makeConverter(const char *name, const T &o) {
    return new VariableConverter<T>( name, o );
}

class Output
{
public:
    virtual ~Output();

    virtual bool canWrite() const { return true; }
    virtual void write( const std::vector<char> &data ) = 0;

protected:
    Output();

private:
    Output( const Output &rhs );
    void operator=( const Output &other );
};

struct TracePoint;

struct TraceEntry {
    TraceEntry( const TracePoint *tracePoint_ )
        : tracePoint( tracePoint_ ),
        backtrace( 0 ),
        variables( 0 )
    {
    }

    ~TraceEntry() {
        if ( variables ) {
            std::vector<AbstractVariableConverter *>::const_iterator it, end = variables->end();
            for ( it = variables->begin(); it != end; ++it ) {
                delete *it;
            }
        }
        delete variables;
        delete backtrace;
    }

    const TracePoint *tracePoint;
    std::vector<AbstractVariableConverter *> *variables;
    Backtrace *backtrace;
};

std::vector<AbstractVariableConverter *> &operator<<( std::vector<AbstractVariableConverter *> &v,
                                                      AbstractVariableConverter *c );

class Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( const TraceEntry &entry ) = 0;

protected:
    Serializer();

private:
    Serializer( const Serializer &rhs );
    void operator=( const Serializer &other );
};

class Filter
{
public:
    virtual ~Filter();

    virtual bool acceptsTracePoint( const TracePoint *tracePoint ) = 0;

protected:
    Filter();

private:
    Filter( const Filter &rhs );
    void operator=( const Filter &other );
};

struct TracePoint {
    TracePoint( unsigned short verbosity_, const char *sourceFile_, unsigned int lineno_, const char *functionName_ )
        : verbosity( verbosity_ ),
        sourceFile( sourceFile_ ),
        lineno( lineno_ ),
        functionName( functionName_ ),
        lastSeenFilter( 0 ),
        active( false ),
        backtracesEnabled( false ),
        variableSnapshotEnabled( false )
    {
    }

    const unsigned short verbosity;
    const char * const sourceFile;
    const unsigned int lineno;
    const char * const functionName;
    const Filter *lastSeenFilter;
    bool active;
    bool backtracesEnabled;
    bool variableSnapshotEnabled;
};

class Trace
{
public:
    Trace();
    ~Trace();

    void reconsiderTracePoint( TracePoint *tracePoint ) const;

    void addEntry( const TraceEntry &entry );
    void setSerializer( Serializer *serializer );
    void setOutput( Output *output );

    const Filter *filter() const { return m_filter; }
    void setFilter( Filter *filter );

private:
    Trace( const Trace &trace );
    void operator=( const Trace &trace );

    Serializer *m_serializer;
    Output *m_output;
    Filter *m_filter;
};

Trace *getActiveTrace();
void setActiveTrace( Trace *trace );

}

#endif // !defined(TRACELIB_H)
