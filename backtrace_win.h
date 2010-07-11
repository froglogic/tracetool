#ifndef BACKTRACE_WIN_H
#define BACKTRACE_WIN_H

#include "backtrace.h"

class MyStackWalker;

namespace Tracelib
{

class WinBacktraceFactory
{
public:
    WinBacktraceFactory();
    virtual ~WinBacktraceFactory();

private:
    virtual std::vector<StackFrame> getStackFrames() const = 0;

    MyStackWalker *m_stackWalker;
};

}

#endif // !defined(BACKTRACE_WIN_H)

