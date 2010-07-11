#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <stdint.h>

#include <string>
#include <vector>

namespace Tracelib
{

struct StackFrame
{
    StackFrame() : functionOffset( 0 ), lineNumber( 0 ) { }
    std::string module;
    std::string function;
    uint64_t functionOffset;
    std::string sourceFile;
    unsigned int lineNumber;
};

class BacktraceFactory;

class Backtrace
{
    friend class BacktraceFactory;

public:
    size_t depth() const;
    const StackFrame &frame( size_t depth ) const;

private:
    explicit Backtrace( const std::vector<StackFrame> &frames );

    std::vector<StackFrame> m_frames;
};

class BacktraceFactory
{
public:
    virtual ~BacktraceFactory();

    Backtrace createBacktrace() const;

protected:
    BacktraceFactory();

private:
    BacktraceFactory( const BacktraceFactory &other );
    void operator=( const BacktraceFactory &other );

    virtual std::vector<StackFrame> getStackFrames() const = 0;
};

}
#endif // !defined(BACKTRACE_H)

