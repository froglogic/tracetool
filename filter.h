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

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    unsigned short m_maxVerbosity;
};

class SourceFilePathFilter : public Filter
{
public:
    SourceFilePathFilter();

    void setPath( const std::string &path );

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    std::string m_path;
};

class ConjunctionFilter : public Filter
{
public:
    void addFilter( Filter *filter );

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    std::vector<Filter *> m_filters;
};

class DisjunctionFilter : public Filter
{
public:
    void addFilter( Filter *filter );

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    std::vector<Filter *> m_filters;
};

}

#endif // !defined(FILTER_H)

