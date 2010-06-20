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

}

#endif // !defined(FILTER_H)

