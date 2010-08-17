#include "backtrace.h"

#include <cassert>

using namespace std;

TRACELIB_NAMESPACE_BEGIN

BacktraceGenerator::BacktraceGenerator()
{
    // XXX implement
    assert( !"NetworkOutput class not implemented Unix" );
}

BacktraceGenerator::~BacktraceGenerator()
{
}

Backtrace BacktraceGenerator::generate( size_t skipInnermostFrames )
{
    return Backtrace( std::vector<StackFrame>() );
}

TRACELIB_NAMESPACE_END

