#include "backtrace.h"

#include <assert.h>

using namespace std;
using namespace Tracelib;

using namespace Tracelib;

Backtrace::Backtrace( const vector<StackFrame> &frames )
    : m_frames( frames )
{
}

size_t Backtrace::depth() const
{
    return m_frames.size();
}

const StackFrame &Backtrace::frame( size_t depth ) const
{
    assert( depth < m_frames.size() );
    return m_frames[depth];
}

BacktraceFactory::BacktraceFactory()
{
}

BacktraceFactory::~BacktraceFactory()
{
}

Backtrace BacktraceFactory::createBacktrace() const
{
    return Backtrace( getStackFrames() );
}

