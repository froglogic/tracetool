#ifndef BACKTRACE_H
#define BACKTRACE_H

#include "tracelib_config.h"

#include <string>
#include <vector>

TRACELIB_NAMESPACE_BEGIN

struct StackFrame
{
    StackFrame() : functionOffset( 0 ), lineNumber( 0 ) { }
    std::string module;
    std::string function;
    size_t functionOffset;
    std::string sourceFile;
    size_t lineNumber;
};

class BacktraceGenerator;

class Backtrace
{
    friend class BacktraceGenerator;

public:
    size_t depth() const;
    const StackFrame &frame( size_t depth ) const;

private:
    explicit Backtrace( const std::vector<StackFrame> &frames );

    std::vector<StackFrame> m_frames;
};

class BacktraceGenerator
{
public:
    BacktraceGenerator();
    ~BacktraceGenerator();

    Backtrace generate( size_t skipInnermostFrames );

private:
    BacktraceGenerator( const BacktraceGenerator &other );
    void operator=( const BacktraceGenerator &rhs );

    struct Private;
    Private *d;
};

TRACELIB_NAMESPACE_END

#endif // !defined(BACKTRACE_H)

