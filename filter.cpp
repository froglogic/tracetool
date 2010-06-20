#include "filter.h"

using namespace Tracelib;

VerbosityFilter::VerbosityFilter()
    : m_maxVerbosity( 1 )
{
}

void VerbosityFilter::setMaximumVerbosity( unsigned short verbosity )
{
    m_maxVerbosity = verbosity;
}

bool VerbosityFilter::acceptsEntry( unsigned short verbosity, const char *sourceFile, unsigned int lineno, const char *functionName )
{
    return verbosity <= m_maxVerbosity;
}

