/**********************************************************************
** Copyright (C) 2010 froglogic GmbH.
** All rights reserved.
**********************************************************************/

#include "tracelib.h"
#include "eventthread_unix.h"

#include <iostream>

using namespace std;

int g_failureCount = 0;
int g_verificationCount = 0;

template <typename T>
static void verify( const char *what, T expected, T actual )
{
    if ( !( expected == actual ) ) {
        cout << "FAIL: " << what << "; expected '" << boolalpha << expected << "', got '" << boolalpha << actual << "'" << endl;
        ++g_failureCount;
    }
    ++g_verificationCount;
}

TRACELIB_NAMESPACE_BEGIN

static void testCommunication()
{
    EventThreadUnix *event_thread = EventThreadUnix::self();
    verify( "EventThreadUnix::running()",
            true,
            event_thread && event_thread->running() );
    if ( event_thread )
        event_thread->stop();
    verify( "EventThreadUnix::stop()",
            true,
            event_thread && !event_thread->running() );
}

TRACELIB_NAMESPACE_END

int main()
{
    TRACELIB_NAMESPACE_IDENT(testCommunication)();
    cout << g_verificationCount << " verifications; " << g_failureCount << " failures found." << endl;
    return g_failureCount;
}

