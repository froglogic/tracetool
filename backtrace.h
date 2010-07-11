#ifndef BACKTRACE_H
#define BACKTRACE_H

#include <string>
#include <vector>

#include <windows.h>

namespace Tracelib
{

struct StackFrame
{
    StackFrame() : functionOffset( 0 ), lineNumber( 0 ) { }
    std::string module;
    std::string function;
    DWORD64 functionOffset;
    std::string sourceFile;
    DWORD lineNumber;
};

class Backtrace
{
public:
    ~Backtrace();

    static Backtrace generate();

    size_t depth() const;
    const StackFrame &frame( size_t depth ) const;

private:
    Backtrace( const std::vector<StackFrame> &frames );

    std::vector<StackFrame> m_frames;
};

}
#endif // !defined(BACKTRACE_H)

