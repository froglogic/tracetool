#include "backtrace.h"

#include <assert.h>

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

using namespace Tracelib;

Backtrace Backtrace::generate()
{
    /* Reuse one StackWalker instance for performance reasons; the StackWalker
     * constructor is rather expensive.
     */
    static MyStackWalker *stackWalker = 0;
    if ( !stackWalker ) {
        stackWalker = new MyStackWalker;
    }

    stackWalker->ShowCallstack();

    return Backtrace( stackWalker->frames() );
}

Backtrace::Backtrace( const vector<StackFrame> &frames )
    : m_frames( frames )
{
}

Backtrace::~Backtrace()
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

