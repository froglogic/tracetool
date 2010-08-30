/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#ifndef TRACELIB_FILTER_H
#define TRACELIB_FILTER_H

#include "tracelib_config.h"

#include <string>
#include <vector>

namespace pcrecpp {
    class RE;
}

TRACELIB_NAMESPACE_BEGIN

struct TracePoint;

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
    virtual ~PathFilter();

    void setPath( MatchingMode matchingMode, const std::string &path );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_path;
    pcrecpp::RE *m_rx;
};

class FunctionFilter : public Filter
{
public:
    FunctionFilter();
    virtual ~FunctionFilter();

    void setFunction( MatchingMode matchingMode, const std::string &function );

    virtual bool acceptsTracePoint( const TracePoint *tracePoint );

private:
    MatchingMode m_matchingMode;
    std::string m_function;
    pcrecpp::RE *m_rx;
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

TRACELIB_NAMESPACE_END

#endif // !defined(TRACELIB_FILTER_H)

