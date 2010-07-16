#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <string>
#include <vector>

namespace Tracelib
{

struct StackFrame
{
    StackFrame() : functionOffset( 0 ), lineNumber( 0 ) { }
    std::string module;
    std::string function;
    size_t functionOffset;
    std::string sourceFile;
    size_t lineNumber;
};

class BacktraceFactory;

class Backtrace
{
    friend class BacktraceFactory;

public:
    static Backtrace generate();

    size_t depth() const;
    const StackFrame &frame( size_t depth ) const;

private:
    explicit Backtrace( const std::vector<StackFrame> &frames );

    std::vector<StackFrame> m_frames;
};

}
#endif // !defined(BACKTRACE_H)

