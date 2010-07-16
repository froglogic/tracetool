#ifndef FILTER_H
#define FILTER_H

#include "tracelib.h"

namespace Tracelib
{

class VerbosityFilter : public Filter
{
public:
    VerbosityFilter();

    void setMaximumVerbosity( unsigned short verbosity );

    virtual bool acceptsEntry( const TraceEntry &entry );

private:
    unsigned short m_maxVerbosity;
};

class PathFilter : public Filter
{
public:
    PathFilter();

    void setPath( const std::string &path );

    virtual bool acceptsEntry( const TraceEntry &entry );

private:
    std::string m_path;
};

class ConjunctionFilter : public Filter
{
public:
    virtual ~ConjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsEntry( const TraceEntry &entry );

private:
    std::vector<Filter *> m_filters;
};

class DisjunctionFilter : public Filter
{
public:
    virtual ~DisjunctionFilter();

    void addFilter( Filter *filter );

    virtual bool acceptsEntry( const TraceEntry &entry );

private:
    std::vector<Filter *> m_filters;
};

}

#endif // !defined(FILTER_H)

