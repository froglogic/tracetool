#ifndef FILTER_H
#define FILTER_H

#include "tracelib.h"

namespace Tracelib
{

enum MatchingMode {
    StrictMatch,
    RegExpMatch,
    WildcardMatch
};

class VerbosityFilter : public Filter
{
public:
    VerbosityFilter();

    void setMaximumVerbosity( unsigned short verbosity );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    unsigned short m_maxVerbosity;
};

class PathFilter : public Filter
{
public:
    PathFilter();

    void setPath( MatchingMode matchingMode, const std::string &path );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_path;
};

class FunctionFilter : public Filter
{
public:
    FunctionFilter();

    void setFunction( MatchingMode matchingMode, const std::string &function );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_function;
};

class ConjunctionFilter : public Filter
{
public:
    virtual ~ConjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    std::vector<Filter *> m_filters;
};

class DisjunctionFilter : public Filter
{
public:
    virtual ~DisjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    std::vector<Filter *> m_filters;
};

}

#endif // !defined(FILTER_H)

