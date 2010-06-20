#ifndef TRACELIB_H
#define TRACELIB_H

#include <vector>

#ifdef _MSC_VER
#  define TRACELIB_BEACON(verbosity) if (Tracelib::getActiveTrace()) Tracelib::getActiveTrace()->addEntry((verbosity), __FILE__, __LINE__, __FUNCSIG__);
#  ifdef TRACELIB_MAKEDLL
#    define TRACELIB_EXPORT __declspec(dllexport)
#  else
#    define TRACELIB_EXPORT __declspec(dllimport)
#  endif
#else
#  error "Unsupported compiler!"
#endif

namespace Tracelib
{

class TRACELIB_EXPORT Output
{
public:
    virtual ~Output();

    virtual void write( const std::vector<char> &data ) = 0;

protected:
    Output();

private:
    Output( const Output &rhs );
    void operator=( const Output &other );
};

class TRACELIB_EXPORT Serializer
{
public:
    virtual ~Serializer();

    virtual std::vector<char> serialize( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName ) = 0;

protected:
    Serializer();

private:
    Serializer( const Serializer &rhs );
    void operator=( const Serializer &other );
};

class TRACELIB_EXPORT Filter
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

class TRACELIB_EXPORT Trace
{
public:
    Trace();
    ~Trace();

    void addEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );
    void setSerializer( Serializer *serializer );
    void setOutput( Output *output );
    void addFilter( Filter *filter );

private:
    Trace( const Trace &trace );
    void operator=( const Trace &trace );

    Serializer *m_serializer;
    Output *m_output;
    std::vector<Filter *> m_filters;
};

TRACELIB_EXPORT Trace *getActiveTrace();
TRACELIB_EXPORT void setActiveTrace( Trace *trace );

}

#endif // !defined(TRACELIB_H)
