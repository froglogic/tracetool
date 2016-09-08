/* tracetool - a framework for tracing the execution of C++ programs
 * Copyright 2010-2016 froglogic GmbH
 *
 * This file is part of tracetool.
 *
 * tracetool is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * tracetool is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with tracetool.  If not, see <http://www.gnu.org/licenses/>.
 */

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

