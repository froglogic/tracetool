#ifndef TRACELIB_H
#define TRACELIB_H

#include <vector>

#ifdef _MSC_VER
#  define TRACELIB_BEACON(verbosity) \
{ \
    if (Tracelib::getActiveTrace()) { \
        static Tracelib::TraceCallback cb = Tracelib::getActiveTrace()->getCallback( (verbosity), __FILE__, __LINE__, __FUNCSIG__ ); \
        cb(Tracelib::getActiveTrace(), (verbosity), __FILE__, __LINE__, __FUNCSIG__, std::vector<Tracelib::AbstractVariableConverter *>() ); \
    } \
}
#  define TRACELIB_SNAPSHOT(verbosity, variables) \
{ \
    if (Tracelib::getActiveTrace()) { \
        static Tracelib::TraceCallback cb = Tracelib::getActiveTrace()->getCallback( (verbosity), __FILE__, __LINE__, __FUNCSIG__ ); \
        Tracelib::SnapshotCreator(cb, Tracelib::getActiveTrace(), (verbosity), __FILE__, __LINE__, __FUNCSIG__) << variables; \
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

class Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const std::vector<AbstractVariableConverter *> &variables ) = 0;

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

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName ) = 0;

protected:
    Filter();

private:
    Filter( const Filter &rhs );
    void operator=( const Filter &other );
};

class Trace;

typedef void (*TraceCallback)( Trace *trace,
                               unsigned short verbosity,
                               const char *sourceFile,
                               unsigned int lineno,
                               const char *functionName,
                               const std::vector<AbstractVariableConverter *> &variables );

class SnapshotCreator
{
public:
    SnapshotCreator( TraceCallback callback, Trace *trace, unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );
    ~SnapshotCreator();

    SnapshotCreator &operator<<( AbstractVariableConverter *converter );

private:
    TraceCallback m_callback;
    Trace *m_trace;
    const unsigned short m_verbosity;
    const char * const m_sourceFile;
    const unsigned int m_lineno;
    const char * const m_functionName;
    std::vector<AbstractVariableConverter *> m_variables;
};

class Trace
{
public:
    Trace();
    ~Trace();

    TraceCallback getCallback( unsigned short verbosity,
                               const char *sourceFile,
                               unsigned int lineno,
                               const char *functionName );
    void addEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName, const std::vector<AbstractVariableConverter *> &variables = std::vector<AbstractVariableConverter *>() );
    void setSerializer( Serializer *serializer );
    void setOutput( Output *output );
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
