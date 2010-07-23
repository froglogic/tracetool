#include "backtrace.h"

#include "3rdparty/stackwalker/StackWalker.h"

using namespace std;

namespace {

class MyStackWalker : public StackWalker
{
public:
    MyStackWalker();

    void setFramesToSkip( size_t i ) { m_framesToSkip = i; }
    const vector<TRACELIB_NAMESPACE_IDENT(StackFrame)> &frames() const { return m_frames; }

protected:
    virtual void OnCallstackEntry( CallstackEntryType type, CallstackEntry &entry );

private:
    size_t m_framesSeen;
    size_t m_framesToSkip;
    vector<TRACELIB_NAMESPACE_IDENT(StackFrame)> m_frames;
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

        TRACELIB_NAMESPACE_IDENT(StackFrame) frame;
        frame.module = entry.moduleName; // XXX Consider encoding issues
        frame.function = entry.undFullName;
        frame.functionOffset = entry.offsetFromSmybol;
        frame.sourceFile = entry.lineFileName; // XXX Consider encoding issues
        frame.lineNumber = entry.lineNumber;
        m_frames.push_back( frame );
    }
}

}

TRACELIB_NAMESPACE_BEGIN

struct BacktraceGenerator::Private {
    CRITICAL_SECTION generationSection;
};

BacktraceGenerator::BacktraceGenerator()
    : d( new Private )
{
    ::InitializeCriticalSection( &d->generationSection );
}

BacktraceGenerator::~BacktraceGenerator()
{
    ::DeleteCriticalSection( &d->generationSection );
    delete d;
}

Backtrace BacktraceGenerator::generate( size_t skipInnermostFrames )
{
    ::EnterCriticalSection( &d->generationSection );
    static MyStackWalker stackWalker;
    stackWalker.setFramesToSkip( 2 /* ShowCallstack() + generate() */ + skipInnermostFrames );
    stackWalker.ShowCallstack();
    Backtrace bt( stackWalker.frames() );
    ::LeaveCriticalSection( &d->generationSection );
    return bt;
}

TRACELIB_NAMESPACE_END

