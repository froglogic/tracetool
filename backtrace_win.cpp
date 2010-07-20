#include "backtrace.h"

#include "3rdparty/stackwalker/StackWalker.h"

using namespace std;
using namespace Tracelib;

class MyStackWalker : public StackWalker
{
public:
    MyStackWalker();

    void setFramesToSkip( size_t i ) { m_framesToSkip = i; }
    const vector<StackFrame> &frames() const { return m_frames; }

protected:
    virtual void OnCallstackEntry( CallstackEntryType type, CallstackEntry &entry );

private:
    size_t m_framesSeen;
    size_t m_framesToSkip;
    vector<StackFrame> m_frames;
};

MyStackWalker::MyStackWalker()
    : StackWalker( RetrieveSymbol | RetrieveLine | RetrieveModuleInfo,
                   NULL,
                   GetCurrentProcessId(),
                   GetCurrentProcess() ),
    m_framesSeen( 0 ),
    m_framesToSkip( 1 )
{
}

void MyStackWalker::OnCallstackEntry( CallstackEntryType type, CallstackEntry &entry )
{
    if ( type == firstEntry ) {
        m_frames.clear();
        m_framesSeen = 0;
    }

    if ( entry.offset != 0 ) {
        if ( ++m_framesSeen <= m_framesToSkip ) {
            return;
        }

        StackFrame frame;
        frame.module = entry.moduleName; // XXX Consider encoding issues
        frame.function = entry.undFullName;
        frame.functionOffset = entry.offsetFromSmybol;
        frame.sourceFile = entry.lineFileName; // XXX Consider encoding issues
        frame.lineNumber = entry.lineNumber;
        m_frames.push_back( frame );
    }
}

Backtrace Backtrace::generate( size_t skipInnermostFrames )
{
    // XXX Make this thread safe using a critical section
    static MyStackWalker stackWalker;
    stackWalker.setFramesToSkip( 2 /* ShowCallstack() + generate() */ + skipInnermostFrames );
    stackWalker.ShowCallstack();
    return Backtrace( stackWalker.frames() );
}

