#ifndef FILTER_H
#define FILTER_H

#include "tracelib.h"

namespace Tracelib
{

class TRACELIB_EXPORT VerbosityFilter : public Filter
{
public:
    VerbosityFilter();

    void setMaximumVerbosity( unsigned short verbosity );

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    unsigned short m_maxVerbosity;
};

class TRACELIB_EXPORT SourceFilePathFilter : public Filter
{
public:
    SourceFilePathFilter();

    void addAcceptablePath( const std::string &path );

    virtual bool acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName );

private:
    std::vector<std::string> m_acceptablePaths;
};

}

#endif // !defined(FILTER_H)

