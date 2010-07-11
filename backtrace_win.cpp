#include "backtrace_win.h"

#include "3rdparty/stackwalker/StackWalker.h"

using namespace std;
using namespace Tracelib;

class MyStackWalker : public StackWalker
{
public:
    MyStackWalker();

    const vector<StackFrame> &frames() const { return m_frames; }

protected:
    virtual void OnCallstackEntry( CallstackEntryType type, CallstackEntry &entry );

private:
    vector<StackFrame> m_frames;
};

MyStackWalker::MyStackWalker()
    : StackWalker( RetrieveSymbol | RetrieveLine | RetrieveModuleInfo,
                   NULL,
                   GetCurrentProcessId(),
                   GetCurrentProcess() )
{
}

void MyStackWalker::OnCallstackEntry( CallstackEntryType type, CallstackEntry &entry )
{
    if ( type == firstEntry ) {
        m_frames.clear();
    }

    if ( entry.offset != 0 ) {
        StackFrame frame;
        frame.module = entry.moduleName;
        frame.function = entry.undFullName;
        frame.functionOffset = entry.offsetFromSmybol;
        frame.sourceFile = entry.lineFileName;
        frame.lineNumber = entry.lineNumber;
        m_frames.push_back( frame );
    }
}

WinBacktraceFactory::WinBacktraceFactory()
    : m_stackWalker( new MyStackWalker )
{
}

WinBacktraceFactory::~WinBacktraceFactory()
{
    delete m_stackWalker;
}

vector<StackFrame> WinBacktraceFactory::getStackFrames() const
{
    m_stackWalker->ShowCallstack();
    return m_stackWalker->frames();
}

